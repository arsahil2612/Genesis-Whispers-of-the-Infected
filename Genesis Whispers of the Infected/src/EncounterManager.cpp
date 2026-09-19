#include "EncounterManager.h"
#include "game_manager.h"
#include <cstdlib>
#include <cmath>

EncounterManager::EncounterManager() {
    m_globalCooldownUntil = 0;
    m_globalCooldownRemaining = 0.0f;
    m_currentLevelNumber = 1;
    Reset();
}

void EncounterManager::Reset() {
    m_difficultyDirector.Reset();
    m_spawnManager.Reset();
    m_envEventManager.Reset();
    m_globalCooldownRemaining = 0.0f;
    m_globalCooldownUntil = 0;

    for (size_t i = 0; i < m_zones.size(); ++i) {
        m_zones[i].triggered = false;
    }

    for (size_t i = 0; i < m_vehicles.size(); ++i) {
        m_vehicles[i].triggered = false;
    }
}

void EncounterManager::Initialize(int levelNumber) {
    m_currentLevelNumber = levelNumber;
    Reset();

    m_zones.clear();
    m_vehicles.clear();
    m_encounterTable.clear();

    // --- Weighted Encounter Table Setup ---
    // 1. Normal Encounter (Standard Front)
    EncounterDefinition e1;
    e1.id = 1;
    e1.category = ENCOUNTER_NORMAL;
    e1.enemyCount = 2;
    e1.enemies[0] = TYPE_SPITTER;
    e1.enemies[1] = TYPE_SPITTER;
    e1.pattern = PATTERN_FRONT_STANDARD;
    e1.minTier = 1; e1.maxTier = 3;
    e1.cooldownMs = 4000; e1.weight = 35;
    m_encounterTable.push_back(e1);

    // 2. Front Attack (Heavy/Raider Push)
    EncounterDefinition e2;
    e2.id = 2;
    e2.category = ENCOUNTER_FRONT_ATTACK;
    e2.enemyCount = 2;
    e2.enemies[0] = (levelNumber == 2) ? TYPE_HEAVY : TYPE_RAIDER;
    e2.enemies[1] = TYPE_SPITTER;
    e2.pattern = PATTERN_FRONT_STANDARD;
    e2.minTier = 2; e2.maxTier = 3;
    e2.cooldownMs = 5000; e2.weight = 25;
    m_encounterTable.push_back(e2);

    // 3. Rear Attack (Ambush from Behind)
    EncounterDefinition e3;
    e3.id = 3;
    e3.category = ENCOUNTER_REAR_ATTACK;
    e3.enemyCount = 1;
    e3.enemies[0] = (levelNumber == 2) ? TYPE_HUNTER : TYPE_RUNNER;
    e3.pattern = PATTERN_REAR_AMBUSH;
    e3.minTier = 1; e3.maxTier = 3;
    e3.cooldownMs = 6000; e3.weight = 25;
    m_encounterTable.push_back(e3);

    // 4. Small Ambush (Fast Flankers)
    EncounterDefinition e4;
    e4.id = 4;
    e4.category = ENCOUNTER_SMALL_AMBUSH;
    e4.enemyCount = 2;
    e4.enemies[0] = TYPE_RUNNER;
    e4.enemies[1] = TYPE_SPITTER;
    e4.pattern = PATTERN_FRONT_STANDARD;
    e4.minTier = 1; e4.maxTier = 3;
    e4.cooldownMs = 4500; e4.weight = 25;
    m_encounterTable.push_back(e4);

    // 5. Multi-Direction Encounter (Pincer Attack)
    EncounterDefinition e5;
    e5.id = 5;
    e5.category = ENCOUNTER_MULTI_DIRECTION;
    e5.enemyCount = 2;
    e5.enemies[0] = TYPE_RUNNER;  // Front
    e5.enemies[1] = TYPE_SPITTER; // Rear
    e5.pattern = PATTERN_PINCER;
    e5.minTier = 1; e5.maxTier = 3;
    e5.cooldownMs = 7000; e5.weight = 20;
    m_encounterTable.push_back(e5);

    // 6. Vehicle Ambush
    EncounterDefinition e6;
    e6.id = 6;
    e6.category = ENCOUNTER_VEHICLE_AMBUSH;
    e6.enemyCount = 2;
    e6.enemies[0] = TYPE_RAIDER;
    e6.enemies[1] = TYPE_RAIDER;
    e6.pattern = PATTERN_VEHICLE_AMBUSH;
    e6.minTier = 1; e6.maxTier = 3;
    e6.cooldownMs = 6000; e6.weight = 20;
    m_encounterTable.push_back(e6);

    // 7. Quiet Section (Pacing Rest)
    EncounterDefinition e7;
    e7.id = 7;
    e7.category = ENCOUNTER_QUIET_SECTION;
    e7.enemyCount = 0;
    e7.pattern = PATTERN_QUIET;
    e7.minTier = 1; e7.maxTier = 3;
    e7.cooldownMs = 3000; e7.weight = 20;
    m_encounterTable.push_back(e7);

    // 8. Environmental Event
    EncounterDefinition e8;
    e8.id = 8;
    e8.category = ENCOUNTER_ENVIRONMENTAL_EVENT;
    e8.enemyCount = 0;
    e8.pattern = PATTERN_ENVIRONMENTAL;
    e8.minTier = 1; e8.maxTier = 3;
    e8.cooldownMs = 3000; e8.weight = 15;
    m_encounterTable.push_back(e8);

    if (levelNumber == 1) {
        // Dynamic trigger zones for Level 1 (Fallen Village: width 14480px)
        // 1. Village Street (1448-2896): Front/Rear dynamic Walker/Runner encounter
        TriggerZone z1 = { 1600.0, 2200.0, ZONE_COMBAT, -1, false, false };
        // 2. Village Square (2896-4344): Dynamic reinforcement from behind or pincer
        TriggerZone z2 = { 3000.0, 3600.0, ZONE_COMBAT, -1, false, false };
        TriggerZone z3 = { 4000.0, 4300.0, ZONE_QUIET,  -1, false, false };
        // 3. Abandoned Market (4344-5792): Dynamic encounter (Walker/Runner/Raider)
        TriggerZone z4 = { 4800.0, 5400.0, ZONE_COMBAT, -1, false, false };
        // 4. Raider Camp (5792-7240): Environmental / Controlled Raider threat (do not flood with infected)
        TriggerZone z5 = { 6300.0, 6800.0, ZONE_ENVIRONMENTAL, 101, false, false };
        // 5. Abandoned Church (7240-8688): Rear encounter (Runner from behind)
        TriggerZone z6 = { 7600.0, 8200.0, ZONE_COMBAT, -1, false, false };
        // 6. Quarantine Zone (8688-10136): Dynamic reinforcement (Walkers)
        TriggerZone z7 = { 8900.0, 9500.0, ZONE_COMBAT, -1, false, false };
        // 7. Broken Bridge (10136-11584): QUIET (Controlled environment - no unfair dynamic combat spawns during platforming)
        TriggerZone z8 = { 10200.0, 10800.0, ZONE_QUIET, -1, false, false };
        // 8. Exit Gate (13032-14480): QUIET (Preserve story ending)
        TriggerZone z9 = { 13200.0, 13800.0, ZONE_QUIET, -1, false, false };

        m_zones.push_back(z1); m_zones.push_back(z2); m_zones.push_back(z3); m_zones.push_back(z4);
        m_zones.push_back(z5); m_zones.push_back(z6); m_zones.push_back(z7); m_zones.push_back(z8);
        m_zones.push_back(z9);

        // Section-appropriate hidden enemy types for Level 1 vehicles
        static const EnemyType level1VehicleEnemies[4] = { TYPE_SPITTER, TYPE_RAIDER, TYPE_RAIDER, TYPE_RUNNER };
        double vehiclePositions[] = { 4250.0, 5280.0, 6150.0, 8280.0 };
        for (int i = 0; i < 4; ++i) {
            VehicleSpawnPoint vp;
            vp.x = vehiclePositions[i];
            vp.y = 185.0;
            vp.isHot = (i == 0 || i == 3) ? true : (rand() % 2 == 0); // Controlled hot vehicle chance
            vp.triggered = false;
            vp.isWarningActive = false;
            vp.warningTimer = 0.0f;
            vp.hiddenType = level1VehicleEnemies[i];
            vp.hiddenCount = 1;
            m_vehicles.push_back(vp);
        }
    }
    else if (levelNumber == 2) {
        // Dynamic trigger zones for Level 2 (Blackwood Forest: width 14480px)
        TriggerZone z1 = { 1000.0, 1400.0, ZONE_COMBAT, -1, false, false };  // Forest Entrance -> Evacuation Camp
        TriggerZone z2 = { 2400.0, 2800.0, ZONE_COMBAT, -1, false, false };  // Evacuation Camp
        TriggerZone z3 = { 4800.0, 5200.0, ZONE_COMBAT, -1, false, false };  // Deep Forest
        TriggerZone z4 = { 6000.0, 6400.0, ZONE_COMBAT, -1, false, false };  // River Crossing
        TriggerZone z5 = { 7400.0, 8400.0, ZONE_QUIET,  -1, false, false };  // Survivor Hideout (Safe atmosphere)
        TriggerZone z6 = { 9000.0, 9400.0, ZONE_COMBAT, -1, false, false };  // Infected Forest (Front push)
        TriggerZone z7 = { 9600.0, 10000.0, ZONE_COMBAT, -1, false, false }; // Infected Forest (Rear Hunter ambush)
        TriggerZone z8 = { 10400.0, 10800.0, ZONE_COMBAT, -1, false, false };// NovaGen Outpost
        TriggerZone z9 = { 12000.0, 12400.0, ZONE_COMBAT, -1, false, false };// Research Facility

        m_zones.push_back(z1); m_zones.push_back(z2); m_zones.push_back(z3); m_zones.push_back(z4);
        m_zones.push_back(z5); m_zones.push_back(z6); m_zones.push_back(z7); m_zones.push_back(z8);
        m_zones.push_back(z9);

        static const EnemyType level2VehicleEnemies[4] = { TYPE_SPITTER, TYPE_SPITTER, TYPE_SPITTER, TYPE_HEAVY };
        double vehiclePositions[] = { 850.0, 2200.0, 6150.0, 10200.0 };
        for (int i = 0; i < 4; ++i) {
            VehicleSpawnPoint vp;
            vp.x = vehiclePositions[i];
            vp.y = 185.0;
            vp.isHot = true;
            vp.triggered = false;
            vp.isWarningActive = false;
            vp.warningTimer = 0.0f;
            vp.hiddenType = level2VehicleEnemies[i];
            vp.hiddenCount = 1;
            m_vehicles.push_back(vp);
        }
    }
}

void EncounterManager::Update(Player& player, Map& map, GameManager& gm, float dt) {
    m_difficultyDirector.Evaluate(player, m_spawnManager.GetMemory());
    m_envEventManager.Update(dt);

    // Global Cooldown Check
    if (m_globalCooldownRemaining > 0.0f) {
        m_globalCooldownRemaining -= dt;
        if (m_globalCooldownRemaining < 0.0f) {
            m_globalCooldownRemaining = 0.0f;
        }
        return; // Do not trigger zones while global cooldown is active
    }

    CheckTriggerZones(player, map, gm);

    if (m_globalCooldownRemaining <= 0.0f) {
        CheckVehicleAmbushes(player, map, gm);
    }
}

void EncounterManager::CheckTriggerZones(Player& player, Map& map, GameManager& gm) {
    for (size_t i = 0; i < m_zones.size(); ++i) {
        TriggerZone& z = m_zones[i];
        if (z.triggered && !z.repeatable) continue;

        if (player.x >= z.startX) {
            if (z.type == ZONE_QUIET) {
                z.triggered = true;
                m_globalCooldownRemaining = 2.0f;
            }
            else if (z.type == ZONE_ENVIRONMENTAL) {
                z.triggered = true;
                m_envEventManager.Fire(z.encounterID);
                m_globalCooldownRemaining = 2.0f;
            }
            else if (z.type == ZONE_COMBAT) {
                int currentTier = m_difficultyDirector.GetTier();
                EncounterDefinition def = SelectWeightedEncounter(z.type, currentTier, z.encounterID);

                if (def.enemyCount == 0) {
                    z.triggered = true;
                    m_globalCooldownRemaining = (float)def.cooldownMs / 1000.0f;
                    break;
                }

                bool spawned = m_spawnManager.TrySpawnEncounter(def, player, map, gm);
                if (spawned) {
                    z.triggered = true;
                    m_globalCooldownRemaining = (float)def.cooldownMs / 1000.0f;
                    break; // Trigger only one encounter per frame
                }
            }
        }
    }
}

void EncounterManager::CheckVehicleAmbushes(Player& player, Map& map, GameManager& gm) {
    for (size_t i = 0; i < m_vehicles.size(); ++i) {
        VehicleSpawnPoint& vp = m_vehicles[i];
        if (!vp.isHot || vp.triggered) continue;

        // Active warning phase: count down before enemy emerges from vehicle
        if (vp.isWarningActive) {
            vp.warningTimer -= 0.016f;
            if (vp.warningTimer <= 0.0f) {
                vp.warningTimer = 0.0f;
                bool spawned = m_spawnManager.TrySpawnVehicleAmbush(vp, player, map, gm);
                if (spawned) {
                    vp.triggered = true;
                    vp.isWarningActive = false;
                    m_globalCooldownRemaining = 5.0f; // 5 second cooldown after vehicle ambush
                    break;
                } else {
                    vp.isWarningActive = false;
                }
            }
            continue;
        }

        // Trigger warning state when Arin gets within 220px of an eligible hot vehicle
        if (std::abs(player.x - vp.x) <= 220.0) {
            if (gm.GetActiveEnemyCount() + vp.hiddenCount <= 4 && m_globalCooldownRemaining <= 0.0f) {
                vp.isWarningActive = true;
                vp.warningTimer = 0.6f; // 0.6s warning shake/cue before emergence
                break;
            }
        }
    }
}

EncounterDefinition EncounterManager::SelectWeightedEncounter(ZoneType zType, int tier, int requestedID) {
    (void)requestedID;
    if (zType == ZONE_QUIET) {
        for (size_t i = 0; i < m_encounterTable.size(); ++i) {
            if (m_encounterTable[i].category == ENCOUNTER_QUIET_SECTION) return m_encounterTable[i];
        }
    }
    if (zType == ZONE_ENVIRONMENTAL) {
        for (size_t i = 0; i < m_encounterTable.size(); ++i) {
            if (m_encounterTable[i].category == ENCOUNTER_ENVIRONMENTAL_EVENT) return m_encounterTable[i];
        }
    }

    const SpawnMemory& memory = m_spawnManager.GetMemory();

    // Check if player had 2 consecutive combat encounters; force a quiet section for pacing
    if (memory.consecutiveCombatSpawns >= 2) {
        for (size_t i = 0; i < m_encounterTable.size(); ++i) {
            if (m_encounterTable[i].category == ENCOUNTER_QUIET_SECTION) return m_encounterTable[i];
        }
    }

    std::vector<EncounterDefinition> candidates;
    std::vector<int> weights;
    int totalWeight = 0;

    for (size_t i = 0; i < m_encounterTable.size(); ++i) {
        const EncounterDefinition& def = m_encounterTable[i];

        if (def.enemyCount == 0 && zType == ZONE_COMBAT) continue; // Skip non-combat in combat zone unless forced

        if (tier >= def.minTier && tier <= def.maxTier) {
            int effectiveWeight = def.weight;

            // Reduce weight if same category as last encounter
            if (def.category == memory.lastCategory) {
                effectiveWeight = effectiveWeight / 4;
            }

            // Exclude rear/pincer if rear was recently used
            if ((def.category == ENCOUNTER_REAR_ATTACK || def.category == ENCOUNTER_MULTI_DIRECTION || def.pattern == PATTERN_REAR_AMBUSH || def.pattern == PATTERN_PINCER)
                && (memory.consecutiveRearSpawns > 0 || memory.lastPattern == PATTERN_REAR_AMBUSH || memory.lastPattern == PATTERN_PINCER)) {
                effectiveWeight = 0;
            }

            if (effectiveWeight > 0) {
                candidates.push_back(def);
                weights.push_back(effectiveWeight);
                totalWeight += effectiveWeight;
            }
        }
    }

    if (candidates.empty() || totalWeight <= 0) {
        // Fallback default encounter
        EncounterDefinition fallback;
        fallback.id = 1;
        fallback.category = ENCOUNTER_NORMAL;
        fallback.enemyCount = 1;
        fallback.enemies[0] = TYPE_SPITTER;
        fallback.pattern = PATTERN_FRONT_STANDARD;
        fallback.minTier = 1; fallback.maxTier = 3;
        fallback.cooldownMs = 4000; fallback.weight = 35;
        return fallback;
    }

    int roll = rand() % totalWeight;
    int currentSum = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        currentSum += weights[i];
        if (roll < currentSum) {
            return candidates[i];
        }
    }

    return candidates[0];
}
