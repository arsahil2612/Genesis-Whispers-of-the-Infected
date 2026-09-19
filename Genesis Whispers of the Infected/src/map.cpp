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
    const int kBgSliceHeight = 765; // Stretched slightly to fill top
    const int kScreenHeight = 720;
    const int kBgDrawYOffset = -45; // Shift down to close the transparent black gap with the ground

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
    currentLevelNumber = 1;
    l3Route = 0;

    // Parallax Factor Configurations
    parallaxFarFactor = 0.25;      // Layer 1: Sky & distant horizon (25% speed)
    parallaxMidFactor = 0.45;      // Layer 2: Midground scenery & trees (45% speed)
    parallaxGameplayFactor = 1.00; // Layer 3: Ground, platforms & world objects (100% speed)
    parallaxFgFactor = 1.15;       // Layer 5: Foreground rain & foliage (115% speed)
}

// ============================================================================
// Level Geometry & Collision Setup
// ============================================================================
void Map::LoadLevel(int levelNumber) {
    platforms.clear();
    currentLevelNumber = levelNumber;

    if (levelNumber == 1) {
        for (int i = 0; i < 10; ++i) {
            LoadLevel1BackgroundTexture(i, false);
        }

        // --- Level 1 Platform Geometry ---
        // Continuous Ground (World X: 0 to 14480, Top Surface Y = 185)
        platforms.push_back(Platform(0, 165, 14480, 20));
    }
    else if (levelNumber == 2) {
        for (int i = 0; i < 10; ++i) {
            LoadLevel2BackgroundTexture(i);
        }

        // --- Level 2 Ground & Platform Geometry (Blackwood Forest) ---
        // Section 1: Forest Entrance to Evacuation Camp Ground (World X: 0 to 14480, Top Surface Y = 185)
        platforms.push_back(Platform(0, 165, 14480, 20));
    }
    else if (levelNumber == 3) {
        // Preload common and initial route textures
        LoadLevel3BackgroundTexture(0, 0);
        for (int i = 0; i < 10; ++i) {
            LoadLevel3BackgroundTexture(1, i);
            LoadLevel3BackgroundTexture(2, i);
        }

        // Level 3 Continuous Ground
        platforms.push_back(Platform(0, 165, 14480, 20));
    }
}

// Helper to resolve texture for current active level
static unsigned int GetCurrentLevelBgTexture(int levelNumber, int sliceIndex, bool bossDefeated, int l3Route = 0) {
    if (levelNumber == 3) {
        return LoadLevel3BackgroundTexture(l3Route, sliceIndex);
    }
    if (levelNumber == 2) {
        return LoadLevel2BackgroundTexture(sliceIndex);
    }
    return LoadLevel1BackgroundTexture(sliceIndex, bossDefeated);
}

// ============================================================================
// LAYER 1: FAR BACKGROUND (Parallax Factor 0.25 - Sky & Distant Horizon)
// ============================================================================
void Map::RenderFarBackground(double camX, bool bossDefeated) {
    double farCamX = (currentLevelNumber == 3) ? camX : (camX * parallaxFarFactor);

    // Determine background camera offset with parallax scrolling across all levels
    double bgCamX = farCamX;

    // Render distant backdrop slices contiguously without overlapping vertical seams
    for (int i = 0; i < 10; ++i) {
        double xPos = (i * kBgSliceWidth) - bgCamX;

        if (xPos + kBgSliceWidth >= -200 && xPos <= 1480) {
            unsigned int tex = GetCurrentLevelBgTexture(currentLevelNumber, i, bossDefeated, l3Route);
            if (tex != 0) {
                int drawX = (int)floor(xPos);
                iShowImage(drawX, kBgDrawYOffset, kBgSliceWidth + 1, kBgSliceHeight, tex);
            }
        }
    }
}

// ============================================================================
// LAYER 2: MIDGROUND (Parallax Factor 0.45 - Medium-Distance Trees & Scenery)
// ============================================================================
void Map::RenderMidground(double camX, bool bossDefeated) {
    // Trees removed in Level 3 facility background
    if (currentLevelNumber == 3) return;

    double midCamX = camX * parallaxMidFactor;

    ResourceManager& rm = ResourceManager::GetInstance();
    unsigned int texTree = rm.GetTexture("Assets/Props/Level 1/Nature/nature_dead_tree_01.png");
    if (texTree == 0) {
        std::string resPath = GetAssetPath("Assets/Props/Level 1/Nature/nature_dead_tree_01.png");
        texTree = iLoadImage((char*)resPath.c_str());
    }

    if (texTree != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Render mid-distance tree silhouettes scrolling at 45% speed
        const double midTreeSpacing = 650.0;
        for (double wx = 120.0; wx < levelWidth; wx += midTreeSpacing) {
            double screenX = wx - midCamX;
            if (screenX + 300 >= -100 && screenX <= 1380) {
                iShowImage((int)screenX, 170, 260, 360, texTree);
            }
        }
    }
}

// ============================================================================
// LAYER 3: GAMEPLAY WORLD GROUND SURFACE (Parallax Factor 1.00 - Synchronized Ground)
// ============================================================================
void Map::RenderGroundSurface(double camX) {
    ResourceManager& rm = ResourceManager::GetInstance();

    std::string groundTilePath = (currentLevelNumber == 2)
        ? "Assets/Tiles/Ground/village_grass_tile.png"
        : "Assets/Tiles/Ground/dirt_tile.png";

    unsigned int texGround = rm.GetTexture(groundTilePath);
    if (texGround == 0) {
        std::string resolved = GetAssetPath(groundTilePath);
        texGround = iLoadImage((char*)resolved.c_str());
    }

    if (texGround == 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int tileSize = 185; // Ground Y=0 to 185 matching top collision surface Y=185

    for (size_t pIdx = 0; pIdx < platforms.size(); ++pIdx) {
        const Platform& plat = platforms[pIdx];

        // Draw ground tiles only on main walkable terrain platforms
        if (plat.y <= 170 && plat.width > 50) {
            for (double wx = plat.x; wx < plat.x + plat.width; wx += tileSize) {
                double screenX = wx - camX;
                if (screenX + tileSize >= -100 && screenX <= 1380) {
                    int drawW = tileSize;
                    if (wx + drawW > plat.x + plat.width) {
                        drawW = (int)(plat.x + plat.width - wx);
                    }
                    iShowImage((int)screenX, 0, drawW, tileSize, texGround);
                }
            }
        }
    }

    // Render soft natural ground-to-background edge blend transition line
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor4f(0.06f, 0.08f, 0.10f, 0.35f);
    glVertex2f(-100.0f, 178.0f);
    glVertex2f(1380.0f, 178.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    glVertex2f(1380.0f, 192.0f);
    glVertex2f(-100.0f, 192.0f);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

// ============================================================================
// LAYER 5: FOREGROUND ATMOSPHERIC PARALLAX (Parallax Factor 1.15 - Rain & Foliage)
// ============================================================================
void Map::RenderForeground(double camX, double animTime) {
    double fgCamX = camX * parallaxFgFactor;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Dynamic Atmospheric Rain Streaks
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.5f);
    glBegin(GL_LINES);

    const int numRainDrops = 65;
    for (int i = 0; i < numRainDrops; ++i) {
        double baseWorldX = (i * 220.0) + (sin(i * 13.0) * 80.0);
        double screenX = fmod(baseWorldX - fgCamX + (animTime * 650.0), 1400.0) - 100.0;
        double screenY = fmod(720.0 - (animTime * 950.0 + i * 45.0), 760.0);

        if (screenX >= -50 && screenX <= 1330) {
            float alpha = 0.25f + (float)(sin(i + animTime * 4.0) * 0.1f);
            glColor4f(0.70f, 0.85f, 1.0f, alpha);
            glVertex2f((float)screenX, (float)screenY);
            glVertex2f((float)(screenX - 8.0), (float)(screenY - 22.0));
        }
    }
    glEnd();

    // 2. Passing Translucent Overhead Foliage / Leaf Particles
    const int numLeaves = 20;
    for (int i = 0; i < numLeaves; ++i) {
        double baseWorldX = (i * 680.0) + (cos(i * 7.0) * 150.0);
        double screenX = fmod(baseWorldX - fgCamX + (animTime * 180.0), 1600.0) - 150.0;
        double screenY = 480.0 + (sin(animTime * 2.0 + i) * 60.0);

        if (screenX >= -50 && screenX <= 1330) {
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_TRIANGLES);
            glColor4f(0.20f, 0.45f, 0.15f, 0.35f);
            glVertex2f((float)screenX, (float)screenY);
            glVertex2f((float)(screenX + 12.0), (float)(screenY + 6.0));
            glVertex2f((float)(screenX + 6.0), (float)(screenY - 8.0));
            glEnd();
        }
    }
}

// Backward-compatible Background Renderer
void Map::RenderBackground(double camX, bool bossDefeated) {
    RenderFarBackground(camX, bossDefeated);
    RenderMidground(camX, bossDefeated);
}

// ============================================================================
// LAYER 2: WATER / RIVER RENDERING (River surface y=0 to 130)
// ============================================================================
void Map::RenderWater(double camX, double camY) {
    // River water removed per user request.
    return;
}

// ============================================================================
// LAYER 3: BRIDGE AND ENVIRONMENT SPRITES (Broken Bridge Structure)
// ============================================================================
void Map::RenderBridgeAndEnvironmentSprites(double camX, double camY) {
    if (currentLevelNumber != 1) return;
    
    // Bridge structure rendering has been removed to be replaced by the abandoned church.
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
