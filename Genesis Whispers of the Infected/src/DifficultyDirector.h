#ifndef DIFFICULTY_DIRECTOR_H
#define DIFFICULTY_DIRECTOR_H

#include "EncounterTypes.h"
#include "player.h"

// ============================================================================
// Dynamic Difficulty Director
// Adjusts encounter tiers (1-3) based on player performance & health
// ============================================================================

class DifficultyDirector {
public:
    DifficultyDirector();

    void Evaluate(Player& player, const SpawnMemory& memory);
    int GetTier() const { return m_state.tier; }
    void OnPlayerDeath();
    void OnEncounterCleared(bool tookDamage);
    void Reset();

    const DifficultyState& GetState() const { return m_state; }

private:
    DifficultyState m_state;
};

#endif // DIFFICULTY_DIRECTOR_H
