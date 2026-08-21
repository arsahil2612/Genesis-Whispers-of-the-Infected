#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "player.h"
#include "map.h"
#include "enemy.h"
#include "leaderboard.h"
#include "UI.h"
#include <vector>
#include <string>

// ============================================================================
// Game State Taxonomy & Data Structs
// ============================================================================
enum GameState {
    STATE_STORY,
    STATE_MENU,
    STATE_PLAYING,
    STATE_DIALOGUE,
    STATE_PAUSED,
    STATE_GAMEOVER,
    STATE_VICTORY,
    STATE_LEADERBOARD
};

enum CollectibleType {
    COL_MEDKIT,
    COL_AMMO,
    COL_WATER,
    COL_BATTERY,
    COL_FOOD,
    COL_NOTE,
    COL_KEYCARD,
    COL_SCRAP,
    COL_RUSTY_KEY,
    COL_COIN
};

struct Collectible {
    double x, y;
    double width, height;
    CollectibleType type;
    bool active;
    int subType;
};

enum PropType {
    PROP_CAR,
    PROP_BARREL_FIRE,
    PROP_CRATE,
    PROP_SANDBAG,
    PROP_DRUM,
    PROP_RIBBON,
    PROP_POSTER_NOVAGEN,
    PROP_POSTER_QUARANTINE,
    PROP_POSTER_MISSING
};

struct Prop {
    double x, y;
    double width, height;
    PropType type;
    int animFrame;
};

enum PropLayer {
    PROP_LAYER_BACKGROUND,  // Rendered behind player and enemies
    PROP_LAYER_FOREGROUND   // Rendered in front of player and enemies
};

struct WorldProp {
    double x;               // World X position
    double y;               // World Y position
    double width;           // Scaled render width
    double height;          // Scaled render height
    std::string assetPath;   // Asset file path in Assets/Props/...
    unsigned int textureID; // Cached OpenGL texture handle
    PropLayer layer;        // Layer depth
    bool visible;           // Render flag
};

enum Level1Area {
    AREA_SPAWN_AREA,
    AREA_DESTROYED_HOUSE,
    AREA_VILLAGE_STREET,
    AREA_VILLAGE_SQUARE,
    AREA_ABANDONED_MARKET,
    AREA_RAIDER_CAMP,
    AREA_ABANDONED_CHURCH,
    AREA_QUARANTINE_ZONE,
    AREA_BROKEN_BRIDGE,
    AREA_MINI_BOSS_ARENA,
    AREA_EXIT_GATE,
    AREA_LEVEL_COMPLETE
};

// Authoritative Level 1 Ground Baseline Coordinate
const double kLevel1GroundY = 185.0;

// Centralized Environment Prop World Rendering Scale Factors (Referenced to Arin's 195px height)
const double kPropScaleSmall  = 1.1;  // Small: Posters, papers, notes, first aid, small items
const double kPropScaleMedium = 1.75; // Medium: Barrels, oil drums, crates, furniture, fences, sandbags, generator
const double kPropScaleLarge  = 2.3;  // Large: Ambulance, pickup trucks, cars, military checkpoint, store
const double kPropScaleTall   = 1.8;  // Tall: Trees, telephone poles, street lamps, watchtowers

// ============================================================================
// Core Game Manager Class
// ============================================================================
class GameManager {
private:
    GameState currentState;
    Player player;
    Map gameMap;
    Leaderboard leaderboard;
    std::vector<Enemy> enemies;
    
    // Props and Collectibles
    std::vector<Collectible> collectibles;
    std::vector<Prop> props;
    std::vector<WorldProp> worldProps;
    unsigned int texPropsSheet;

    // Level 1 Area Tracking
    Level1Area currentArea;
    Level1Area previousArea;
    double areaBannerTimer;
    double areaBannerAlpha;

    // Boss Battle variables
    bool bossSpawned;
    bool bossDefeated;
    int bossHp;
    int bossMaxHp;

    // Environmental Ribbon status
    bool ribbonCollected;
    bool hasKeycard;
    bool showInventory;
    double hudAlpha;

    // Mouse Cursor state & coordinates
    int mouseX;
    int mouseY;
    bool isMouseDown;

    // UI Animation & Smooth Transition States
    double menuTransitionAlpha;
    double missionNotifyAlpha;
    double missionNotifyTimer;
    int lastObjectiveID;
    double uiAnimTime;
    int pauseSubMenu; // 0 = Pause Main, 1 = Controls, 2 = Settings

    // Contextual Interaction Prompt
    std::string activePromptText;
    int activePromptX;
    int activePromptY;

    int score;
    int currentLevel;

    // Helper functions for localized state updates/rendering
    void UpdatePlaying(bool keys[], bool specialKeys[]);
    void RenderPlaying();
    void RenderMenu();
    void RenderLeaderboard();
    void RenderDialogue();
    void RenderGameOver();
    void RenderVictory();
    void RenderCursor();

public:
    GameManager();
    
    void Initialize();
    void Update(bool keys[], bool specialKeys[]);
    void Render();

    // Independent Environment Prop System Methods
    void AddWorldProp(const std::string& assetPath, double x, double y, double width, double height, PropLayer layer = PROP_LAYER_BACKGROUND);
    void RenderWorldProps(PropLayer layer, double camX, double camY);
    double GetPropWorldScale(const std::string& assetPath) const;
    double GetPropGroundOffset(const std::string& assetPath) const;

    // Input hooks from iGraphics
    void HandleKeyPress(unsigned char key);
    void HandleSpecialKeyPress(unsigned char key);
    void HandleMouseClick(int button, int state, int mx, int my);
    void HandleMouseMove(int mx, int my);

    // Score utility
    void AddScore(int amount);
    
    GameState GetCurrentState() const { return currentState; }
    void SetCurrentState(GameState state) { currentState = state; }

    Level1Area GetCurrentArea() const { return currentArea; }
    Level1Area GetAreaFromPosition(double px) const;
    const char* GetAreaName(Level1Area area) const;
};

#endif // GAME_MANAGER_H

