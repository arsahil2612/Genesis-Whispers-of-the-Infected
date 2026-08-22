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
void Map::LoadLevel(int levelNumber) {
    platforms.clear();

    if (levelNumber == 1) {
        for (int i = 0; i < 10; ++i) {
            LoadLevel1BackgroundTexture(i, false);
        }

        // --- Ground Platforms (aligned with y = 185 top height matching genesis_bg terrain) ---
        // Segment 1: Sections 1-8 (Spawn to Quarantine Zone)
        platforms.push_back({0, 165, 17800, 20});

        // Segment 2: Broken Bridge Step 1
        platforms.push_back({18100, 165, 400, 20});

        // Segment 3: Broken Bridge Step 2
        platforms.push_back({18800, 165, 400, 20});

        // Segment 4: Sections 10-11 (Boss Arena and Exit Gate)
        platforms.push_back({19400, 165, 2320, 20});
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
// Tilemap Surface & Platform Rendering Loop
// ============================================================================
void Map::RenderTiles(double camX, double camY) {
    ResourceManager& rm = ResourceManager::GetInstance();

    // Bridge Tiles
    unsigned int texBridgeFloor = rm.GetBridgeFloorTile();
    unsigned int texBrokenBridge = rm.GetBrokenBridgeFloorTile();
    unsigned int texRiverWater = rm.GetRiverWaterTile();

    const int kTileWidth = 160;

    // Render River Water beneath the Broken Bridge gap (World X: 17500 to 19400)
    if (17500 - camX <= 1480 && 19400 - camX >= -200) {
        for (double wx = 17500; wx < 19400; wx += kTileWidth) {
            double screenWX = wx - camX;
            if (screenWX + kTileWidth >= -200 && screenWX <= 1480) {
                iShowImage((int)screenWX, 0, kTileWidth, 140, texRiverWater);
            }
        }
    }

    // Render Bridge steps across the river gap
    for (size_t i = 0; i < platforms.size(); ++i) {
        const Platform& p = platforms[i];
        if (p.x >= 17500 && p.x < 19400) {
            double screenPx = p.x - camX;
            double screenPy = p.y - camY;

            if (screenPx + p.width >= -200 && screenPx <= 1480) {
                int count = (int)(p.width / kTileWidth) + 1;
                for (int t = 0; t < count; ++t) {
                    double tileX = p.x + (t * kTileWidth) - camX;
                    double drawW = kTileWidth;
                    if (tileX + drawW > (p.x + p.width - camX)) {
                        drawW = (p.x + p.width - camX) - tileX;
                    }

                    if (tileX + drawW >= -200 && tileX <= 1480 && drawW > 0) {
                        unsigned int currentTex = (t % 2 == 0) ? texBridgeFloor : texBrokenBridge;
                        iShowImage((int)tileX, (int)(screenPy - 40), (int)drawW, 64, currentTex);
                    }
                }
            }
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

