#include "TileManager.h"
#include "ResourceManager.h"
#include "asset_loader.h"
#include "map.h"
#include "../iGraphics.h"
#include <cstdio>
#include <GL/gl.h>

// ============================================================================
// Constructor & Destructor
// ============================================================================
TileManager::TileManager() {
}

TileManager::~TileManager() {
    ClearTiles();
}

// ============================================================================
// Initialization & Cleanup
// ============================================================================
void TileManager::Initialize() {
    ClearTiles();
}

void TileManager::ClearTiles() {
    tiles.clear();
}

// ============================================================================
// Tile Registration
// ============================================================================
void TileManager::AddTile(const std::string& assetPath, double x, double y, double width, double height, 
                          TileType type, TileCollisionType collisionType, int renderLayer) {
    TileInstance tile;
    tile.assetPath = assetPath;
    
    // Attempt to load the tile texture using ResourceManager
    std::string fullPath = "Assets/Tiles/" + assetPath;
    tile.textureID = ResourceManager::GetInstance().GetTexture(fullPath);
    
    // If ResourceManager failed to find it in cache, fallback to immediate iLoadImage
    if (tile.textureID == 0) {
        std::string absolutePath = GetAssetPath(fullPath);
        tile.textureID = iLoadImage((char*)absolutePath.c_str());
    }

    tile.x = x;
    tile.y = y;
    tile.width = width;
    tile.height = height;
    tile.type = type;
    tile.collisionType = collisionType;
    tile.renderLayer = renderLayer;

    tiles.push_back(tile);
}

// ============================================================================
// Internal Rendering Helper
// ============================================================================
static void RenderTileInstances(const std::vector<TileInstance>& tiles, int targetLayer, double camX, double camY) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].renderLayer == targetLayer && tiles[i].textureID != 0) {
            double screenX = tiles[i].x - camX;
            double screenY = tiles[i].y - camY;

            // Frustum Culling (Viewport horizontal bounds: -200 to 1480 for a 1280 wide screen)
            if (screenX + tiles[i].width >= -200 && screenX <= 1480) {
                iShowImage((int)screenX, (int)screenY, (int)tiles[i].width, (int)tiles[i].height, tiles[i].textureID);
            }
        }
    }
}

// ============================================================================
// Layer-Specific Rendering Passes
// ============================================================================
void TileManager::RenderFarEnvironmentTiles(double camX, double camY) {
    RenderTileInstances(tiles, 2, camX, camY); // Layer 2: Far Environment (Nature, Backdrop)
}

void TileManager::RenderGroundTiles(double camX, double camY) {
    RenderTileInstances(tiles, 3, camX, camY); // Layer 3: Ground / Stone / Concrete
}

void TileManager::RenderPlatformAndBridgeTiles(double camX, double camY) {
    RenderTileInstances(tiles, 4, camX, camY); // Layer 4: Platforms / Bridges
}

// ============================================================================
// Collision Generation Helper
// ============================================================================
void TileManager::ExportCollisionPlatforms(std::vector<Platform>& outPlatforms) const {
    for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].collisionType == COLLISION_SOLID || tiles[i].collisionType == COLLISION_ONE_WAY) {
            Platform p;
            p.x = tiles[i].x;
            p.y = tiles[i].y;
            p.width = tiles[i].width;
            p.height = tiles[i].height;
            outPlatforms.push_back(p);
        }
    }
}
