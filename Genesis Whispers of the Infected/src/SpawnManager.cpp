#include "SpawnManager.h"
#include "game_manager.h"
#include <cstdio>
#include <cstdlib>

SpawnManager::SpawnManager() {
    Reset();
}

void SpawnManager::Reset() {
    m_memory.lastPattern = PATTERN_FRONT_STANDARD;
    m_memory.lastCategory = ENCOUNTER_NORMAL;
    m_memory.consecutiveRearSpawns = 0;
    m_memory.consecutiveCombatSpawns = 0;
    m_memory.totalSpawnsThisLevel = 0;
    m_memory.dynamicHeavyCount = 0;
}

bool SpawnManager::TrySpawnEncounter(EncounterDefinition def, Player& player, Map& map, GameManager& gm) {
    if (def.enemyCount <= 0) return false;

    // Cap dynamic Heavy Infected spawns in Level 1 to max 1 (preserving 2-3 total Heavy Infected)
    for (int i = 0; i < def.enemyCount; ++i) {
        if (def.enemies[i] == TYPE_HEAVY) {
            if (m_memory.dynamicHeavyCount >= 1) {
                def.enemies[i] = TYPE_RUNNER; // Substitute with Runner if Heavy budget reached
            }
        }
    }

    // Constraint 2: Check max active concurrent enemies
    if (gm.GetActiveEnemyCount() + def.enemyCount > MAX_CONCURRENT_ENEMIES) {
        return false;
    }

    // Constraint 3: Prevent consecutive rear ambushes & multi-direction pincers
    SpawnPattern effectivePattern = def.pattern;
    if (effectivePattern == PATTERN_REAR_AMBUSH || effectivePattern == PATTERN_PINCER) {
        if (m_memory.lastPattern == PATTERN_REAR_AMBUSH || 
            m_memory.lastPattern == PATTERN_PINCER || 
            m_memory.consecutiveRearSpawns > 0) {
            // Force downgrade to front standard
            effectivePattern = PATTERN_FRONT_STANDARD;
        }
    }

    // Constraint 6: Validate ALL proposed spawn positions before spawning any enemy
    double proposedPositions[4];
    bool valid = true;
    for (int i = 0; i < def.enemyCount; ++i) {
        proposedPositions[i] = ResolveSpawnX(effectivePattern, player, map, i, def.enemyCount);
        if (!IsSpawnPositionValid(proposedPositions[i], map, player)) {
            valid = false;
            break;
        }
    }

    // Boundary Fallback: If proposed spawn is blocked by level boundary, attempt flipping orientation
    if (!valid) {
        SpawnPattern flippedPattern = effectivePattern;
        if (effectivePattern == PATTERN_FRONT_STANDARD) flippedPattern = PATTERN_REAR_AMBUSH;
        else if (effectivePattern == PATTERN_REAR_AMBUSH) flippedPattern = PATTERN_FRONT_STANDARD;

        valid = true;
        for (int i = 0; i < def.enemyCount; ++i) {
            proposedPositions[i] = ResolveSpawnX(flippedPattern, player, map, i, def.enemyCount);
            if (!IsSpawnPositionValid(proposedPositions[i], map, player)) {
                valid = false;
                break;
            }
        }
        if (valid) {
            effectivePattern = flippedPattern;
        }
    }

    if (!valid) {
        return false;
    }

    // Telegraph alert cue for rear ambushes and multi-direction encounters to give player reaction time
    if (effectivePattern == PATTERN_REAR_AMBUSH || effectivePattern == PATTERN_PINCER || def.category == ENCOUNTER_REAR_ATTACK || def.category == ENCOUNTER_MULTI_DIRECTION) {
        TelegraphRearSpawn(def.category, gm);
    }

    // Execute atomic spawn
    for (int i = 0; i < def.enemyCount; ++i) {
        double sx = proposedPositions[i];
        double patrolStart = sx - 50.0;
        double patrolEnd = sx + 50.0;
        Enemy e(patrolStart, patrolEnd, 185.0, def.enemies[i]);
        gm.AddEnemy(e);
        if (def.enemies[i] == TYPE_HEAVY) {
            m_memory.dynamicHeavyCount++;
        }
    }

    // Update spawn memory
    m_memory.lastPattern = effectivePattern;
    m_memory.lastCategory = def.category;
    m_memory.totalSpawnsThisLevel += def.enemyCount;
    if (effectivePattern == PATTERN_REAR_AMBUSH || effectivePattern == PATTERN_PINCER) {
        m_memory.consecutiveRearSpawns++;
    } else {
        m_memory.consecutiveRearSpawns = 0;
    }
    m_memory.consecutiveCombatSpawns++;

    return true;
}

bool SpawnManager::TrySpawnVehicleAmbush(const VehicleSpawnPoint& v, Player& player, Map& map, GameManager& gm) {
    if (!v.isHot || v.triggered || v.hiddenCount <= 0) return false;
    if (gm.GetActiveEnemyCount() + v.hiddenCount > MAX_CONCURRENT_ENEMIES) return false;

    EncounterDefinition vDef;
    vDef.id = 9999;
    vDef.category = ENCOUNTER_VEHICLE_AMBUSH;
    vDef.enemyCount = v.hiddenCount;
    vDef.pattern = PATTERN_VEHICLE_AMBUSH;
    vDef.minTier = 1;
    vDef.maxTier = 3;
    vDef.cooldownMs = 3000;

    EnemyType actualType = v.hiddenType;
    if (actualType == TYPE_HEAVY) {
        if (m_memory.dynamicHeavyCount >= 1) {
            actualType = TYPE_RUNNER;
        }
    }

    for (int i = 0; i < v.hiddenCount; ++i) {
        vDef.enemies[i] = actualType;
    }

    double proposedPositions[4];
    for (int i = 0; i < v.hiddenCount; ++i) {
        proposedPositions[i] = ResolveSpawnX(PATTERN_VEHICLE_AMBUSH, player, map, i, v.hiddenCount, v.x);
        if (!IsSpawnPositionValid(proposedPositions[i], map, player)) {
            return false;
        }
    }

    TelegraphRearSpawn(ENCOUNTER_VEHICLE_AMBUSH, gm);

    for (int i = 0; i < v.hiddenCount; ++i) {
        double sx = proposedPositions[i];
        Enemy e(sx - 50.0, sx + 50.0, 185.0, actualType);
        gm.AddEnemy(e);
        if (actualType == TYPE_HEAVY) {
            m_memory.dynamicHeavyCount++;
        }
    }

    m_memory.lastPattern = PATTERN_VEHICLE_AMBUSH;
    m_memory.lastCategory = ENCOUNTER_VEHICLE_AMBUSH;
    m_memory.consecutiveRearSpawns = 0;
    m_memory.consecutiveCombatSpawns++;
    m_memory.totalSpawnsThisLevel += v.hiddenCount;
    return true;
}

double SpawnManager::ResolveSpawnX(SpawnPattern pattern, Player& player, Map& map, int enemyIndex, int totalEnemies, double vehicleX) {
    (void)map;
    double frontDir = player.isFacingRight ? 1.0 : -1.0;
    double rearDir  = player.isFacingRight ? -1.0 : 1.0;

    double spawnX = player.x + (frontDir * 380.0); // Facing-aware default fallback

    switch (pattern) {
    case PATTERN_FRONT_STANDARD:
        spawnX = player.x + frontDir * ((double)MIN_SPAWN_DISTANCE + 80.0 + (enemyIndex * 90.0));
        break;

    case PATTERN_REAR_AMBUSH:
        spawnX = player.x + rearDir * ((double)MIN_SPAWN_DISTANCE + 80.0 + (enemyIndex * 90.0));
        break;

    case PATTERN_VEHICLE_AMBUSH:
        spawnX = vehicleX + (enemyIndex * 70.0) - 35.0;
        break;

    case PATTERN_PINCER:
        if (enemyIndex < (totalEnemies + 1) / 2) {
            // Front attacker facing player's line of sight
            spawnX = player.x + frontDir * ((double)MIN_SPAWN_DISTANCE + 80.0 + (enemyIndex * 90.0));
        } else {
            // Rear attacker approaching player's back
            int rearIdx = enemyIndex - ((totalEnemies + 1) / 2);
            spawnX = player.x + rearDir * ((double)MIN_SPAWN_DISTANCE + 80.0 + (rearIdx * 90.0));
        }
        break;

    default:
        spawnX = player.x + frontDir * ((double)MIN_SPAWN_DISTANCE + 80.0);
        break;
    }

    return spawnX;
}

bool SpawnManager::IsSpawnPositionValid(double spawnX, Map& map, Player& player) {
    // Constraint 1: Minimum distance to player
    if (std::abs(spawnX - player.x) < (double)MIN_SPAWN_DISTANCE) {
        return false;
    }

    // Map bounds check
    int mapWidth = map.GetLevelWidth();
    if (spawnX < 50.0 || spawnX > (double)mapWidth - 50.0) {
        return false;
    }

    // Constraint 6: Check if spawn position lands on valid ground platform
    const std::vector<Platform>& platforms = map.GetPlatforms();
    bool onValidGround = false;
    for (size_t i = 0; i < platforms.size(); ++i) {
        const Platform& p = platforms[i];
        if (spawnX >= p.x && spawnX <= (p.x + p.width)) {
            onValidGround = true;
            break;
        }
    }

    return onValidGround;
}

void SpawnManager::TelegraphRearSpawn(EncounterCategory category, GameManager& gm) {
    // Trigger on-screen warning alert for reaction time
    (void)category;
    (void)gm;
}
