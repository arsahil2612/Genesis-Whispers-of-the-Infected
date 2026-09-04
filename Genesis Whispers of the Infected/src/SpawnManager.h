#ifndef SPAWN_MANAGER_H
#define SPAWN_MANAGER_H

#include "EncounterTypes.h"
#include "player.h"
#include "map.h"
#include <cmath>

class GameManager; // Forward declaration to break circular dependency

// ============================================================================
// Spawn Validation & Enemy Creation Manager
// Enforces max enemy cap, safe spawn distance, and platform validation
// ============================================================================

class SpawnManager {
public:
    SpawnManager();

    bool TrySpawnEncounter(EncounterDefinition def, Player& player, Map& map, GameManager& gm);
    bool TrySpawnVehicleAmbush(const VehicleSpawnPoint& v, Player& player, Map& map, GameManager& gm);
    const SpawnMemory& GetMemory() const { return m_memory; }
    void Reset();

private:
    double ResolveSpawnX(SpawnPattern pattern, Player& player, Map& map, int enemyIndex, int totalEnemies, double vehicleX = 0.0);
    bool IsSpawnPositionValid(double spawnX, Map& map, Player& player);
    void TelegraphRearSpawn(EncounterCategory category, GameManager& gm);

    SpawnMemory m_memory;
    static const int MAX_CONCURRENT_ENEMIES = 4;
    static const int MIN_SPAWN_DISTANCE = 300;
};

#endif // SPAWN_MANAGER_H
