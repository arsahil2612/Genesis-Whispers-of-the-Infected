#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <string>
#include <vector>

// ============================================================================
// Leaderboard Entry Struct
// ============================================================================
struct ScoreEntry {
    char name[32];
    int score;
};

// ============================================================================
// Leaderboard System Class
// ============================================================================
class Leaderboard {
private:
    std::vector<ScoreEntry> entries;
    std::string filename;

public:
    Leaderboard();
    void LoadScores();
    void SaveScores();
    void AddScore(const std::string& name, int score);
    const std::vector<ScoreEntry>& GetEntries() const { return entries; }
    int GetScoreForName(const std::string& name, int defaultVal = 0) const;
};

#endif // LEADERBOARD_H

