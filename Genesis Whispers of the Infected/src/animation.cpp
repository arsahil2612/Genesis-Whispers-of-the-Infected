#include "animation.h"

// ============================================================================
// Constructors & Initializers
// ============================================================================
Animation::Animation() {
    textureID = 0;
    frameCount = 1;
    frameWidth = 128;
    frameHeight = 128;
    currentFrame = 0;
    frameCounter = 0;
    frameDuration = 8;
    loop = true;
    finished = false;
}

Animation::Animation(unsigned int texID, int count, int fWidth, int fHeight, int duration, bool isLooping) {
    Init(texID, count, fWidth, fHeight, duration, isLooping);
}

void Animation::Init(unsigned int texID, int count, int fWidth, int fHeight, int duration, bool isLooping) {
    textures.clear();
    textureID = texID;
    frameCount = (count > 0) ? count : 1;
    frameWidth = fWidth;
    frameHeight = fHeight;
    currentFrame = 0;
    frameCounter = 0;
    frameDuration = (duration > 0) ? duration : 1;
    loop = isLooping;
    finished = false;
}

void Animation::Init(const std::vector<unsigned int>& texIDs, int duration, bool isLooping) {
    InitSequence(texIDs, duration, isLooping);
}

void Animation::InitSequence(const std::vector<unsigned int>& texIDs, int duration, bool isLooping) {
    textures = texIDs;
    textureID = textures.empty() ? 0 : textures[0];
    frameCount = (int)textures.size();
    if (frameCount <= 0) frameCount = 1;
    frameWidth = 128;
    frameHeight = 128;
    currentFrame = 0;
    frameCounter = 0;
    frameDuration = (duration > 0) ? duration : 1;
    loop = isLooping;
    finished = false;
}

// ============================================================================
// Playback Step Update
// ============================================================================
void Animation::Update() {
    if (finished || frameDuration <= 0) return;

    int totalFrames = textures.empty() ? frameCount : (int)textures.size();
    if (totalFrames <= 0) totalFrames = 1;

    frameCounter++;
    if (frameCounter >= frameDuration) {
        frameCounter = 0;
        currentFrame++;
        if (currentFrame >= totalFrames) {
            if (loop) {
                currentFrame = 0;
            }
            else {
                currentFrame = totalFrames - 1;
                finished = true;
            }
        }
    }
}

// ============================================================================
// OpenGL Quad Rendering Pipeline
// ============================================================================
void Animation::Render(int x, int y, int drawWidth, int drawHeight, bool isFacingRight, float brightness) const {
    if (!IsValid()) return;

    unsigned int activeTex = textureID;
    double uStart = 0.0;
    double uEnd = 1.0;
    double vStart = 0.0;
    double vEnd = 1.0;

    if (!textures.empty()) {
        int frameIndex = (currentFrame >= 0) ? (currentFrame % (int)textures.size()) : 0;
        activeTex = textures[frameIndex];
    }
    else {
        int totalCols = (frameCount > 0) ? frameCount : 1;
        int frameIndex = (currentFrame >= 0) ? (currentFrame % totalCols) : 0;
        double colWidth = 1.0 / (double)totalCols;
        uStart = (double)frameIndex * colWidth;
        uEnd = uStart + colWidth;
    }

    if (activeTex == 0) return;

    // Flip UV horizontally if facing left
    if (!isFacingRight) {
        double temp = uStart;
        uStart = uEnd;
        uEnd = temp;
    }

    // 1. Subtle drop shadow outline pass behind character (2px offset, dark alpha)
    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, activeTex);

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBegin(GL_QUADS);
    glTexCoord2f((float)uStart, (float)vEnd);   glVertex2f((float)(x + 2), (float)(y - 2));
    glTexCoord2f((float)uEnd,   (float)vEnd);   glVertex2f((float)(x + 2 + drawWidth), (float)(y - 2));
    glTexCoord2f((float)uEnd,   (float)vStart); glVertex2f((float)(x + 2 + drawWidth), (float)(y - 2 + drawHeight));
    glTexCoord2f((float)uStart, (float)vStart); glVertex2f((float)(x + 2), (float)(y - 2 + drawHeight));
    glEnd();

    // 2. Main character pass with brightness boost
    glColor4f(brightness, brightness, brightness, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f((float)uStart, (float)vEnd);   glVertex2f((float)x, (float)y);
    glTexCoord2f((float)uEnd,   (float)vEnd);   glVertex2f((float)(x + drawWidth), (float)y);
    glTexCoord2f((float)uEnd,   (float)vStart); glVertex2f((float)(x + drawWidth), (float)(y + drawHeight));
    glTexCoord2f((float)uStart, (float)vStart); glVertex2f((float)x, (float)(y + drawHeight));
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// ============================================================================
// Animation State Reset
// ============================================================================
void Animation::Reset() {
    currentFrame = 0;
    frameCounter = 0;
    finished = false;
}

