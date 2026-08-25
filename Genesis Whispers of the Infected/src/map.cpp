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

        // --- Level 1 Platform & Broken Bridge Geometry ---
        // Section 1: Spawn Area to Quarantine Zone (World X: 0 to 10300)
        platforms.push_back({0, 165, 10300, 20});

        // Section 2: Broken Bridge Section 8 Traversal Platforms (World X: 10300 to 11440)
        // Segment 2A: Left Bridge Platform (10300 to 10620, Width 320)
        platforms.push_back({10300, 165, 320, 20});

        // [Gap 1: Broken Gap from 10620 to 10760 (140px jump chasm)]

        // Segment 2B: Middle Broken Bridge Plank / Island (10760 to 11000, Width 240)
        platforms.push_back({10760, 165, 240, 20});

        // [Gap 2: Broken Gap from 11000 to 11150 (150px jump chasm)]

        // Segment 2C: Right Bridge Platform (11150 to 11440, Width 290)
        platforms.push_back({11150, 165, 290, 20});

        // Section 3: Mini Boss Arena to Exit Gate (World X: 11440 to 14480)
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
// LAYER 2: WATER / RIVER RENDERING (Low river surface y=0 to 90)
// ============================================================================
void Map::RenderWater(double camX, double camY) {
    ResourceManager& rm = ResourceManager::GetInstance();

    const double kChasmStartX = 10300.0;
    const double kChasmEndX = 11440.0;

    double screenChasmStart = kChasmStartX - camX;
    double screenChasmEnd = kChasmEndX - camX;

    // Render River Water low in the chasm (y = 0 to 90), safely below bridge platform at y = 165
    if (screenChasmEnd >= -200 && screenChasmStart <= 1480) {
        unsigned int texRiverWater = rm.GetRiverWaterTile();
        const int kTileW = 160;
        const int kWaterH = 90;

        for (double wx = kChasmStartX; wx < kChasmEndX; wx += kTileW) {
            double screenX = wx - camX;
            if (screenX + kTileW >= -200 && screenX <= 1480) {
                iShowImage((int)screenX, 0, kTileW, kWaterH, texRiverWater);
            }
        }
    }
}

// ============================================================================
// LAYER 3: BRIDGE AND ENVIRONMENT SPRITES (Beams, planks & edge caps)
// ============================================================================
void Map::RenderBridgeAndEnvironmentSprites(double camX, double camY) {
    ResourceManager& rm = ResourceManager::GetInstance();

    unsigned int texBridgeFloor = rm.GetBridgeFloorTile();
    unsigned int texBrokenBridge = rm.GetBrokenBridgeFloorTile();
    unsigned int texSupportBeam = rm.GetWoodenBridgeSupportBeamTile();

    const int kTileW = 80;

    // 1. Render Wooden Bridge Support Beams (y = 90 to 165, from river surface to platform base)
    if (texSupportBeam != 0) {
        double beamPositions[] = { 10350.0, 10530.0, 10720.0, 10960.0, 11150.0, 11400.0 };
        for (double bX : beamPositions) {
            double sX = bX - camX;
            if (sX >= -100 && sX <= 1380) {
                iShowImage((int)sX, 90, 48, 75, texSupportBeam);
            }
        }
    }

    // 2. Render Wooden Bridge Deck Planks across the 3 playable bridge sections (World X: 10300 to 11440)
    struct BridgeSection { double startX, endX; };
    BridgeSection sections[] = {
        { 10300.0, 10550.0 }, // Left Bridge Platform (Section 2A)
        { 10700.0, 10980.0 }, // Middle Bridge Island (Section 2B)
        { 11130.0, 11440.0 }  // Right Bridge Platform (Section 2C)
    };

    for (const auto& sec : sections) {
        double secWidth = sec.endX - sec.startX;
        double screenStart = sec.startX - camX;
        double screenEnd = sec.endX - camX;

        if (screenEnd >= -200 && screenStart <= 1480) {
            int count = (int)(secWidth / kTileW) + 1;
            for (int t = 0; t < count; ++t) {
                double tileX = sec.startX + (t * kTileW) - camX;
                double drawW = kTileW;
                if (sec.startX + (t * kTileW) + drawW > sec.endX) {
                    drawW = sec.endX - (sec.startX + (t * kTileW));
                }

                if (tileX + drawW >= -200 && tileX <= 1480 && drawW > 0) {
                    unsigned int currentTex = (t % 2 == 0) ? texBridgeFloor : texBrokenBridge;
                    if (currentTex != 0) {
                        // Planks rendered at y=165, height=20 (top surface aligned exactly at y=185 matching player feet)
                        iShowImage((int)tileX, 165, (int)drawW, 20, currentTex);
                    }
                }
            }
        }
    }
}

// ============================================================================
// Tilemap Surface & Platform Rendering Loop
// ============================================================================
void Map::RenderTiles(double camX, double camY) {
    RenderWater(camX, camY);
    RenderBridgeAndEnvironmentSprites(camX, camY);
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

