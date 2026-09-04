#ifndef ENCOUNTER_MANAGER_H
#define ENCOUNTER_MANAGER_H

#include "EncounterTypes.h"
#include "DifficultyDirector.h"
#include "SpawnManager.h"
#include "EnvironmentalEventManager.h"
#include "player.h"
#include "map.h"
#include <vector>

class GameManager; // Forward declaration

// ============================================================================
// Top-Level Dynamic Encounter Manager
// Orchestrates level zones, vehicle ambushes, global cooldowns, & difficulty
// ============================================================================

class EncounterManager {
public:
    EncounterManager();

    void Initialize(int levelNumber);
    void Update(Player& player, Map& map, GameManager& gm, float dt);
    void Reset();

    DifficultyDirector& GetDifficultyDirector() { return m_difficultyDirector; }
    const SpawnMemory& GetSpawnMemory() const { return m_spawnManager.GetMemory(); }
    const std::vector<VehicleSpawnPoint>& GetVehicles() const { return m_vehicles; }

private:
    void CheckTriggerZones(Player& player, Map& map, GameManager& gm);
    void CheckVehicleAmbushes(Player& player, Map& map, GameManager& gm);
    EncounterDefinition SelectWeightedEncounter(ZoneType zType, int tier, int requestedID = -1);

    DifficultyDirector        m_difficultyDirector;
    SpawnManager               m_spawnManager;
    EnvironmentalEventManager   m_envEventManager;
    std::vector<TriggerZone>   m_zones;
    std::vector<VehicleSpawnPoint> m_vehicles;
    std::vector<EncounterDefinition> m_encounterTable;
    
    unsigned long m_globalCooldownUntil;
    float m_globalCooldownRemaining;
    int m_currentLevelNumber;
};

#endif // ENCOUNTER_MANAGER_H
