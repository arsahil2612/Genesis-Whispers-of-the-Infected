#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include "asset_loader.h"
#include "igraphics_declarations.h"
#include <cstdio>

// Map rendering layout constants
namespace {
    const int kBgSliceWidth = 1448;
    const int kBgSliceHeight = 768;
    const int kScreenHeight = 768;
    const int kBgDrawYOffset = 0; // Align background bottom to y=0 so visual ground matches feet
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

        // --- Environmental Platform Mechanics (Floating Platforms) ---
        // Section 2: Destroyed House second floors
        platforms.push_back({2500, 300, 300, 20});
        platforms.push_back({2700, 450, 200, 20});

        // Section 3: Village Street barricade tops
        platforms.push_back({4800, 250, 180, 20});

        // Section 4: Village Square fountain highlight
        platforms.push_back({6800, 220, 250, 30});

        // Section 5: Abandoned Market shelves
        platforms.push_back({9500, 280, 220, 20});
        platforms.push_back({9800, 400, 180, 20});

        // Section 6: Raider Camp watchtowers
        platforms.push_back({11500, 320, 300, 20});
        platforms.push_back({11600, 450, 150, 20});

        // Section 7: Abandoned Church rafters
        platforms.push_back({13800, 350, 400, 20});

        // Section 8: Quarantine Zone command post tent top
        platforms.push_back({16000, 280, 250, 20});

        // Section 9: Broken Bridge platforming assists (floating wood/debris)
        platforms.push_back({17900, 300, 100, 20});
        platforms.push_back({18600, 320, 100, 20});

        // Section 10: Boss Arena destroyed military trucks
        platforms.push_back({20100, 260, 200, 20});
    }
}

// ============================================================================
// Background Layer Render Loop
// ============================================================================
void Map::RenderBackground(double camX, bool bossDefeated) {
    // Render main background layer with smooth side-scrolling across 1280-wide window
    for (int i = 0; i < 10; ++i) {
        double xPos = (i * kBgSliceWidth) - camX;

        // Render all slices that overlap or border the 1280-wide screen viewport
        if (xPos + kBgSliceWidth >= -200 && xPos <= 1480) {
            unsigned int tex = LoadLevel1BackgroundTexture(i, bossDefeated);
            if (tex != 0) {
                // Use floor() so negative screen coordinates round down accurately, with 1px precision edge alignment
                int drawX = (int)floor(xPos);
                iShowImage(drawX, kBgDrawYOffset, kBgSliceWidth + 1, kBgSliceHeight, tex);
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

