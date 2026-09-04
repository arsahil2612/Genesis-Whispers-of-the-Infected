#include "DifficultyDirector.h"

DifficultyDirector::DifficultyDirector() {
    Reset();
}

void DifficultyDirector::Reset() {
    m_state.tier = 2; // Baseline tier
    m_state.playerHpPercent = 100.0;
    m_state.deathsThisLevel = 0;
    m_state.encountersSurvivedCleanly = 0;
}

void DifficultyDirector::Evaluate(Player& player, const SpawnMemory& memory) {
    if (player.maxHp > 0) {
        m_state.playerHpPercent = ((double)player.hp / (double)player.maxHp) * 100.0;
    } else {
        m_state.playerHpPercent = 100.0;
    }

    if (m_state.playerHpPercent < 40.0 || m_state.deathsThisLevel >= 2) {
        m_state.tier = 1;
    }
    else if (m_state.playerHpPercent >= 80.0 && m_state.encountersSurvivedCleanly >= 2) {
        m_state.tier = 3;
    }
    else {
        m_state.tier = 2;
    }
}

void DifficultyDirector::OnPlayerDeath() {
    m_state.deathsThisLevel++;
    m_state.encountersSurvivedCleanly = 0;
    m_state.tier = 1;
}

void DifficultyDirector::OnEncounterCleared(bool tookDamage) {
    if (!tookDamage) {
        m_state.encountersSurvivedCleanly++;
    } else {
        m_state.encountersSurvivedCleanly = 0;
    }
}
