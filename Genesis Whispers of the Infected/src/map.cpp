#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include "ResourceManager.h"
#include "asset_loader.h"
#include "igraphics_declarations.h"
#include <cstdio>
#include <cmath>

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
// Tilemap Surface & Platform Rendering Loop
// ============================================================================
void Map::RenderTiles(double camX, double camY) {
    ResourceManager& rm = ResourceManager::GetInstance();

    // Wooden Tiles
    unsigned int texWood = rm.GetWoodFloorTile();
    unsigned int texBrokenWood = rm.GetBrokenWoodFloorTile();
    unsigned int texEdge = rm.GetWoodFloorEdgeTile();
    unsigned int texPlatform = rm.GetWoodPlatformTile();

    // Ground & Road Tiles
    unsigned int texDirt = rm.GetDirtTile();
    unsigned int texGrass = rm.GetVillageGrassTile();
    unsigned int texRoad = rm.GetBrokenRoadTile();

    // Church Tiles
    unsigned int texChurchStone = rm.GetChurchStoneFloorTile();
    unsigned int texChurchEdge = rm.GetChurchStoneFloorEdge();
    unsigned int texChurchPlatform = rm.GetChurchStonePlatform();

    // Quarantine Zone & Military Concrete Tiles
    unsigned int texMilitaryConcrete = rm.GetMilitaryConcreteFloorTile();
    unsigned int texCrackedConcrete = rm.GetCrackedMilitaryConcreteTile();
    unsigned int texHazardConcrete = rm.GetHazardMilitaryConcreteTile();
    unsigned int texConcreteEdge = rm.GetConcreteToGroundEdgeTile();

    // Bridge Tiles
    unsigned int texBridgeFloor = rm.GetBridgeFloorTile();
    unsigned int texBrokenBridge = rm.GetBrokenBridgeFloorTile();
    unsigned int texBrokenBridgeEdge = rm.GetBrokenBridgeEdgeTile();
    unsigned int texRiverWater = rm.GetRiverWaterTile();

    const int kTileWidth = 160;
    const int kTileHeight = 64;

    // Render River Water beneath the Broken Bridge gap (World X: 17500 to 19400)
    if (17500 - camX <= 1480 && 19400 - camX >= -200) {
        for (double wx = 17500; wx < 19400; wx += kTileWidth) {
            double screenWX = wx - camX;
            if (screenWX + kTileWidth >= -200 && screenWX <= 1480) {
                iShowImage((int)screenWX, 0, kTileWidth, 140, texRiverWater);
            }
        }
    }

    for (size_t i = 0; i < platforms.size(); ++i) {
        const Platform& p = platforms[i];

        double screenPx = p.x - camX;
        double screenPy = p.y - camY;

        if (screenPx + p.width >= -200 && screenPx <= 1480) {
            if (p.y > 200) {
                // --- Elevated Platforms ---
                int count = (int)(p.width / kTileWidth) + 1;
                for (int t = 0; t < count; ++t) {
                    double tileX = p.x + (t * kTileWidth) - camX;
                    double worldX = p.x + (t * kTileWidth);
                    double drawW = kTileWidth;
                    if (tileX + drawW > (p.x + p.width - camX)) {
                        drawW = (p.x + p.width - camX) - tileX;
                    }

                    if (tileX + drawW >= -200 && tileX <= 1480 && drawW > 0) {
                        unsigned int currentPlatformTex = texPlatform;

                        // Area-based platform styling
                        if (worldX >= 13000 && worldX < 15000) {
                            // Abandoned Church rafters / platform
                            currentPlatformTex = texChurchPlatform;
                        } else if (worldX >= 15000) {
                            // Quarantine Zone & Boss Arena elevated structures
                            currentPlatformTex = texHazardConcrete;
                        }

                        iShowImage((int)tileX, (int)(screenPy - 10), (int)drawW, 36, currentPlatformTex);
                    }
                }
            } else {
                // --- Main Ground Platforms ---
                int count = (int)(p.width / kTileWidth) + 1;
                for (int t = 0; t < count; ++t) {
                    double tileX = p.x + (t * kTileWidth) - camX;
                    double worldX = p.x + (t * kTileWidth);
                    double drawW = kTileWidth;
                    if (tileX + drawW > (p.x + p.width - camX)) {
                        drawW = (p.x + p.width - camX) - tileX;
                    }

                    if (tileX + drawW >= -200 && tileX <= 1480 && drawW > 0) {
                        unsigned int currentTex = texWood;

                        // 10 Level 1 Area Tile Themes
                        if (worldX < 3500) {
                            // Area 1 & 2: Spawn Area & Destroyed House (Wooden Floor)
                            currentTex = (t % 4 == 3) ? texBrokenWood : texWood;
                        } else if (worldX >= 3500 && worldX < 7500) {
                            // Area 3 & 4: Village Street & Square (Broken Road & Grass)
                            currentTex = (t % 3 == 0) ? texRoad : ((t % 3 == 1) ? texGrass : texDirt);
                        } else if (worldX >= 7500 && worldX < 12500) {
                            // Area 5 & 6: Abandoned Market & Raider Camp (Decayed Wood & Dirt)
                            currentTex = (t % 3 == 0) ? texBrokenWood : texWood;
                        } else if (worldX >= 12500 && worldX < 15000) {
                            // Area 7: Abandoned Church (Stone Floor)
                            currentTex = texChurchStone;
                        } else if (worldX >= 15000 && worldX < 17500) {
                            // Area 8: Quarantine Zone (Military Concrete & Hazard Paint)
                            currentTex = (t % 4 == 0) ? texHazardConcrete : texMilitaryConcrete;
                        } else if (worldX >= 17500 && worldX < 19400) {
                            // Area 9: Broken Bridge (Bridge Floor & Broken Edge)
                            currentTex = (t % 2 == 0) ? texBridgeFloor : texBrokenBridge;
                        } else if (worldX >= 19400) {
                            // Area 10 & 11: Boss Arena & Exit Gate (Cracked Military Concrete)
                            currentTex = (t % 3 == 0) ? texHazardConcrete : texCrackedConcrete;
                        }

                        // Boundary / Ledge Edge Alignment
                        if (t == count - 1 && p.width < 2000) {
                            if (worldX >= 12500 && worldX < 15000) {
                                currentTex = texChurchEdge;
                            } else if (worldX >= 15000 && worldX < 17500) {
                                currentTex = texConcreteEdge;
                            } else if (worldX >= 17500 && worldX < 19400) {
                                currentTex = texBrokenBridgeEdge;
                            } else {
                                currentTex = texEdge;
                            }
                        }

                        iShowImage((int)tileX, (int)(screenPy - 40), (int)drawW, kTileHeight, currentTex);
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

