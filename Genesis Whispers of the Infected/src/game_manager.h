#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "player.h"
#include "map.h"
#include "enemy.h"
#include "leaderboard.h"
#include "UI.h"
#include "EncounterManager.h"
#include "Level3Boss.h"
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
    int width, height;
    CollectibleType type;
    bool active;
    int subType;

    Collectible() : x(0), y(0), width(32), height(32), type(COL_MEDKIT), active(false), subType(0) {}
    Collectible(double _x, double _y, int _w, int _h, CollectibleType _t, bool _a = true, int _st = 0)
        : x(_x), y(_y), width(_w), height(_h), type(_t), active(_a), subType(_st) {}
};

struct InventoryItem {
    bool isOccupied;
    std::string id;
    std::string name;
    std::string description;
    int count;
    std::string iconPath;
    std::string altIconPath;
    unsigned int textureID;

    InventoryItem() : isOccupied(false), count(0), textureID(0) {}
    InventoryItem(std::string itemId, std::string itemName, std::string itemDesc, int itemCnt, std::string path, std::string altPath = "")
        : isOccupied(true), id(itemId), name(itemName), description(itemDesc), count(itemCnt), iconPath(path), altIconPath(altPath), textureID(0) {}
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
    bool isObstacle;        // Solid obstacle flag
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

enum Level2Area {
    L2_AREA_FOREST_ENTRANCE,
    L2_AREA_ABANDONED_ROAD,
    L2_AREA_EVACUATION_CAMP,
    L2_AREA_DEEP_FOREST,
    L2_AREA_RIVER_CROSSING,
    L2_AREA_SURVIVOR_HIDEOUT,
    L2_AREA_INFECTED_FOREST,
    L2_AREA_NOVAGEN_OUTPOST,
    L2_AREA_RESEARCH_FACILITY,
    L2_AREA_BOSS_ARENA,
    L2_AREA_FACILITY_B_ROAD,
    L2_AREA_LEVEL_COMPLETE
};

// Authoritative Level 1 Ground Baseline Coordinate
const double kLevel1GroundY = 185.0;

// Centralized Environment Prop World Rendering Scale Factors (Referenced to Arin's 195px height)
const double kPropScaleSmall  = 1.1;  // Small: Posters, papers, notes, first aid, small items
const double kPropScaleMedium = 1.75; // Medium: Barrels, oil drums, crates, furniture, fences, sandbags, generator
const double kPropScaleLarge  = 2.3;  // Large: Ambulance, pickup trucks, cars, military checkpoint, store
const double kPropScaleTall   = 1.8;  // Tall: Trees, telephone poles, street lamps, watchtowers

// ============================================================================
// NPC System
// ============================================================================
enum NPCType {
    NPC_OLD_MAN,
    NPC_INJURED_WOMAN,
    NPC_SURVIVOR_CHILD
};

struct NPC {
    NPCType type;
    double x, y;
    int width, height;
    Animation animIdle;
    Animation animTalk;
    bool isTalking;
    bool isFacingRight;
    std::string dialogueText;
    std::string name;

    void Update() {
        if (isTalking) {
            animTalk.Update();
        } else {
            animIdle.Update();
        }
    }

    void Render(double camX, double camY) {
        double screenX = x - camX;
        double screenY = y - camY;
        if (isTalking) {
            animTalk.Render((int)screenX, (int)screenY, width, height, isFacingRight);
        } else {
            animIdle.Render((int)screenX, (int)screenY, width, height, isFacingRight);
        }
    }
};

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
    EncounterManager m_encounterManager;
    
    // Props and Collectibles
    std::vector<Collectible> collectibles;
    std::vector<Prop> props;
    std::vector<WorldProp> worldProps;
    std::vector<NPC> level2NPCs;
    std::vector<NPC> level3NPCs;
    Level3Boss m_l3Boss;
    int m_l3Route; // 0 = Undecided/Common, 1 = Easy Route, 2 = Hard Route
    bool m_l3NpcDialogueActive;
    bool m_l3Interrupted;
    double m_l3NpcTimer;
    double m_l3SpawnTimer;
    int m_l3NpcHitCount;
    unsigned int texPropsSheet;

    // Level Area Tracking
    int currentAreaIndex;
    int previousAreaIndex;
    double areaBannerTimer;
    double areaBannerAlpha;

    // Boss Battle variables
    bool bossSpawned;
    bool bossDefeated;
    int bossHp;
    int bossMaxHp;
    double displayedBossHp;

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
    double deathTimer;
    int pauseSubMenu; // 0 = Pause Main, 1 = Controls, 2 = Settings

    // Contextual Interaction Prompt
    std::string activePromptText;
    int activePromptX;
    int activePromptY;

    int score;
    int currentLevel;

    // Level 1 Staged Encounter Trigger Stage Flags
    int m_l1StreetStage;
    int m_l1SquareStage;
    int m_l1MarketStage;
    int m_l1CampStage;
    int m_l1ChurchStage;
    int m_l1QuarantineStage;
    int m_l1BossStage;

    // Inventory Management
    InventoryItem inventory[12];
    void InitInventory();
    void LoadInventoryTextures();
    void UseInventorySlot(int slotIndex);
    void AddInventoryItem(const std::string& itemId, int count = 1);

    // Helper functions for localized state updates/rendering
    void UpdatePlaying(float dt = 0.016f, bool keys[] = NULL, bool specialKeys[] = NULL);
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
    void LoadLevel1();
    // Level 2 & 3 Helpers
    void LoadLevel2();
    void LoadLevel2NPCs();
    void LoadLevel3();
    void LoadLevel3NPCs();
    void TriggerLevel3Route(int route);
    void Update(float dt = 0.016f, bool keys[] = NULL, bool specialKeys[] = NULL);
    void Render();

    // Independent Environment Prop System Methods
    void AddWorldProp(const std::string& assetPath, double x, double y, double width, double height, PropLayer layer = PROP_LAYER_BACKGROUND, bool isObstacle = false);
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
    
    // Enemy Accessors for Encounter System
    void AddEnemy(const Enemy& e) { enemies.push_back(e); }
    int GetActiveEnemyCount() const;

    GameState GetCurrentState() const { return currentState; }
    void SetCurrentState(GameState state) { currentState = state; }

    int GetCurrentLevel() const { return currentLevel; }
    int GetAreaFromPosition(double px) const;
    const char* GetAreaName(int areaIdx) const;
    const char* GetCurrentChapterName() const;
};

#endif // GAME_MANAGER_H

