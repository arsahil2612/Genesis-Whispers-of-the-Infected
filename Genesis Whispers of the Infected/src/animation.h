#ifndef ANIMATION_H
#define ANIMATION_H

#include <windows.h>
#include <GL/gl.h>
#include <vector>

class Animation {
private:
    unsigned int textureID;             // Single sprite sheet texture handle
    std::vector<unsigned int> textures; // Multi-file frame sequence texture handles
    int frameCount;                     // Total number of frames
    int frameWidth;                     // Width of each individual frame in pixels
    int frameHeight;                    // Height of each individual frame in pixels
    int currentFrame;                   // Current frame index (0-indexed)
    int frameCounter;                   // Tick accumulator for frame timing
    int frameDuration;                  // Ticks per frame (duration)
    bool loop;                          // Whether animation loops continuously
    bool finished;                      // Set true when non-looping animation finishes

public:
    // ========================================================================
    // Constructors & Initializer Overloads
    // ========================================================================
    Animation();
    Animation(unsigned int texID, int count, int fWidth, int fHeight, int duration, bool isLooping = true);

    void Init(const std::vector<unsigned int>& texIDs, int duration, bool isLooping = true);
    void Init(unsigned int texID, int count, int fWidth, int fHeight, int duration, bool isLooping = true);
    void InitSequence(const std::vector<unsigned int>& texIDs, int duration, bool isLooping = true);

    // ========================================================================
    // Playback & Render Pipeline
    // ========================================================================
    void Update();
    void Render(int x, int y, int drawWidth, int drawHeight, bool isFacingRight, float brightness = 1.20f) const;
    void Reset();

    // ========================================================================
    // Getters & Accessors
    // ========================================================================
    unsigned int GetTextureID() const { return textures.empty() ? textureID : textures[currentFrame % textures.size()]; }
    int GetFrameCount() const { return frameCount; }
    int GetFrameWidth() const { return frameWidth; }
    int GetFrameHeight() const { return frameHeight; }
    int GetCurrentFrame() const { return currentFrame; }
    int GetFrameDuration() const { return frameDuration; }
    bool IsFinished() const { return finished; }
    bool IsValid() const { return (!textures.empty() && textures[0] != 0) || (textureID != 0 && frameCount > 0); }

    // ========================================================================
    // Setters
    // ========================================================================
    void SetFrameDuration(int duration) { frameDuration = duration; }
    void SetCurrentFrame(int frame) { currentFrame = frame; }
};

#endif // ANIMATION_H

