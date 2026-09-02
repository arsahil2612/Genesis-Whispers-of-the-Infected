#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <string>
#include <vector>

// ============================================================================
// Tile Types & Properties
// ============================================================================
enum TileType {
    TILE_GROUND,
    TILE_STONE,
    TILE_WOODEN,
    TILE_PLATFORM,
    TILE_BRIDGES,
    TILE_SPECIAL_SURFACE,
    TILE_MILITARY_CONCRETE,
    TILE_NATURE,
    TILE_ROADS,
    TILE_QUARANTINE_ZONE,
    TILE_CHURCH
};

enum TileCollisionType {
    COLLISION_NONE,         // Purely visual / decorative (e.g., Nature bushes)
    COLLISION_SOLID,        // Solid block/surface (impassable from sides, walkable on top)
    COLLISION_ONE_WAY       // Pass-through from below, solid landing on top (Platform)
};

// ============================================================================
// Tile Instance Structure
// ============================================================================
struct TileInstance {
    std::string assetPath;     // Relative path under Assets/Tiles/ (e.g., "Ground/dirt_tile.png")
    unsigned int textureID;    // Loaded OpenGL texture handle
    double x, y;               // World position (bottom-left coordinate)
    double width, height;      // Render dimensions
    TileType type;             // Tile classification
    TileCollisionType collisionType; // Collision behavior
    int renderLayer;           // Rendering order (2: Far Env, 3: Ground, 4: Platforms/Bridges)
};

// Forward declaration of Map Platform struct to avoid circular dependency
struct Platform;

// ============================================================================
// Tile Manager Class
// ============================================================================
class TileManager {
private:
    std::vector<TileInstance> tiles;

public:
    TileManager();
    ~TileManager();

    // Tile lifecycle
    void Initialize();
    void ClearTiles();

    // Add a new tile to the world
    void AddTile(const std::string& assetPath, double x, double y, double width, double height, 
                 TileType type, TileCollisionType collisionType, int renderLayer);

    // Render passes per layer
    void RenderFarEnvironmentTiles(double camX, double camY);
    void RenderGroundTiles(double camX, double camY);
    void RenderPlatformAndBridgeTiles(double camX, double camY);

    // Collision generation helper
    void ExportCollisionPlatforms(std::vector<Platform>& outPlatforms) const;
};

#endif // TILE_MANAGER_H
