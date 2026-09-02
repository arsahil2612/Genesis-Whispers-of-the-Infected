#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include "ResourceManager.h"
#include "asset_loader.h"
#include "igraphics_declarations.h"
#include <cstdio>
#include <cmath>
#include <windows.h>
#include <GL/gl.h>

// Map rendering layout constants
namespace {
    const int kBgSliceWidth = 1448;
    const int kBgSliceHeight = 768;
    const int kScreenHeight = 768;
    const int kBgDrawYOffset = 0; // Align background bottom to y=0 so visual ground matches feet

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

    // OpenGL quad renderer with horizontal linear alpha gradient for smooth map background cross-fading
    void RenderTexturedQuadGradientAlpha(unsigned int texture, float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, float alpha1, float alpha2) {
        if (texture == 0) return;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glBegin(GL_QUADS);

        // Bottom-Left
        glColor4f(1.0f, 1.0f, 1.0f, alpha1);
        glTexCoord2f(u1, 0.999f);
        glVertex2f(x1, y1);

        // Bottom-Right
        glColor4f(1.0f, 1.0f, 1.0f, alpha2);
        glTexCoord2f(u2, 0.999f);
        glVertex2f(x2, y1);

        // Top-Right
        glColor4f(1.0f, 1.0f, 1.0f, alpha2);
        glTexCoord2f(u2, 0.001f);
        glVertex2f(x2, y2);

        // Top-Left
        glColor4f(1.0f, 1.0f, 1.0f, alpha1);
        glTexCoord2f(u1, 0.001f);
        glVertex2f(x1, y2);

        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

// ============================================================================
// Map Constructor
// ============================================================================
Map::Map() {
    cameraX = 0;
    cameraY = 0;
    levelWidth = 14480; // 10 background sections of width 1448
    levelHeight = 768;
}

// ============================================================================
// Level Geometry & Collision Setup
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// Level Geometry & Collision Setup
// ============================================================================
void Map::LoadLevel(int levelNumber) {
    platforms.clear();

    if (levelNumber == 1) {
        for (int i = 0; i < 10; ++i) {
            LoadLevel1BackgroundTexture(i, false);
        }

        // --- Level 1 Platform & Broken Bridge Geometry ---
        // Section 1: Spawn Area to Quarantine Zone Ground (World X: 0 to 10300, Top Surface Y = 185)
        platforms.push_back({0, 165, 10300, 20});

        // Section 2: Broken Bridge Section 8 Traversal Platforms (World X: 10300 to 11440, Top Surface Y = 185)
        // Segment 2A: Left Bridge Platform (10300 to 10620, Width 320, Top Y = 185)
        platforms.push_back({10300, 165, 320, 20});

        // [Gap 1: Broken Gap from 10620 to 10760 (140px jump chasm)]

        // Segment 2B: Middle Broken Bridge Plank / Island (10760 to 11000, Width 240, Top Y = 185)
        platforms.push_back({10760, 165, 240, 20});

        // [Gap 2: Broken Gap from 11000 to 11150 (150px jump chasm)]

        // Segment 2C: Right Bridge Platform (11150 to 11440, Width 290, Top Y = 185)
        platforms.push_back({11150, 165, 290, 20});

        // Section 3: Mini Boss Arena to Exit Gate Ground (World X: 11440 to 14480, Top Surface Y = 185)
        platforms.push_back({11440, 165, 3040, 20});
    }
}

// ============================================================================
// Background Layer Render Loop
// ============================================================================
void Map::RenderBackground(double camX, bool bossDefeated) {
    // Pass 1: Render main background slices
    for (int i = 0; i < 10; ++i) {
        double xPos = (i * kBgSliceWidth) - camX;

        if (xPos + kBgSliceWidth >= -200 && xPos <= 1480) {
            unsigned int tex = LoadLevel1BackgroundTexture(i, bossDefeated);
            if (tex != 0) {
                int drawX = (int)floor(xPos);
                iShowImage(drawX, kBgDrawYOffset, kBgSliceWidth + 1, kBgSliceHeight, tex);
            }
        }
    }

    // Pass 2: Soft, natural 360px centered cross-fade over map slice boundaries
    const float kBlendHalf = 180.0f;
    float uSpan = (kBlendHalf * 2.0f) / (float)kBgSliceWidth;

    for (int i = 0; i < 9; ++i) {
        double boundaryX = ((i + 1) * kBgSliceWidth) - camX;
        double blendLeft = boundaryX - kBlendHalf;
        double blendRight = boundaryX + kBlendHalf;

        if (blendRight >= -100 && blendLeft <= 1380) {
            unsigned int prevTex = LoadLevel1BackgroundTexture(i, bossDefeated);
            unsigned int nextTex = LoadLevel1BackgroundTexture(i + 1, bossDefeated);

            if (prevTex != 0 && nextTex != 0) {
                // Render previous slice (i) right edge fading from 1.0 to 0.0 alpha
                RenderTexturedQuadGradientAlpha(
                    prevTex,
                    (float)blendLeft, (float)kBgDrawYOffset,
                    (float)blendRight, (float)(kBgDrawYOffset + kBgSliceHeight),
                    1.0f - uSpan, 0.001f, 0.999f, 0.999f,
                    1.0f, 0.0f
                );

                // Render next slice (i+1) left edge fading from 0.0 to 1.0 alpha
                RenderTexturedQuadGradientAlpha(
                    nextTex,
                    (float)blendLeft, (float)kBgDrawYOffset,
                    (float)blendRight, (float)(kBgDrawYOffset + kBgSliceHeight),
                    0.001f, 0.001f, uSpan, 0.999f,
                    0.0f, 1.0f
                );
            }
        }
    }
}

// ============================================================================
// LAYER 2: WATER / RIVER RENDERING (River surface y=0 to 130)
// ============================================================================
void Map::RenderWater(double camX, double camY) {
    ResourceManager& rm = ResourceManager::GetInstance();

    const double kChasmStartX = 10300.0;
    const double kChasmEndX = 11440.0;

    double screenChasmStart = kChasmStartX - camX;
    double screenChasmEnd = kChasmEndX - camX;

    // Render River Water low in the chasm (y = 0 to 130), clearly visible under bridge deck at y = 185
    if (screenChasmEnd >= -200 && screenChasmStart <= 1480) {
        unsigned int texRiverWater = rm.GetRiverWaterTile();
        if (texRiverWater != 0) {
            const int kTileW = 160;
            const int kWaterH = 130;

            for (double wx = kChasmStartX; wx < kChasmEndX; wx += kTileW) {
                double screenX = wx - camX;
                if (screenX + kTileW >= -200 && screenX <= 1480) {
                    int drawW = kTileW;
                    if (wx + drawW > kChasmEndX) {
                        drawW = (int)(kChasmEndX - wx);
                    }
                    iShowImage((int)screenX, 0, drawW, kWaterH, texRiverWater);
                }
            }
        }
    }
}

// ============================================================================
// LAYER 3: BRIDGE AND ENVIRONMENT SPRITES (Broken Bridge Structure)
// ============================================================================
void Map::RenderBridgeAndEnvironmentSprites(double camX, double camY) {
    ResourceManager& rm = ResourceManager::GetInstance();

    unsigned int texBridgeStructure = rm.GetBrokenBridgeEdgeTile();

    const double kBridgeStartX = 10300.0;
    const double kBridgeEndX = 11440.0;
    const double kBridgeSpanW = kBridgeEndX - kBridgeStartX; // 1140 px

    // Render High-Resolution Broken Bridge Structure Sprite (broken_bridge_edge.png [1774x887])
    // Scaled to full proportion (drawW = 1140, drawH = 330) matching reference screenshot.
    // Deck top surface is at fraction 0.7993 of texture height.
    // Setting drawY = 225.8 - (0.7993 * 330.0) = -38.0 aligns top surface of bridge deck EXACTLY at y=225.8 matching Arin's boots baseline,
    // so Arin stands directly on top of the wooden bridge planks while support pillars extend deep into the river water!
    if (texBridgeStructure != 0) {
        double screenBridgeX = kBridgeStartX - camX;
        if (screenBridgeX + kBridgeSpanW >= -200 && screenBridgeX <= 1480) {
            const int kDrawW = (int)kBridgeSpanW;
            const int kDrawH = 330;
            const int kDrawY = (int)floor(225.8 - (0.7993 * (double)kDrawH)); // -38
            iShowImage((int)screenBridgeX, kDrawY, kDrawW, kDrawH, texBridgeStructure);
        }
    }
}


// ============================================================================
// Smooth Viewport Camera Tracking
// ============================================================================
void Map::ApplyCameraTracking(double playerX, double playerY, int screenWidth, int screenHeight) {
    double targetCameraX = playerX - (screenWidth / 2.0);
    cameraX += (targetCameraX - cameraX) * 0.1;

    if (cameraX < 0) cameraX = 0;
    if (cameraX > (levelWidth - screenWidth)) cameraX = levelWidth - screenWidth;

    cameraY = 0;
}
