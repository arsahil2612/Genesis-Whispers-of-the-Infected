#include "ResourceManager.h"
#include "../iGraphics.h"
#include <stdio.h>

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
    ClearCache();
}

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

unsigned int ResourceManager::GetTexture(const std::string& filePath) {
    // Check if texture is already cached
    std::map<std::string, unsigned int>::iterator it = m_textureCache.find(filePath);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // Load texture using iGraphics iLoadImage
    unsigned int textureID = iLoadImage(filePath.c_str());
    if (textureID != 0) {
        m_textureCache[filePath] = textureID;
        printf("[ResourceManager] Loaded and cached texture: %s (ID: %u)\n", filePath.c_str(), textureID);
    } else {
        printf("[ResourceManager] WARNING: Failed to load texture: %s\n", filePath.c_str());
    }

    return textureID;
}

unsigned int ResourceManager::GetWoodFloorTile() {
    return GetTexture("Assets/Tiles/Wooden Tiles/wood_floor_tile.png");
}

unsigned int ResourceManager::GetBrokenWoodFloorTile() {
    return GetTexture("Assets/Tiles/Wooden Tiles/broken_wooden_floor_tile.png");
}

unsigned int ResourceManager::GetWoodFloorEdgeTile() {
    return GetTexture("Assets/Tiles/Wooden Tiles/wooden_floor_edge_tile_.png");
}

unsigned int ResourceManager::GetWoodPlatformTile() {
    return GetTexture("Assets/Tiles/Wooden Tiles/wooden_platform_tile.png");
}

unsigned int ResourceManager::GetDirtTile() {
    return GetTexture("Assets/Tiles/Ground/dirt_tile.png");
}

unsigned int ResourceManager::GetVillageGrassTile() {
    return GetTexture("Assets/Tiles/Ground/village_grass_tile.png");
}

unsigned int ResourceManager::GetChurchStoneFloorTile() {
    return GetTexture("Assets/Tiles/Church Tiles/church_stone_floor_tile.png");
}

unsigned int ResourceManager::GetChurchStoneFloorEdge() {
    return GetTexture("Assets/Tiles/Church Tiles/church_stone_floor_edge.png");
}

unsigned int ResourceManager::GetChurchStonePlatform() {
    return GetTexture("Assets/Tiles/Church Tiles/church_stone_platform.png");
}

unsigned int ResourceManager::GetMilitaryConcreteFloorTile() {
    return GetTexture("Assets/Tiles/Quarantine Zone Tiles/military_concrete_floor_tile.png");
}

unsigned int ResourceManager::GetCrackedMilitaryConcreteTile() {
    return GetTexture("Assets/Tiles/Quarantine Zone Tiles/cracked_military_conncrete_tile.png");
}

unsigned int ResourceManager::GetHazardMilitaryConcreteTile() {
    return GetTexture("Assets/Tiles/Quarantine Zone Tiles/hazard-painted_military_concrete_tile.png");
}

unsigned int ResourceManager::GetConcreteToGroundEdgeTile() {
    return GetTexture("Assets/Tiles/Quarantine Zone Tiles/concrete-to-ground-edge-tile.png");
}

unsigned int ResourceManager::GetBridgeFloorTile() {
    return GetTexture("Assets/Tiles/Bridges/bridge_floor.png");
}

unsigned int ResourceManager::GetBrokenBridgeFloorTile() {
    return GetTexture("Assets/Tiles/Bridges/broken_bridge_floor_tile_.png");
}

unsigned int ResourceManager::GetBrokenBridgeEdgeTile() {
    return GetTexture("Assets/Tiles/Bridges/broken_bridge_edge.png");
}

unsigned int ResourceManager::GetRiverWaterTile() {
    return GetTexture("Assets/Tiles/Bridges/River_water.png");
}

unsigned int ResourceManager::GetBrokenRoadTile() {
    return GetTexture("Assets/Tiles/Ground/broken_road.png");
}

unsigned int ResourceManager::GetConcreteGroundTile() {
    return GetTexture("Assets/Tiles/Ground/concrete_ground_tile.png");
}

void ResourceManager::ClearCache() {
    for (std::map<std::string, unsigned int>::iterator it = m_textureCache.begin(); it != m_textureCache.end(); ++it) {
        if (it->second != 0) {
            GLuint tex = (GLuint)it->second;
            glDeleteTextures(1, &tex);
        }
    }
    m_textureCache.clear();
    m_animationCache.clear();
}
