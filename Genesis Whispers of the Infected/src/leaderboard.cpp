#define _CRT_SECURE_NO_WARNINGS
#include "leaderboard.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

// ============================================================================
// Score Comparison Predicate (Sort Descending)
// ============================================================================
bool CompareScores(const ScoreEntry& a, const ScoreEntry& b) {
    return a.score > b.score;
}

// ============================================================================
// Leaderboard Constructor
// ============================================================================
Leaderboard::Leaderboard() {
    entries.clear();
}

// ============================================================================
// File Loading & Default Initialization
// ============================================================================
void Leaderboard::LoadScores() {
    entries.clear();
    
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        // Populate default mock values if file doesn't exist yet
        AddScore("ARIN_RESCUER", 8500);
        AddScore("SURVIVOR_X", 6200);
        AddScore("NOVAGEN_DEFECTOR", 4100);
        SaveScores();
        return;
    }

    ScoreEntry entry;
    while (fread(&entry, sizeof(ScoreEntry), 1, file) == 1) {
        entries.push_back(entry);
    }
    fclose(file);

    // Keep entries sorted descending
    std::sort(entries.begin(), entries.end(), CompareScores);
}

// ============================================================================
// Binary File Persistence
// ============================================================================
void Leaderboard::SaveScores() {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) return;

    for (size_t i = 0; i < entries.size() && i < 5; ++i) {
        fwrite(&entries[i], sizeof(ScoreEntry), 1, file);
    }
    fclose(file);
}

// ============================================================================
// Score Entry Insertion
// ============================================================================
void Leaderboard::AddScore(const std::string& name, int score) {
    ScoreEntry newEntry;
    strncpy(newEntry.name, name.c_str(), sizeof(newEntry.name) - 1);
    newEntry.name[sizeof(newEntry.name) - 1] = '\0';
    newEntry.score = score;

    entries.push_back(newEntry);
    std::sort(entries.begin(), entries.end(), CompareScores);

    // Keep top 5 entries only
    if (entries.size() > 5) {
        entries.resize(5);
    }
}

