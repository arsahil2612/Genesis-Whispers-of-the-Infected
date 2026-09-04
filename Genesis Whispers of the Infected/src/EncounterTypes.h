#ifndef ENCOUNTER_TYPES_H
#define ENCOUNTER_TYPES_H

#include "enemy.h"

// ============================================================================
// Dynamic Encounter System Enums & Data Structures
// ============================================================================

enum SpawnPattern {
    PATTERN_FRONT_STANDARD,
    PATTERN_REAR_AMBUSH,
    PATTERN_VEHICLE_AMBUSH,
    PATTERN_PINCER,
    PATTERN_ENVIRONMENTAL,
    PATTERN_QUIET
};

enum ZoneType {
    ZONE_COMBAT,
    ZONE_ENVIRONMENTAL,
    ZONE_QUIET
};

enum EncounterCategory {
    ENCOUNTER_NORMAL,
    ENCOUNTER_FRONT_ATTACK,
    ENCOUNTER_REAR_ATTACK,
    ENCOUNTER_SMALL_AMBUSH,
    ENCOUNTER_MULTI_DIRECTION,
    ENCOUNTER_VEHICLE_AMBUSH,
    ENCOUNTER_QUIET_SECTION,
    ENCOUNTER_ENVIRONMENTAL_EVENT
};

struct EncounterDefinition {
    int id;
    EncounterCategory category;
    EnemyType enemies[4];
    int enemyCount;
    SpawnPattern pattern;
    int minTier;
    int maxTier;
    int cooldownMs;
    int weight; // Selection probability weight
};

struct TriggerZone {
    double startX, endX;
    ZoneType type;
    int encounterID;      // -1 if dynamically weighted selection
    bool triggered;
    bool repeatable;
};

struct VehicleSpawnPoint {
    double x, y;
    bool isHot;
    bool triggered;
    bool isWarningActive;
    float warningTimer;
    EnemyType hiddenType;
    int hiddenCount;

    VehicleSpawnPoint()
        : x(0), y(0), isHot(false), triggered(false)
        , isWarningActive(false), warningTimer(0.0f)
        , hiddenType(TYPE_RAIDER), hiddenCount(1) {}
};

struct DifficultyState {
    int tier;
    double playerHpPercent;
    int deathsThisLevel;
    int encountersSurvivedCleanly;
};

struct SpawnMemory {
    SpawnPattern lastPattern;
    EncounterCategory lastCategory;
    int consecutiveRearSpawns;
    int consecutiveCombatSpawns;
    int totalSpawnsThisLevel;
    int dynamicHeavyCount;

    SpawnMemory() 
        : lastPattern(PATTERN_FRONT_STANDARD)
        , lastCategory(ENCOUNTER_NORMAL)
        , consecutiveRearSpawns(0)
        , consecutiveCombatSpawns(0)
        , totalSpawnsThisLevel(0)
        , dynamicHeavyCount(0) {}
};

#endif // ENCOUNTER_TYPES_H
