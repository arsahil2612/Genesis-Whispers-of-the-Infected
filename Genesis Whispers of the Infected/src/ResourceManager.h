#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <map>
#include <vector>

// ============================================================================
// RESOURCE MANAGER CLASS (PHASE 2 - SINGLETON ASSET CACHE)
// ============================================================================
class ResourceManager {
private:
    std::map<std::string, unsigned int> m_textureCache;
    std::map<std::string, std::vector<unsigned int> > m_animationCache;

    ResourceManager();
    ~ResourceManager();

    // Prevent copy construction and assignment operator
    ResourceManager(const ResourceManager&);
    ResourceManager& operator=(const ResourceManager&);

public:
    static ResourceManager& GetInstance();

    // Core Texture Management (Loads every image once, caches & provides global access)
    unsigned int GetTexture(const std::string& filePath);

    // Tile Texture Helper Methods for Level 1 Tiles
    unsigned int GetWoodFloorTile();
    unsigned int GetBrokenWoodFloorTile();
    unsigned int GetWoodFloorEdgeTile();
    unsigned int GetWoodPlatformTile();
    unsigned int GetDirtTile();
    unsigned int GetVillageGrassTile();

    // Church Tiles
    unsigned int GetChurchStoneFloorTile();
    unsigned int GetChurchStoneFloorEdge();
    unsigned int GetChurchStonePlatform();

    // Quarantine Zone & Military Concrete Tiles
    unsigned int GetMilitaryConcreteFloorTile();
    unsigned int GetCrackedMilitaryConcreteTile();
    unsigned int GetHazardMilitaryConcreteTile();
    unsigned int GetConcreteToGroundEdgeTile();

    // Bridge Tiles
    unsigned int GetBridgeFloorTile();
    unsigned int GetBrokenBridgeFloorTile();
    unsigned int GetBrokenBridgeEdgeTile();
    unsigned int GetRiverWaterTile();
    unsigned int GetWoodenBridgeSupportBeamTile();
    unsigned int GetRainSoakedBrokenBridgeTile();

    // Road & Ground Tiles
    unsigned int GetBrokenRoadTile();
    unsigned int GetConcreteGroundTile();
    unsigned int GetLevel3Tile();

    // Cache management
    void ClearCache();
};

#endif // RESOURCE_MANAGER_H
