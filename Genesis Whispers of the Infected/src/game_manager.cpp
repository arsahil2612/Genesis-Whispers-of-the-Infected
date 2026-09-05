#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include "game_manager.h"
#include "ResourceManager.h"
#include "igraphics_declarations.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

// Active Dialogue Text buffer
static char g_dialogueSpeaker[64] = "";
static char g_dialogueText[512] = "";

// ============================================================================
// Typography & Text Shadow / Outline Helpers
// ============================================================================
static void DrawShadowText(int x, int y, const char* str, void* font, int r, int g, int b, int shadowOffset = 1) {
    iSetColor(0, 0, 0);
    iText(x + shadowOffset, y - shadowOffset, (char*)str, font);

    iSetColor(r, g, b);
    iText(x, y, (char*)str, font);
}

static void DrawOutlinedText(int x, int y, const char* str, void* font, int r, int g, int b) {
    iSetColor(0, 0, 0);
    iText(x + 1, y, (char*)str, font);
    iText(x - 1, y, (char*)str, font);
    iText(x, y + 1, (char*)str, font);
    iText(x, y - 1, (char*)str, font);

    iSetColor(r, g, b);
    iText(x, y, (char*)str, font);
}

static void RenderMenuButtonSlot(int slotIdx, int textX, int textY, const char* label, void* font, int mouseX, int mouseY, bool isMouseDown, double animTime = 0.0) {
    int yMin = 420, yMax = 470;
    if (slotIdx == 2) { yMin = 345; yMax = 390; }
    else if (slotIdx == 3) { yMin = 265; yMax = 310; }

    bool isHovered = (mouseX >= 440 && mouseX <= 840 && mouseY >= yMin && mouseY <= yMax);

    if (isHovered) {
        // Glowing cyan hover indicator border with pulsing glow
        double pulse = 0.8 + 0.2 * sin(animTime * 8.0);
        int gVal = (int)(240 * pulse);
        int bVal = (int)(255 * pulse);

        iSetColor(0, gVal, bVal);
        iRectangle(436, yMin - 6, 408, yMax - yMin + 12);
        iRectangle(438, yMin - 4, 404, yMax - yMin + 8);

        if (isMouseDown) {
            // Button press offset animation (+3, -3) with gold click tint
            DrawShadowText(textX + 3, textY - 3, label, font, 255, 220, 0);
        } else {
            // High contrast cyan text on hover
            DrawShadowText(textX, textY, label, font, 0, 240, 255);
        }
    } else {
        DrawShadowText(textX, textY, label, font, 255, 255, 255);
    }
}

// Environmental Weather Simulation Particles
struct RainParticle {
    double x, y, speed;
    RainParticle() : x(0), y(0), speed(0) {}
    RainParticle(double _x, double _y, double _s) : x(_x), y(_y), speed(_s) {}
};

struct FogParticle {
    double x, y, speed, alpha;
    FogParticle() : x(0), y(0), speed(0), alpha(0) {}
    FogParticle(double _x, double _y, double _s, double _a) : x(_x), y(_y), speed(_s), alpha(_a) {}
};

static std::vector<RainParticle> rainParticles;
static std::vector<FogParticle> fogParticles;

// UI PNG Asset Texture Handles
static unsigned int g_texHealthFrame = 0;
static unsigned int g_texHealthFill = 0;
static unsigned int g_texStaminaFrame = 0;
static unsigned int g_texStaminaFill = 0;
static unsigned int g_texMissionBox = 0;

// Inventory UI PNG Texture Handles
static unsigned int g_texInventoryPanel = 0;
static unsigned int g_texInventorySlot = 0;
static unsigned int g_texBackpackIcon = 0;
static unsigned int g_texPauseOverlay = 0;
static unsigned int g_texMainMenuBg = 0;
static unsigned int g_texGameOverBg = 0;
static unsigned int g_texLevelCompleteBg = 0;

// Item PNG Asset Texture Handles (Assets/Items & Assets/Collectibles)
static unsigned int g_texItemBread = 0;
static unsigned int g_texItemApple = 0;
static unsigned int g_texItemWaterBottle = 0;
static unsigned int g_texItemFirstAid = 0;
static unsigned int g_texItemBandage = 0;
static unsigned int g_texItemScrapMetal = 0;
static unsigned int g_texItemRustyKey = 0;
static unsigned int g_texItemNovagenKeycard = 0;
static unsigned int g_texItemMissionNote = 0;
static unsigned int g_texItemCoin = 0;
static unsigned int g_texItemBattery = 0;

// Instant Floating Item Pickup Notification Data
static char g_pickupText[64] = "";
static double g_pickupX = 0.0;
static double g_pickupY = 0.0;
static double g_pickupTimer = 0.0;
static int g_pickupR = 255, g_pickupG = 255, g_pickupB = 255;

// ============================================================================
// Constructor & Level Initialization
// ============================================================================
const char* GameManager::GetAreaName(int areaIdx) const {
    if (currentLevel == 2) {
        switch (areaIdx) {
        case L2_AREA_FOREST_ENTRANCE:  return "Forest Entrance";
        case L2_AREA_ABANDONED_ROAD:   return "Abandoned Road";
        case L2_AREA_EVACUATION_CAMP:  return "Evacuation Camp";
        case L2_AREA_DEEP_FOREST:      return "Deep Forest";
        case L2_AREA_RIVER_CROSSING:   return "River Crossing";
        case L2_AREA_SURVIVOR_HIDEOUT: return "Survivor Hideout";
        case L2_AREA_INFECTED_FOREST:  return "Infected Forest";
        case L2_AREA_NOVAGEN_OUTPOST:  return "NovaGen Outpost";
        case L2_AREA_RESEARCH_FACILITY:return "Research Facility";
        case L2_AREA_BOSS_ARENA:       return "Boss Arena";
        case L2_AREA_FACILITY_B_ROAD:  return "Facility B Road";
        case L2_AREA_LEVEL_COMPLETE:   return "Level Complete";
        default:                        return "Blackwood Forest";
        }
    }
    switch (areaIdx) {
    case AREA_SPAWN_AREA:
    case AREA_DESTROYED_HOUSE:   return "Spawn Area / Destroyed House";
    case AREA_VILLAGE_STREET:    return "Village Street";
    case AREA_VILLAGE_SQUARE:    return "Village Square";
    case AREA_ABANDONED_MARKET:  return "Abandoned Market";
    case AREA_RAIDER_CAMP:       return "Raider Camp";
    case AREA_ABANDONED_CHURCH:  return "Abandoned Church";
    case AREA_QUARANTINE_ZONE:   return "Quarantine Zone";
    case AREA_BROKEN_BRIDGE:     return "Broken Bridge";
    case AREA_MINI_BOSS_ARENA:   return "Mini Boss Arena";
    case AREA_EXIT_GATE:         return "Exit Gate";
    case AREA_LEVEL_COMPLETE:    return "Level Complete";
    default:                     return "The Fallen Village";
    }
}

const char* GameManager::GetCurrentChapterName() const {
    return (currentLevel == 2) ? "BLACKWOOD FOREST" : "THE FALLEN VILLAGE";
}

GameManager::GameManager() {
    currentState = STATE_MENU;
    score = 0;
    currentLevel = 1;
    texPropsSheet = 0;

    currentAreaIndex = AREA_SPAWN_AREA;
    previousAreaIndex = AREA_SPAWN_AREA;
    areaBannerTimer = 4.0;
    areaBannerAlpha = 1.0;

    bossSpawned = false;
    bossDefeated = false;
    bossHp = 300;
    bossMaxHp = 300;
    displayedBossHp = 300.0;
    ribbonCollected = false;
    hasKeycard = false;
    showInventory = false;
    hudAlpha = 0.0;
    mouseX = 640;
    mouseY = 360;
    isMouseDown = false;

    menuTransitionAlpha = 1.0;
    missionNotifyAlpha = 0.0;
    missionNotifyTimer = 0.0;
    lastObjectiveID = 0;
    uiAnimTime = 0.0;
    deathTimer = 0.0;
    pauseSubMenu = 0;
    activePromptText = "";
    activePromptX = 0;
    activePromptY = 0;

    InitInventory();
}

void GameManager::InitInventory() {
    for (int i = 0; i < 12; ++i) {
        inventory[i] = InventoryItem();
    }

    // Row 1: Medicine & Supplies
    inventory[0] = InventoryItem("medkit", "FIRST AID MEDKIT", "Restores 40 HP [Click or H]", 2, "Assets/Items/Medicine/first_aid.png");
    inventory[1] = InventoryItem("bandage", "MEDICAL BANDAGE", "Restores 20 HP [Click to Use]", 3, "Assets/Items/Medicine/bandage.png");
    inventory[2] = InventoryItem("food_can", "FOOD CAN", "Restores 25 Stamina [Click or F]", 2, "Assets/Items/Food/food_can.png");
    inventory[3] = InventoryItem("water_bottle", "WATER BOTTLE", "Restores 25 Stamina [Click to Use]", 2, "Assets/Items/Food/water_bottle.png");

    // Row 2: Rations & Key Items
    inventory[4] = InventoryItem("bread", "FRESH BREAD", "Restores 35 Stamina [Click to Use]", 2, "Assets/Items/Food/Bread.png");
    inventory[5] = InventoryItem("apple", "FRESH APPLE", "Restores 15 Stamina & 10 HP", 4, "Assets/Items/Food/apple.png");
    inventory[6] = InventoryItem("keycard", "NOVAGEN KEYCARD", "Level 1 Exit Gate Clearance", 1, "Assets/Items/KeyItems/Novagen_keycard.png");
    inventory[7] = InventoryItem("rusty_key", "RUSTY KEY", "Unlocks village gates & lockers", 1, "Assets/Items/KeyItems/Rusty_Key.png");

    // Row 3: Survival Gear & Documents
    inventory[8] = InventoryItem("battery", "BATTERY", "Powers flashlight & electronics", 2, "Assets/Items/KeyItems/battery.png");
    inventory[9] = InventoryItem("scrap_metal", "SCRAP METAL", "Crafting & upgrade material", 5, "Assets/Items/KeyItems/Scrap_Metal.png");
    inventory[10] = InventoryItem("katana", "KATANA", "Sharp melee weapon (50 DMG) [Attack: J]", 1, "Assets/Items/KeyItems/katana.png");
    inventory[11] = InventoryItem("mission_note", "CLASSIFIED NOTE", "Intel on Project Genesis & Luna [Click to Read]", 1, "Assets/Items/Documents/Mission_note.png");

    LoadInventoryTextures();
}

void GameManager::LoadInventoryTextures() {
    for (int i = 0; i < 12; ++i) {
        if (inventory[i].isOccupied) {
            if (inventory[i].textureID == 0 && !inventory[i].iconPath.empty()) {
                std::string fullPath = GetAssetPath(inventory[i].iconPath.c_str());
                inventory[i].textureID = iLoadImage((char*)fullPath.c_str());
            }
            if (inventory[i].textureID == 0 && !inventory[i].altIconPath.empty()) {
                std::string altPath = GetAssetPath(inventory[i].altIconPath.c_str());
                inventory[i].textureID = iLoadImage((char*)altPath.c_str());
            }
        }
    }
}

void GameManager::UseInventorySlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= 12 || !inventory[slotIndex].isOccupied) return;

    InventoryItem& item = inventory[slotIndex];
    std::string id = item.id;

    if (id == "medkit") {
        if (player.hp >= player.maxHp) {
            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "HEALTH FULL");
            g_pickupR = 255; g_pickupG = 200; g_pickupB = 50;
        } else {
            player.hp = player.maxHp;
            player.displayedHp = (double)player.hp;
            if (player.medkits > 0) player.medkits--;
            item.count--;

            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "MEDKIT USED (FULL HP)");
            g_pickupR = 255; g_pickupG = 100; g_pickupB = 100;
        }
    }
    else if (id == "bandage") {
        if (player.hp >= player.maxHp) {
            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "HEALTH FULL");
            g_pickupR = 255; g_pickupG = 200; g_pickupB = 50;
        } else {
            player.hp = (player.hp + 40 > player.maxHp) ? player.maxHp : player.hp + 40;
            player.displayedHp = (double)player.hp;
            item.count--;

            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "BANDAGE USED (+40 HP)");
            g_pickupR = 255; g_pickupG = 100; g_pickupB = 100;
        }
    }
    else if (id == "food_can") {
        if (player.staminaDouble >= (double)player.maxStamina && player.hp >= player.maxHp) {
            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "HP & STAMINA FULL");
            g_pickupR = 255; g_pickupG = 200; g_pickupB = 50;
        } else {
            player.staminaDouble = (player.staminaDouble + 25.0 > (double)player.maxStamina) ? (double)player.maxStamina : player.staminaDouble + 25.0;
            player.stamina = (int)(player.staminaDouble + 0.5);
            player.displayedStamina = player.staminaDouble;
            if (player.staminaDouble >= 15.0) {
                player.isExhausted = false;
            }
            player.hp = (player.hp + 20 > player.maxHp) ? player.maxHp : player.hp + 20;
            player.displayedHp = (double)player.hp;
            if (player.foodCount > 0) player.foodCount--;
            item.count--;

            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "FOOD CAN USED (+25 STAMINA, +20 HP)");
            g_pickupR = 255; g_pickupG = 180; g_pickupB = 0;
        }
    }
    else if (id == "water_bottle") {
        if (player.staminaDouble >= (double)player.maxStamina) {
            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "STAMINA FULL");
            g_pickupR = 255; g_pickupG = 200; g_pickupB = 50;
        } else {
            player.staminaDouble = (player.staminaDouble + 25.0 > (double)player.maxStamina) ? (double)player.maxStamina : player.staminaDouble + 25.0;
            player.stamina = (int)(player.staminaDouble + 0.5);
            player.displayedStamina = player.staminaDouble;
            if (player.staminaDouble >= 15.0) {
                player.isExhausted = false;
            }
            if (player.waterBottleCount > 0) player.waterBottleCount--;
            item.count--;

            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "WATER BOTTLE USED (+25 STAMINA)");
            g_pickupR = 0; g_pickupG = 220; g_pickupB = 255;
        }
    }
    else if (id == "bread") {
        if (player.staminaDouble >= (double)player.maxStamina && player.hp >= player.maxHp) {
            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "HP & STAMINA FULL");
            g_pickupR = 255; g_pickupG = 200; g_pickupB = 50;
        } else {
            player.staminaDouble = (player.staminaDouble + 35.0 > (double)player.maxStamina) ? (double)player.maxStamina : player.staminaDouble + 35.0;
            player.stamina = (int)(player.staminaDouble + 0.5);
            player.displayedStamina = player.staminaDouble;
            if (player.staminaDouble >= 15.0) {
                player.isExhausted = false;
            }
            player.hp = (player.hp + 25 > player.maxHp) ? player.maxHp : player.hp + 25;
            player.displayedHp = (double)player.hp;
            if (player.foodCount > 0) player.foodCount--;
            item.count--;

            g_pickupTimer = 2.0;
            g_pickupX = player.x;
            g_pickupY = player.y + 120.0;
            sprintf_s(g_pickupText, sizeof(g_pickupText), "BREAD CONSUMED (+35 STAMINA, +25 HP)");
            g_pickupR = 255; g_pickupG = 180; g_pickupB = 0;
        }
    }
    else if (id == "apple") {
        if (player.staminaDouble < player.maxStamina || player.hp < player.maxHp) {
            player.staminaDouble = (double)player.maxStamina;
            player.stamina = player.maxStamina;
            player.displayedStamina = (double)player.maxStamina;
            player.isExhausted = false;
            player.hp = (player.hp + 25 > player.maxHp) ? player.maxHp : player.hp + 25;
            player.displayedHp = (double)player.hp;
            item.count--;
        }
    }
    else if (id == "mission_note") {
        currentState = STATE_DIALOGUE;
        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "NovaGen Research Note");
        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Project Genesis Intel:\nEvacuation path compromised. Convoy heading East.\nDr. Kael took Subject Luna through the checkpoint.\"");
    }
    else if (id == "keycard") {
        currentState = STATE_DIALOGUE;
        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "NovaGen Keycard");
        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"NovaGen Command Security Keycard.\nRequired to open the main exit gate at the end of Level 1.\"");
    }
    else if (id == "rusty_key") {
        currentState = STATE_DIALOGUE;
        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Rusty Gate Key");
        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"An old iron key recovered from the fallen village.\nUnlocks supply lockers and wooden gates.\"");
    }
    else if (id == "battery") {
        currentState = STATE_DIALOGUE;
        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "High-Capacity Battery");
        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Powers tactical flashlights and electronic lab scanners.\"");
    }
    else if (id == "scrap_metal") {
        currentState = STATE_DIALOGUE;
        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Scrap Metal");
        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Raw metal scrap collected from ruined structures. Used for crafting upgrades.\"");
    }
    else if (id == "katana") {
        currentState = STATE_DIALOGUE;
        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin's Katana");
        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Tempered steel blade (50 Melee DMG).\nPress J during gameplay to perform melee katana slashes!\"");
    }

    if (item.count <= 0) {
        item = InventoryItem();
    }
}

void GameManager::AddInventoryItem(const std::string& itemId, int count) {
    // 1. Try to stack in existing slot
    for (int i = 0; i < 12; ++i) {
        if (inventory[i].isOccupied && inventory[i].id == itemId) {
            inventory[i].count += count;
            return;
        }
    }

    // 2. Add to first empty slot
    for (int i = 0; i < 12; ++i) {
        if (!inventory[i].isOccupied) {
            if (itemId == "medkit") {
                inventory[i] = InventoryItem("medkit", "FIRST AID MEDKIT", "Restores 40 HP [Click or H]", count, "Assets/Items/Medicine/first_aid.png");
            } else if (itemId == "bandage") {
                inventory[i] = InventoryItem("bandage", "MEDICAL BANDAGE", "Restores 20 HP [Click to Use]", count, "Assets/Items/Medicine/bandage.png");
            } else if (itemId == "food_can") {
                inventory[i] = InventoryItem("food_can", "FOOD CAN", "Restores 25 Stamina [Click or F]", count, "Assets/Items/Food/food_can.png");
            } else if (itemId == "water_bottle") {
                inventory[i] = InventoryItem("water_bottle", "WATER BOTTLE", "Restores 25 Stamina [Click to Use]", count, "Assets/Items/Food/water_bottle.png");
            } else if (itemId == "bread") {
                inventory[i] = InventoryItem("bread", "FRESH BREAD", "Restores 35 Stamina [Click to Use]", count, "Assets/Items/Food/Bread.png");
            } else if (itemId == "apple") {
                inventory[i] = InventoryItem("apple", "FRESH APPLE", "Restores 15 Stamina & 10 HP", count, "Assets/Items/Food/apple.png");
            } else if (itemId == "keycard") {
                inventory[i] = InventoryItem("keycard", "NOVAGEN KEYCARD", "Level 1 Exit Gate Clearance", count, "Assets/Items/KeyItems/Novagen_keycard.png");
            } else if (itemId == "rusty_key") {
                inventory[i] = InventoryItem("rusty_key", "RUSTY KEY", "Unlocks village gates & lockers", count, "Assets/Items/KeyItems/Rusty_Key.png");
            } else if (itemId == "battery") {
                inventory[i] = InventoryItem("battery", "BATTERY", "Powers flashlight & electronics", count, "Assets/Items/KeyItems/battery.png");
            } else if (itemId == "scrap_metal") {
                inventory[i] = InventoryItem("scrap_metal", "SCRAP METAL", "Crafting & upgrade material", count, "Assets/Items/KeyItems/Scrap_Metal.png");
            } else if (itemId == "mission_note") {
                inventory[i] = InventoryItem("mission_note", "CLASSIFIED NOTE", "Intel on Project Genesis & Luna", count, "Assets/Items/Documents/Mission_note.png");
            } else {
                inventory[i] = InventoryItem(itemId, "SURVIVAL ITEM", "Useful survival resource", count, "Assets/Items/KeyItems/Scrap_Metal.png");
            }

            if (!inventory[i].iconPath.empty()) {
                std::string fullPath = GetAssetPath(inventory[i].iconPath.c_str());
                inventory[i].textureID = iLoadImage((char*)fullPath.c_str());
            }
            return;
        }
    }
}

int GameManager::GetActiveEnemyCount() const {
    int activeCount = 0;
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (enemies[i].hp > 0) {
            activeCount++;
        }
    }
    return activeCount;
}

void GameManager::Initialize() {
    UI::Initialize();
    score = 0;
    leaderboard.LoadScores();
    InitInventory();
    Enemy::PreloadAllTextures();
    LoadLevel1();
}

void GameManager::LoadLevel1() {
    currentLevel = 1;
    player.Initialize(200, 185); // Arin starting location inside destroyed house (x=200, groundY=185)
    gameMap.LoadLevel(currentLevel);
    m_encounterManager.Initialize(1);

    currentAreaIndex = AREA_SPAWN_AREA;
    previousAreaIndex = AREA_SPAWN_AREA;
    areaBannerTimer = 4.0;
    areaBannerAlpha = 1.0;

    bossSpawned = false;
    bossDefeated = false;
    bossHp = 300;
    bossMaxHp = 300;
    displayedBossHp = 300.0;
    ribbonCollected = false;
    hasKeycard = false;
    showInventory = false;
    hudAlpha = 0.0;
    mouseX = 640;
    mouseY = 360;
    isMouseDown = false;

    menuTransitionAlpha = 1.0;
    missionNotifyAlpha = 0.0;
    missionNotifyTimer = 0.0;
    lastObjectiveID = 0;
    uiAnimTime = 0.0;
    deathTimer = 0.0;
    pauseSubMenu = 0;
    activePromptText = "";
    activePromptX = 0;
    activePromptY = 0;

    // Initialize Independent Environment Prop System & World Props
    worldProps.clear();

    // --- AREA 1 & 2: DESTROYED HOUSE (ARIN'S FAMILY HOME: x = 0 to 3500) ---
    AddWorldProp("Assets/Props/Furniture/furn_broken_chair_01.png", 420.0, 185.0, 56.0, 56.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #1: Chair
    AddWorldProp("Assets/Props/Furniture/furn_dining_table_01.png", 950.0, 185.0, 110.0, 70.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #2: Low Dining Table

    // 2. Kitchen / Storage Area (x = 1200 to 2000)
    AddWorldProp("Assets/Props/Furniture/furn_wooden_cabinet_01.png", 1500.0, 185.0, 85.0, 115.0, PROP_LAYER_BACKGROUND, false); // Decorative Wardrobe/Cabinet
    AddWorldProp("Assets/Props/Decorations/veh_shopping_cart_destroyed.png", 1950.0, 185.0, 75.0, 60.0, PROP_LAYER_BACKGROUND, true); // Solid Shopping Cart / Tool Box Obstacle

    // 3. Arin & Luna's Bedrooms (x = 2000 to 3000)
    AddWorldProp("Assets/Props/Furniture/furn_broken_bed_01.png", 2400.0, 185.0, 130.0, 75.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle: Standable Bed
    AddWorldProp("Assets/Props/Furniture/furn_broken_bench_01.png", 2850.0, 185.0, 120.0, 65.0, PROP_LAYER_BACKGROUND, true); // Larger Jumpable Bench Obstacle

    // 4. Grounded Storage & Boundary (x = 3000 to 3500)
    AddWorldProp("Assets/Props/Decorations/prop_wooden_crate_01.png", 3200.0, 185.0, 48.0, 48.0, PROP_LAYER_BACKGROUND, true); // Solid Crate Obstacle
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 3520.0, 185.0, 100.0, 65.0, PROP_LAYER_BACKGROUND, true); // Solid Barricade Fence

    // --- AREA 3 & 4: VILLAGE STREET & SQUARE (x = 3500 to 7500) ---
    AddWorldProp("Assets/Props/Decorations/prop_telephone_pole_01.png", 3650.0, 185.0, 60.0, 240.0, PROP_LAYER_BACKGROUND); // Decorative Pole
    AddWorldProp("Assets/Posters/poster_emergency_evacuation.png", 3655.0, 250.0, 38.0, 50.0, PROP_LAYER_BACKGROUND); // Decorative Paper Poster
    AddWorldProp("Assets/Props/Nature/nature_dead_tree_01.png", 3900.0, 185.0, 160.0, 240.0, PROP_LAYER_BACKGROUND); // Larger Taller Tree
    AddWorldProp("Assets/Props/Vehicles/veh_pickup_destroyed.png", 4250.0, 185.0, 160.0, 90.0, PROP_LAYER_BACKGROUND); // Decorative Backdrop Vehicle
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 4270.0, 215.0, 36.0, 48.0, PROP_LAYER_BACKGROUND); // Decorative Paper Warning

    // 2. Mid Street & Barricade Zone (x = 4400 to 5500)
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 4580.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND); // Decorative Lamp
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 4585.0, 240.0, 36.0, 48.0, PROP_LAYER_BACKGROUND); // Decorative Paper Warning
    AddWorldProp("Assets/Props/Decorations/prop_burning_barrel_01.png", 4880.0, 185.0, 65.0, 84.0, PROP_LAYER_BACKGROUND, true); // Larger Jumpable Fire Drum Obstacle
    AddWorldProp("Assets/Props/Vehicles/veh_destroyed_car_01.png", 5280.0, 185.0, 150.0, 80.0, PROP_LAYER_BACKGROUND); // Decorative Vehicle
    AddWorldProp("Assets/Props/Nature/dry_bush.png", 5520.0, 185.0, 48.0, 36.0, PROP_LAYER_FOREGROUND, false); // Decorative Bush / Foliage

    // 3. Village Square Approach (x = 5500 to 6700)
    AddWorldProp("Assets/Props/Decorations/prop_telephone_pole_01.png", 5800.0, 185.0, 60.0, 240.0, PROP_LAYER_BACKGROUND); // Decorative Pole
    AddWorldProp("Assets/Props/Vehicles/veh_ambulance_burned.png", 6150.0, 185.0, 170.0, 95.0, PROP_LAYER_BACKGROUND); // Decorative Vehicle
    AddWorldProp("Assets/Posters/poster_emergency_evacuation.png", 6170.0, 220.0, 38.0, 50.0, PROP_LAYER_BACKGROUND); // Decorative Poster
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 6600.0, 185.0, 90.0, 60.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #2: Barricade Fence

    // 4. Village Square Edge (x = 6700 to 7400)
    AddWorldProp("Assets/Props/Nature/nature_dead_tree_01.png", 7000.0, 185.0, 160.0, 240.0, PROP_LAYER_BACKGROUND); // Larger Taller Tree
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 7350.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND); // Decorative Lamp
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 7355.0, 240.0, 36.0, 48.0, PROP_LAYER_BACKGROUND); // Decorative Warning
    AddWorldProp("Assets/Props/Decorations/prop_oil_drum_01.png", 7550.0, 185.0, 44.0, 55.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #3: Oil Drum

    // --- AREA 4: VILLAGE SQUARE & QUARANTINE (x = 7500 to 9000) ---
    AddWorldProp("Assets/Props/Decorations/prop_sandbags_01.png", 7750.0, 185.0, 110.0, 50.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #1: Sandbags
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 8050.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND); // Decorative Lamp

    // 2. Central Plaza & Rubble (x = 7900 to 8400)
    AddWorldProp("Assets/Props/Vehicles/veh_ambulance_burned.png", 8280.0, 185.0, 170.0, 95.0, PROP_LAYER_BACKGROUND); // Decorative Vehicle
    AddWorldProp("Assets/Props/Decorations/prop_oil_drum_01.png", 8500.0, 185.0, 44.0, 55.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #2: Plaza Oil Drum

    // 3. Military Checkpoint Barricade & East Exit (x = 8400 to 9000)
    AddWorldProp("Assets/Props/Military/bld_military_checkpoint.png", 8800.0, 185.0, 130.0, 85.0, PROP_LAYER_BACKGROUND); // Decorative Checkpoint
    AddWorldProp("Assets/Posters/poster_novagen_genesis.png", 8825.0, 220.0, 38.0, 50.0, PROP_LAYER_BACKGROUND); // Decorative Poster
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 9080.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND); // Decorative Lamp

    // --- AREA 5: ABANDONED MARKET (x = 9000 to 11000) ---
    AddWorldProp("Assets/Props/Buildings/bld_grocery_store_abandoned.png", 9250.0, 185.0, 160.0, 130.0, PROP_LAYER_BACKGROUND); // Decorative Storefront
    AddWorldProp("Assets/Posters/poster_novagen_genesis.png", 9275.0, 225.0, 38.0, 50.0, PROP_LAYER_BACKGROUND); // Decorative Poster
    AddWorldProp("Assets/Props/Decorations/veh_shopping_cart_destroyed.png", 9600.0, 185.0, 75.0, 60.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #1: Tool Box / Cart

    // 2. Inner Market Aisles (x = 9500 to 10500)
    AddWorldProp("Assets/Props/Furniture/furn_grocery_shelf_01.png", 10050.0, 185.0, 90.0, 120.0, PROP_LAYER_BACKGROUND, false); // Image-Only Decorative Shelf
    AddWorldProp("Assets/Props/Decorations/prop_wooden_crate_01.png", 10450.0, 185.0, 48.0, 48.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #2: Market Crate 1

    // 3. Market Storage & Rear Exit (x = 10500 to 11000)
    AddWorldProp("Assets/Props/Furniture/furn_grocery_shelf_01.png", 10800.0, 185.0, 90.0, 120.0, PROP_LAYER_BACKGROUND, false); // Image-Only Decorative Shelf
    AddWorldProp("Assets/Props/Decorations/prop_wooden_crate_01.png", 11100.0, 185.0, 48.0, 48.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #3: Market Crate 2

    // --- AREA 6: RAIDER CAMP & EXIT GATE (x = 11000 to 13500) ---
    AddWorldProp("Assets/Props/Decorations/prop_sandbags_01.png", 11400.0, 185.0, 110.0, 50.0, PROP_LAYER_BACKGROUND, false); // Decorative Sandbags
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 11430.0, 210.0, 36.0, 48.0, PROP_LAYER_BACKGROUND); // Decorative Poster Warning
    AddWorldProp("Assets/Props/Decorations/prop_generator_01.png", 11600.0, 185.0, 70.0, 60.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #1: Generator Barricade

    // 2. Watchtower & Campfire Hub (x = 11400 to 12200)
    AddWorldProp("Assets/Props/Military/bld_raider_watchtower.png", 11900.0, 185.0, 180.0, 280.0, PROP_LAYER_BACKGROUND); // Decorative Watchtower

    // 3. Exit Gate & Luna's Ribbon Checkpoint (x = 12200 to 13500)
    AddWorldProp("Assets/Props/Buildings/Quarantine_CheckpointQuarantine_Checkpoint.png", 12400.0, 185.0, 160.0, 120.0, PROP_LAYER_BACKGROUND); // Decorative Checkpoint
    AddWorldProp("Assets/Props/Decorations/drum.png", 12750.0, 185.0, 50.0, 60.0, PROP_LAYER_BACKGROUND, true); // Jumpable Obstacle #2: Outpost Oil Drum
    AddWorldProp("Assets/Props/Decorations/prop_sandbags_01.png", 13000.0, 185.0, 90.0, 45.0, PROP_LAYER_FOREGROUND, false); // Decorative Sandbags
    AddWorldProp("Assets/Props/Military/Exit_Gate.png", 13350.0, 185.0, 586.0, 440.0, PROP_LAYER_BACKGROUND); // Decorative Gate Structure

    // Load props texture sheet
    if (texPropsSheet == 0) {
        texPropsSheet = iLoadImage((char*)GetAssetPath("Assets/Props/props_sheet.png").c_str());
    }

    // Preload UI HUD assets
    if (g_texHealthFrame == 0) {
        g_texHealthFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Health_Bar_Frame.png").c_str());
        if (g_texHealthFrame == 0) g_texHealthFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_health_frame.png").c_str());
    }
    if (g_texHealthFill == 0) {
        g_texHealthFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Health_Fill.png").c_str());
        if (g_texHealthFill == 0) g_texHealthFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_health_fill.png").c_str());
    }
    if (g_texStaminaFrame == 0) {
        g_texStaminaFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Stamina_Bar_Frame.png").c_str());
        if (g_texStaminaFrame == 0) g_texStaminaFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_stamina_frame.png").c_str());
    }
    if (g_texStaminaFill == 0) {
        g_texStaminaFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/stamina_fill.png").c_str());
        if (g_texStaminaFill == 0) g_texStaminaFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_stamina_fill.png").c_str());
    }
    if (g_texMissionBox == 0) {
        g_texMissionBox = iLoadImage((char*)GetAssetPath("Assets/UI/Mission/mission_update_box.png").c_str());
        if (g_texMissionBox == 0) g_texMissionBox = iLoadImage((char*)GetAssetPath("Assets/UI/Mission/ui_mission_box.png").c_str());
    }
    if (g_texInventoryPanel == 0) {
        g_texInventoryPanel = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/inventory__panel.png").c_str());
        if (g_texInventoryPanel == 0) g_texInventoryPanel = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/ui_inventory_panel.png").c_str());
    }
    if (g_texInventorySlot == 0) {
        g_texInventorySlot = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/single_empty_inventory_slot.png").c_str());
        if (g_texInventorySlot == 0) g_texInventorySlot = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/ui_inventory_slot.png").c_str());
    }
    if (g_texBackpackIcon == 0) {
        g_texBackpackIcon = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/backpack_icon.png").c_str());
    }
    if (g_texPauseOverlay == 0) {
        g_texPauseOverlay = iLoadImage((char*)GetAssetPath("Assets/UI/Pause/pause_menu.png").c_str());
        if (g_texPauseOverlay == 0) g_texPauseOverlay = iLoadImage((char*)GetAssetPath("Assets/UI/Pause/ui_pause_overlay.png").c_str());
        if (g_texPauseOverlay == 0) g_texPauseOverlay = iLoadImage((char*)GetAssetPath("Assets/UI/Pause/ui_pause_menu.png").c_str());
    }
    if (g_texMainMenuBg == 0) {
        g_texMainMenuBg = iLoadImage((char*)GetAssetPath("Assets/UI/Main Menu/new_main_menu.png").c_str());
        if (g_texMainMenuBg == 0) g_texMainMenuBg = iLoadImage((char*)GetAssetPath("Assets/UI/Main Menu/main_menu_bg.png").c_str());
        if (g_texMainMenuBg == 0) g_texMainMenuBg = iLoadImage((char*)GetAssetPath("Assets/UI/Main Menu/ui_main_menu.png").c_str());
    }
    if (g_texGameOverBg == 0) {
        g_texGameOverBg = iLoadImage((char*)GetAssetPath("Assets/UI/Game Over/game_over_screen.png").c_str());
        if (g_texGameOverBg == 0) g_texGameOverBg = iLoadImage((char*)GetAssetPath("Assets/UI/Game Over/ui_game_over_background.png").c_str());
        if (g_texGameOverBg == 0) g_texGameOverBg = iLoadImage((char*)GetAssetPath("Assets/UI/Game Over/ui_game_over_screen.png").c_str());
    }
    if (g_texLevelCompleteBg == 0) {
        g_texLevelCompleteBg = iLoadImage((char*)GetAssetPath("Assets/UI/Level Complete/level_complete_screen.png").c_str());
        if (g_texLevelCompleteBg == 0) g_texLevelCompleteBg = iLoadImage((char*)GetAssetPath("Assets/UI/Level Complete/ui_level_complete_background.png").c_str());
        if (g_texLevelCompleteBg == 0) g_texLevelCompleteBg = iLoadImage((char*)GetAssetPath("Assets/UI/Level Complete/ui_level_complete_screen.png").c_str());
    }

    // Load static item textures once
    if (g_texItemFirstAid == 0) {
        g_texItemFirstAid = iLoadImage((char*)GetAssetPath("Assets/Items/Medicine/first_aid.png").c_str());
        g_texItemBandage = iLoadImage((char*)GetAssetPath("Assets/Items/Medicine/bandage.png").c_str());
        g_texItemApple = iLoadImage((char*)GetAssetPath("Assets/Items/Food/apple.png").c_str());
        g_texItemBread = iLoadImage((char*)GetAssetPath("Assets/Items/Food/Bread.png").c_str());
        g_texItemWaterBottle = iLoadImage((char*)GetAssetPath("Assets/Items/Food/water_bottle.png").c_str());
        g_texItemScrapMetal = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/Scrap_Metal.png").c_str());
        g_texItemRustyKey = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/Rusty_Key.png").c_str());
        if (g_texItemRustyKey == 0) {
            g_texItemRustyKey = ResourceManager::GetInstance().GetTexture("Assets/Items/KeyItems/Rusty_Key.png");
        }
        g_texItemNovagenKeycard = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/Novagen_keycard.png").c_str());
        if (g_texItemNovagenKeycard == 0) {
            g_texItemNovagenKeycard = ResourceManager::GetInstance().GetTexture("Assets/Items/KeyItems/Novagen_keycard.png");
        }
        g_texItemMissionNote = iLoadImage((char*)GetAssetPath("Assets/Items/Documents/Mission_note.png").c_str());
        if (g_texItemMissionNote == 0) {
            g_texItemMissionNote = ResourceManager::GetInstance().GetTexture("Assets/Items/Documents/Mission_note.png");
        }
        g_texItemCoin = iLoadImage((char*)GetAssetPath("Assets/Collectibles/coin.png").c_str());
        g_texItemBattery = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/battery.png").c_str());
        if (g_texItemBattery == 0) {
            g_texItemBattery = ResourceManager::GetInstance().GetTexture("Assets/Items/KeyItems/battery.png");
        }
    }

    // Populate Level 1 Enemies per area specification
    enemies.clear();

    // Section 2: Village Street (3 Walkers)
    enemies.push_back(Enemy(1800, 1950, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(2200, 2350, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(2600, 2750, kLevel1GroundY, TYPE_SPITTER));

    // Section 3: Village Square (4 Walkers, 1 Runner)
    enemies.push_back(Enemy(3100, 3220, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3350, 3470, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3600, 3720, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3850, 3970, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(4150, 4280, kLevel1GroundY, TYPE_RUNNER));

    // Section 4: Abandoned Market (2 Walkers, 1 Raider)
    enemies.push_back(Enemy(4600, 4750, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(5000, 5150, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(5400, 5550, kLevel1GroundY, TYPE_RAIDER));

    // Section 5: Raider Camp (3 Raiders)
    enemies.push_back(Enemy(6050, 6200, kLevel1GroundY, TYPE_RAIDER));
    enemies.push_back(Enemy(6450, 6600, kLevel1GroundY, TYPE_RAIDER));
    enemies.push_back(Enemy(6850, 7000, kLevel1GroundY, TYPE_RAIDER));

    // Section 6: Abandoned Church (2 Walkers, 1 Runner)
    enemies.push_back(Enemy(7500, 7650, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(7900, 8050, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(8350, 8500, kLevel1GroundY, TYPE_RUNNER));

    // Section 7: Quarantine Zone (2 Walkers, 1 Heavy Infected #1)
    enemies.push_back(Enemy(8950, 9100, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(9350, 9500, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(9750, 9950, kLevel1GroundY, TYPE_HEAVY));

    // Section 8/9: Pre-boss Gate (1 Heavy Infected #2)
    enemies.push_back(Enemy(11650, 11850, kLevel1GroundY, TYPE_HEAVY));

    // Section 9: Mini Boss Arena (1 Mutated Brute)
    enemies.push_back(Enemy(12200, 12450, kLevel1GroundY, TYPE_ABOMINATION));

    props.clear();

    // Populate Collectibles aligned with ground baseline
    collectibles.clear();
    collectibles.push_back(Collectible(350, kLevel1GroundY, 32, 32, COL_SCRAP, true, 0));      // Scrap Metal
    collectibles.push_back(Collectible(2400, kLevel1GroundY, 32, 32, COL_WATER, true, 0));     // Water Bottle
    collectibles.push_back(Collectible(3500, kLevel1GroundY, 32, 32, COL_FOOD, true, 0));      // Food (Bread)
    collectibles.push_back(Collectible(4200, kLevel1GroundY, 32, 32, COL_AMMO, true, 0));      // Ammo
    collectibles.push_back(Collectible(4900, kLevel1GroundY, 32, 32, COL_BATTERY, true, 0));   // Battery
    collectibles.push_back(Collectible(6925, kLevel1GroundY, 32, 32, COL_NOTE, true, 0));       // Mission Note
    collectibles.push_back(Collectible(7300, kLevel1GroundY, 32, 32, COL_MEDKIT, true, 0));     // Medkit (First Aid)
    collectibles.push_back(Collectible(9250, kLevel1GroundY, 32, 32, COL_KEYCARD, true, 0));    // NovaGen Keycard

    // Initialize rain particle simulation
    rainParticles.clear();
    for (int i = 0; i < 80; ++i) {
        rainParticles.push_back(RainParticle((double)(rand() % 1280), (double)(rand() % 720), 6.0 + (rand() % 40) / 10.0));
    }

    fogParticles.clear();
    for (int i = 0; i < 24; ++i) {
        fogParticles.push_back(FogParticle((double)(rand() % 1280), (double)(80 + rand() % 420), 0.4 + (rand() % 8) / 10.0, 0.15 + (rand() % 20) / 100.0));
    }
}

void GameManager::LoadLevel2() {
    ResourceManager::GetInstance().ClearCache();
    currentLevel = 2;
    player.Initialize(200, 185); // Arin starting location in Blackwood Forest Entrance
    gameMap.LoadLevel(2);

    currentAreaIndex = L2_AREA_FOREST_ENTRANCE;
    previousAreaIndex = L2_AREA_FOREST_ENTRANCE;
    areaBannerTimer = 4.0;
    areaBannerAlpha = 1.0;

    bossSpawned = false;
    bossDefeated = false;
    bossHp = 400;
    bossMaxHp = 400;
    displayedBossHp = 400.0;
    ribbonCollected = false;
    hasKeycard = false;
    showInventory = false;
    hudAlpha = 0.0;
    mouseX = 640;
    mouseY = 360;
    isMouseDown = false;

    menuTransitionAlpha = 1.0;
    missionNotifyAlpha = 0.0;
    missionNotifyTimer = 0.0;
    lastObjectiveID = 0;
    uiAnimTime = 0.0;
    deathTimer = 0.0;
    pauseSubMenu = 0;
    activePromptText = "";
    activePromptX = 0;
    activePromptY = 0;

    worldProps.clear();

    // --- AREA 1: FOREST ENTRANCE PROPS (World X: 0 to 1448) ---
    AddWorldProp("Assets/Props/Nature/nature_dead_tree_01.png", 280.0, 185.0, 160.0, 240.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 520.0, 185.0, 90.0, 60.0, PROP_LAYER_BACKGROUND, true); // Jumpable Fence
    AddWorldProp("Assets/Props/Vehicles/veh_destroyed_car_01.png", 850.0, 185.0, 150.0, 80.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_burning_barrel_01.png", 1150.0, 185.0, 65.0, 84.0, PROP_LAYER_BACKGROUND, true); // Fire Drum

    // --- AREA 2: EVACUATION CAMP PROPS (World X: 1448 to 4344) ---
    AddWorldProp("Assets/Props/Vehicles/veh_pickup_destroyed.png", 1800.0, 185.0, 160.0, 90.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_oil_drum_01.png", 2200.0, 185.0, 44.0, 55.0, PROP_LAYER_BACKGROUND, true);
    AddWorldProp("Assets/Props/Vehicles/veh_ambulance_burned.png", 2600.0, 185.0, 170.0, 95.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_burning_barrel_01.png", 3400.0, 185.0, 65.0, 84.0, PROP_LAYER_BACKGROUND, true);

    // --- AREA 4: RIVER CROSSING PROPS ---
    AddWorldProp("Assets/Props/Vehicles/veh_pickup_destroyed.png", 6150.0, 185.0, 160.0, 90.0, PROP_LAYER_BACKGROUND);

    // --- AREA 7: NOVAGEN OUTPOST PROPS ---
    AddWorldProp("Assets/Props/Vehicles/veh_ambulance_burned.png", 10200.0, 185.0, 170.0, 95.0, PROP_LAYER_BACKGROUND);

    props.clear();
    enemies.clear();

    // ========================================================================
    // PRE-PLACED LEVEL 2 ENEMIES (23 Pre-placed enemies across 10 areas)
    // ========================================================================
    // Area 1: Forest Entrance (2 Walkers)
    enemies.push_back(Enemy(550, 650, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(1050, 1150, kLevel1GroundY, TYPE_SPITTER));

    // Area 2: Evacuation Camp (2 Walkers, 1 Runner)
    enemies.push_back(Enemy(1950, 2050, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3150, 3250, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3750, 3850, kLevel1GroundY, TYPE_RUNNER));

    // Area 3: Deep Forest (1 Walker, 1 Heavy Infected)
    enemies.push_back(Enemy(4550, 4650, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(5050, 5150, kLevel1GroundY, TYPE_HEAVY));

    // Area 4: River Crossing (1 Walker, 1 Runner)
    enemies.push_back(Enemy(6050, 6150, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(6650, 6750, kLevel1GroundY, TYPE_RUNNER));

    // Area 5: Survivor Hideout (1 Walker)
    enemies.push_back(Enemy(7550, 7650, kLevel1GroundY, TYPE_SPITTER));

    // Area 6: Infected Forest (2 Walkers, 1 Heavy Infected, 1 Infected Hunter)
    enemies.push_back(Enemy(8850, 8950, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(9350, 9450, kLevel1GroundY, TYPE_HEAVY));
    enemies.push_back(Enemy(9650, 9750, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(9850, 9950, kLevel1GroundY, TYPE_HUNTER));

    // Area 7: NovaGen Outpost (2 Walkers, 1 Runner)
    enemies.push_back(Enemy(10450, 10550, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(10750, 10850, kLevel1GroundY, TYPE_RUNNER));
    enemies.push_back(Enemy(11050, 11150, kLevel1GroundY, TYPE_SPITTER));

    // Area 8: Research Facility (2 Walkers, 1 Heavy Infected)
    enemies.push_back(Enemy(11850, 11950, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(12250, 12350, kLevel1GroundY, TYPE_HEAVY));
    enemies.push_back(Enemy(12550, 12650, kLevel1GroundY, TYPE_SPITTER));

    // Area 9: Boss Arena (1 Alpha Hunter Boss)
    enemies.push_back(Enemy(13550, 13650, kLevel1GroundY, TYPE_ALPHA_HUNTER));

    // Area 10: Facility B Road (2 Walkers)
    enemies.push_back(Enemy(14150, 14250, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(14300, 14400, kLevel1GroundY, TYPE_SPITTER));

    m_encounterManager.Initialize(2);

    collectibles.clear();
    collectibles.push_back(Collectible(450, kLevel1GroundY, 32, 32, COL_AMMO, true, 0));
    collectibles.push_back(Collectible(1300, kLevel1GroundY, 32, 32, COL_MEDKIT, true, 0));
    collectibles.push_back(Collectible(2400, kLevel1GroundY, 32, 32, COL_FOOD, true, 0));
    collectibles.push_back(Collectible(3600, kLevel1GroundY, 32, 32, COL_AMMO, true, 0));
    collectibles.push_back(Collectible(4800, kLevel1GroundY, 32, 32, COL_WATER, true, 0));
    collectibles.push_back(Collectible(6200, kLevel1GroundY, 32, 32, COL_MEDKIT, true, 0));
    collectibles.push_back(Collectible(7800, kLevel1GroundY, 32, 32, COL_FOOD, true, 0));
    collectibles.push_back(Collectible(9100, kLevel1GroundY, 32, 32, COL_AMMO, true, 0));
    collectibles.push_back(Collectible(10500, kLevel1GroundY, 32, 32, COL_MEDKIT, true, 0));
    collectibles.push_back(Collectible(12100, kLevel1GroundY, 32, 32, COL_AMMO, true, 0));

    // Initialize rain particle simulation
    rainParticles.clear();
    for (int i = 0; i < 80; ++i) {
        rainParticles.push_back(RainParticle((double)(rand() % 1280), (double)(rand() % 720), 6.0 + (rand() % 40) / 10.0));
    }

    fogParticles.clear();
    for (int i = 0; i < 24; ++i) {
        fogParticles.push_back(FogParticle((double)(rand() % 1280), (double)(80 + rand() % 420), 0.4 + (rand() % 8) / 10.0, 0.15 + (rand() % 20) / 100.0));
    }

    printf("[GENESIS Engine] Level 2: Blackwood Forest Loaded Successfully with %d pre-placed enemies.\n", (int)enemies.size());
}

// ============================================================================
// Core Update Loop
// ============================================================================
void GameManager::Update(bool keys[], bool specialKeys[]) {
    uiAnimTime += 0.016;

    // Smooth screen transition fade out
    if (menuTransitionAlpha > 0.0) {
        menuTransitionAlpha -= 3.0 * 0.016;
        if (menuTransitionAlpha < 0.0) menuTransitionAlpha = 0.0;
    }

    // Mission Notification Fade-in & Fade-out logic
    int currentObjID = 0;
    if (bossSpawned && !bossDefeated) currentObjID = 1;
    else if (bossDefeated && !ribbonCollected) currentObjID = 2;
    else if (bossDefeated && ribbonCollected) currentObjID = 3;

    if (currentObjID != lastObjectiveID) {
        lastObjectiveID = currentObjID;
        missionNotifyTimer = 4.0; // Show banner for 4 seconds on objective change
    }

    if (missionNotifyTimer > 0.0) {
        missionNotifyTimer -= 0.016;
        missionNotifyAlpha += 4.0 * 0.016;
        if (missionNotifyAlpha > 1.0) missionNotifyAlpha = 1.0;
    } else {
        missionNotifyAlpha -= 2.0 * 0.016;
        if (missionNotifyAlpha < 0.0) missionNotifyAlpha = 0.0;
    }

    // ------------------------------------------------------------------------
    // LEVEL 1 AREA TRACKING & TRANSITION DETECTION
    // ------------------------------------------------------------------------
    int newArea = GetAreaFromPosition(player.x);
    if (currentState == STATE_VICTORY) {
        newArea = (currentLevel == 2) ? L2_AREA_LEVEL_COMPLETE : AREA_LEVEL_COMPLETE;
    }

    if (newArea != currentAreaIndex) {
        previousAreaIndex = currentAreaIndex;
        currentAreaIndex = newArea;
        areaBannerTimer = 3.0; // Briefly display area title banner on transition
    }

    if (areaBannerTimer > 0.0) {
        areaBannerTimer -= 0.016;
        areaBannerAlpha += 4.0 * 0.016;
        if (areaBannerAlpha > 1.0) areaBannerAlpha = 1.0;
    } else {
        areaBannerAlpha -= 3.0 * 0.016;
        if (areaBannerAlpha < 0.0) areaBannerAlpha = 0.0;
    }

    if (currentState == STATE_PLAYING) {
        UpdatePlaying(keys, specialKeys);
    }
}

void GameManager::UpdatePlaying(bool keys[], bool specialKeys[]) {
    // 0. Check Arin death transition to STATE_GAMEOVER
    if (player.hp <= 0 || player.state == STATE_DEAD) {
        if (player.state != STATE_DEAD) {
            player.SetState(STATE_DEAD);
        }
        deathTimer += 0.016;
        if (deathTimer >= 1.5) { // 1.5s delay to allow death animation playback
            currentState = STATE_GAMEOVER;
            menuTransitionAlpha = 1.0;
            deathTimer = 0.0;
            return;
        }
    } else {
        deathTimer = 0.0;
    }

    // Smooth HUD fade-in animation
    hudAlpha += 3.0 * 0.016;
    if (hudAlpha > 1.0) hudAlpha = 1.0;

    // Update current level area based on player position (decoupled from background slices)
    int newArea = GetAreaFromPosition(player.x);
    if (newArea != currentAreaIndex) {
        previousAreaIndex = currentAreaIndex;
        currentAreaIndex = newArea;
        areaBannerTimer = 3.0;
        areaBannerAlpha = 1.0;
    }

    // 1. Update Player Physics and animations
    player.Update(keys, specialKeys);

    // 1b. Update Controlled Dynamic Encounter System
    m_encounterManager.Update(player, gameMap, *this, 0.016f);

    // 2. Collision checks against floating/ground platforms
    const std::vector<Platform>& platforms = gameMap.GetPlatforms();
    player.isGrounded = false;

    for (size_t i = 0; i < platforms.size(); ++i) {
        const Platform& p = platforms[i];
        double topY = p.y + p.height;

        // Check horizontal overlap with player feet
        if (player.x + player.width * 0.7 > p.x && player.x + player.width * 0.3 < p.x + p.width) {
            // Landing check: falling down and player feet (player.y) are near/above platform top
            if (player.vy <= 0 && player.y >= topY - 24.0 && player.y <= topY + 24.0) {
                player.y = topY;
                player.vy = 0;
                player.isGrounded = true;
            }
        }
    }

    // 2b. Solid World Props Obstacle Collision System (Physical barriers: chairs, barrels, drums, crates, debris)
    for (size_t i = 0; i < worldProps.size(); ++i) {
        if (!worldProps[i].visible || !worldProps[i].isObstacle) continue;

        double scale = GetPropWorldScale(worldProps[i].assetPath);
        double renderW = worldProps[i].width * scale;
        double renderH = worldProps[i].height * scale;

        // Tight collision box factors tuned specifically per visible prop geometry
        double halfWidthFactor = 0.35; // Default inset (70% of total render width)
        double topHeightFactor = 0.65; // Default top height (65% of total render height)

        const std::string& path = worldProps[i].assetPath;

        if (path.find("furn_broken_chair") != std::string::npos) {
            halfWidthFactor = 0.26; // Chair visible frame is centered in texture
            topHeightFactor = 0.55;
        }
        else if (path.find("furn_broken_bench") != std::string::npos) {
            halfWidthFactor = 0.36;
            topHeightFactor = 0.50;
        }
        else if (path.find("furn_dining_table") != std::string::npos) {
            halfWidthFactor = 0.38;
            topHeightFactor = 0.60;
        }
        else if (path.find("furn_wooden_cabinet") != std::string::npos || path.find("furn_grocery_shelf") != std::string::npos) {
            halfWidthFactor = 0.38;
            topHeightFactor = 0.75;
        }
        else if (path.find("furn_broken_bed") != std::string::npos) {
            halfWidthFactor = 0.25; // Compact physical mattress & frame width allowing Arin to jump ON and OVER bed
            topHeightFactor = 0.35; // Mattress top surface height
        }
        else if (path.find("burning_barrel") != std::string::npos) {
            halfWidthFactor = 0.30; // Matches physical metal/wooden barrel body (below flames)
            topHeightFactor = 0.52;
        }
        else if (path.find("oil_drum") != std::string::npos) {
            halfWidthFactor = 0.32; // Matches oil drum cylinder
            topHeightFactor = 0.68;
        }
        else if (path.find("drum.png") != std::string::npos) {
            halfWidthFactor = 0.33;
            topHeightFactor = 0.65;
        }
        else if (path.find("crate") != std::string::npos) {
            halfWidthFactor = 0.36; // Matches wooden crate box
            topHeightFactor = 0.45; // Crate sprite has transparent space, 0.45 perfectly aligns jumping feet
        }
        else if (path.find("shopping_cart") != std::string::npos) {
            halfWidthFactor = 0.34; // Matches tool box / cart body
            topHeightFactor = 0.58;
        }
        else if (path.find("Assets__stone") != std::string::npos) {
            halfWidthFactor = 0.36; // Matches small stone / debris
            topHeightFactor = 0.45;
        }
        else if (path.find("sandbags") != std::string::npos) {
            halfWidthFactor = 0.38;
            topHeightFactor = 0.45;
        }
        else if (path.find("generator") != std::string::npos) {
            halfWidthFactor = 0.35;
            topHeightFactor = 0.60;
        }

        double obsLeft = worldProps[i].x - (renderW * halfWidthFactor);
        double obsRight = worldProps[i].x + (renderW * halfWidthFactor);
        double obsTop = worldProps[i].y + (renderH * topHeightFactor);

        double playerLeft = player.x + 12.0;
        double playerRight = player.x + (double)player.width - 12.0;

        // Solid World Prop Collision (Arin cannot walk through obstacles, can land/jump on top)
        if (playerRight > obsLeft && playerLeft < obsRight) {
            if (player.vy <= 0 && player.y >= obsTop - 20.0 && player.y <= obsTop + 24.0) {
                player.y = obsTop;
                player.vy = 0.0;
                player.isGrounded = true;
            }
            else if (player.y < obsTop - 5.0) {
                double playerCenterX = player.x + ((double)player.width / 2.0);
                if (playerCenterX < worldProps[i].x) {
                    player.x = obsLeft - ((double)player.width - 12.0);
                    if (player.vx > 0.0) player.vx = 0.0;
                }
                else {
                    player.x = obsRight - 12.0;
                    if (player.vx < 0.0) player.vx = 0.0;
                }
            }
        }
    }

    // Broken Bridge Pit Detection (Gap 1: 10620 to 10760, Gap 2: 11000 to 11150)
    bool inPitGap1 = (player.x > 10620.0 && player.x < 10760.0);
    bool inPitGap2 = (player.x > 11000.0 && player.x < 11150.0);
    bool inPit = inPitGap1 || inPitGap2;

    if (!inPit && player.y <= 185.0) {
        player.y = 185.0;
        player.vy = 0.0;
        player.isGrounded = true;
    }
    else if (inPit && player.y < 30.0) {
        // Pit safety respawn after falling into broken bridge water chasm
        if (inPitGap1) {
            player.x = 10520.0; // Safe platform before Gap 1
        } else {
            player.x = 10920.0; // Safe platform before Gap 2
        }
        player.y = 185.0; // Respawns directly on bridge platform
        player.vy = 0.0;
        player.isGrounded = true;
        player.TakeDamage(10); // Environmental hazard damage penalty
    }

    // 3. Environmental rain particle simulation update
    for (size_t i = 0; i < rainParticles.size(); ++i) {
        rainParticles[i].y -= rainParticles[i].speed * 60.0 * 0.016;
        rainParticles[i].x -= 1.5;
        if (rainParticles[i].y < 0) {
            rainParticles[i].y = 720;
            rainParticles[i].x = rand() % 1380;
        }
    }

    // 4. Horizontal camera tracking & Exit Gate collision boundary
    // Enforcement of Exit Gate solid world structure (Player cannot clip past entrance at 13120)
    if (currentLevel == 1) {
        if (player.x > 13120.0 && !bossDefeated) {
            player.x = 13120.0;
        }
    } else if (currentLevel == 2) {
        if (player.x > 14400.0) {
            player.x = 14400.0;
        }
    }

    // Check boss spawning boundary trigger
    double bossTriggerX = (currentLevel == 2) ? 13100.0 : 11800.0;
    if (player.x >= bossTriggerX && !bossSpawned) {
        bossSpawned = true;
        // Load boss stats dynamically
        for (size_t i = 0; i < enemies.size(); ++i) {
            if ((currentLevel == 2 && enemies[i].type == TYPE_ALPHA_HUNTER) ||
                (currentLevel == 1 && enemies[i].type == TYPE_ABOMINATION)) {
                bossMaxHp = enemies[i].maxHp;
                bossHp = enemies[i].hp;
                displayedBossHp = (double)enemies[i].hp;
            }
        }
    }

    // If boss fight is active, lock the player camera inside the arena bounds
    if (bossSpawned && !bossDefeated) {
        double minCam = (currentLevel == 2) ? 12700.0 : 11400.0;
        double maxCam = (currentLevel == 2) ? 13800.0 : 12600.0;
        double minPx  = (currentLevel == 2) ? 12750.0 : 11450.0;
        double maxPx  = (currentLevel == 2) ? 13850.0 : 12650.0;

        double targetCam = player.x - (1280.0 / 2.0);
        if (targetCam < minCam) targetCam = minCam;
        if (targetCam > maxCam) targetCam = maxCam;

        double curCam = gameMap.GetCameraX();
        double nextCam = curCam + (targetCam - curCam) * 0.1;
        gameMap.SetCameraX(nextCam);
        if (player.x < minPx) player.x = minPx;
        if (player.x > maxPx) player.x = maxPx;
    }
    else {
        // Normal viewport tracking
        gameMap.ApplyCameraTracking(player.x, player.y, 1280, 720);
    }

    // Update animated props (flickering fires)
    for (size_t i = 0; i < props.size(); ++i) {
        if (props[i].type == PROP_BARREL_FIRE) {
            if (rand() % 10 == 0) {
                props[i].animFrame = (props[i].animFrame + 1) % 3;
            }
        }
    }

    // 5. Water gap falling (Broken Bridge respawn logic)
    if (player.y < 0) {
        player.TakeDamage(25);
        if (player.hp > 0) {
            // Find nearest checkpoint coordinate
            double respawnX = 100;
            if (player.x >= 19600) respawnX = 19600;
            else if (player.x >= 13500) respawnX = 13500;
            else if (player.x >= 6800) respawnX = 6800;

            player.Initialize(respawnX, 220); // Spawns above safe ground checkpoint
        }
    }

    // 6. Update active Collectibles interaction (Auto pickup for resources, [E] keypress for keycard/note)
    for (size_t i = 0; i < collectibles.size(); ++i) {
        if (collectibles[i].active) {
            // Check bounding collision between player and collectible item
            bool intersectX = (player.x + player.width >= collectibles[i].x) && (collectibles[i].x + collectibles[i].width >= player.x);
            bool intersectY = (player.y + player.height >= collectibles[i].y) && (collectibles[i].y + collectibles[i].height >= player.y);

            // Story items (Keycard, Note) require manual [E] interaction, while consumables auto-pickup on collision
            if (intersectX && intersectY && collectibles[i].type != COL_KEYCARD && collectibles[i].type != COL_NOTE) {
                collectibles[i].active = false;
                score += 100;

                // Apply GDD rewards and trigger floating feedback pop-up
                switch (collectibles[i].type) {
                case COL_MEDKIT:
                    player.medkits++;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 FIRST AID MEDKIT");
                    g_pickupR = 255; g_pickupG = 100; g_pickupB = 100;
                    break;
                case COL_BATTERY:
                    player.batteryCount++;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 BATTERY");
                    g_pickupR = 0; g_pickupG = 255; g_pickupB = 200;
                    break;
                case COL_FOOD:
                    player.foodCount++;
                    player.hp = (player.hp + 15 > player.maxHp) ? player.maxHp : player.hp + 15;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 RATION (+15 HP)");
                    g_pickupR = 255; g_pickupG = 180; g_pickupB = 0;
                    break;
                case COL_WATER:
                    player.waterBottleCount++;
                    AddInventoryItem("water_bottle", 1);
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 WATER BOTTLE");
                    g_pickupR = 0; g_pickupG = 220; g_pickupB = 255;
                    break;
                case COL_SCRAP:
                    player.scrapCount++;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 SCRAP METAL");
                    g_pickupR = 200; g_pickupG = 210; g_pickupB = 220;
                    break;
                case COL_AMMO:
                    player.ammo += 15;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+15 PISTOL AMMO");
                    g_pickupR = 255; g_pickupG = 215; g_pickupB = 0;
                    break;
                case COL_COIN:
                    player.ammo += 10;
                    score += 50;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+10 AMMO (+50 PTS)");
                    g_pickupR = 255; g_pickupG = 215; g_pickupB = 0;
                    break;
                case COL_RUSTY_KEY:
                    hasKeycard = true;
                    currentState = STATE_DIALOGUE;
                    sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                    sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"An old Rusty Key! This could unlock hidden doors or emergency storage.\"");
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "RUSTY KEY ACQUIRED");
                    g_pickupR = 255; g_pickupG = 215; g_pickupB = 0;
                    break;
                case COL_KEYCARD:
                    hasKeycard = true;
                    currentState = STATE_DIALOGUE;
                    sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                    sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"A NovaGen command keycard! This will grant me access to open the steel gate checkpoint.\"");
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "NOVAGEN KEYCARD ACQUIRED");
                    g_pickupR = 255; g_pickupG = 215; g_pickupB = 0;
                    break;
                case COL_NOTE:
                    currentState = STATE_DIALOGUE;
                    sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Survivor's Clue Note");
                    if (collectibles[i].x < 10000) {
                        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"NovaGen Memo:\nEvacuation path compromised.\nConvoy heading East to Blackwood Forest.\nSubject Luna immune.\"");
                    }
                    else {
                        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"A crumpled note:\nDr. Kael took the silver-haired girl through the forest checkpoint.\nShe is our only hope...\"");
                    }
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "MISSION NOTE DISCOVERED");
                    g_pickupR = 0; g_pickupG = 230; g_pickupB = 255;
                    break;
                default:
                    break;
                }

                g_pickupX = player.x;
                g_pickupY = player.y + 160.0;
                g_pickupTimer = 1.0;
            }
        }
    }

    // 6b. Resolve player's ranged attack (pistol) against nearest enemy
    if (player.rangedAttackTriggered) {
        double bestDist = 1e9;
        int bestIndex = -1;
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (enemies[i].hp <= 0) continue;

            bool inFront = (player.isFacingRight && enemies[i].x > player.x) ||
                (!player.isFacingRight && enemies[i].x < player.x);
            if (!inFront) continue;

            double dxAbs = std::abs(enemies[i].x - player.x);
            double dyAbs = std::abs(enemies[i].y - player.y);
            if (dxAbs < 700.0 && dyAbs < 60.0 && dxAbs < bestDist) {
                bestDist = dxAbs;
                bestIndex = (int)i;
            }
        }

        if (bestIndex != -1) {
            enemies[bestIndex].TakeDamage(15);
            score += 30;
        }
        player.rangedAttackTriggered = false;
    }

    // 7. Update Enemy physics, AI states, and damage interactions
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (enemies[i].hp > 0) {
            enemies[i].Update(player.x, player.y, player.state == STATE_ATTACK_MELEE);

            // Assume falling unless hit prop top
            if (enemies[i].y > 185.0) {
                enemies[i].isGrounded = false;
            } else {
                enemies[i].isGrounded = true;
                enemies[i].y = 185.0;
                enemies[i].vy = 0.0;
            }

            // Solid World Prop Collision for Enemies
            for (size_t pIdx = 0; pIdx < worldProps.size(); ++pIdx) {
                if (!worldProps[pIdx].visible || !worldProps[pIdx].isObstacle) continue;

                double scale = GetPropWorldScale(worldProps[pIdx].assetPath);
                double renderW = worldProps[pIdx].width * scale;
                double renderH = worldProps[pIdx].height * scale;

                double halfWidthFactor = 0.35;
                double topHeightFactor = 0.65;
                const std::string& path = worldProps[pIdx].assetPath;

                if (path.find("burning_barrel") != std::string::npos) {
                    halfWidthFactor = 0.30;
                    topHeightFactor = 0.52;
                }
                else if (path.find("oil_drum") != std::string::npos) {
                    halfWidthFactor = 0.32;
                    topHeightFactor = 0.68;
                }
                else if (path.find("drum.png") != std::string::npos) {
                    halfWidthFactor = 0.33;
                    topHeightFactor = 0.65;
                }
                else if (path.find("barrel") != std::string::npos || path.find("Barrel") != std::string::npos) {
                    halfWidthFactor = 0.35;
                    topHeightFactor = 0.70;
                }
                else if (path.find("furn_broken_bed") != std::string::npos) {
                    halfWidthFactor = 0.25;
                    topHeightFactor = 0.35;
                }
                else if (path.find("furn_wooden_cabinet") != std::string::npos || path.find("furn_grocery_shelf") != std::string::npos) {
                    halfWidthFactor = 0.38;
                    topHeightFactor = 0.75;
                }
                else if (path.find("shopping_cart") != std::string::npos) {
                    halfWidthFactor = 0.34;
                    topHeightFactor = 0.58;
                }
                else if (path.find("crate") != std::string::npos) {
                    halfWidthFactor = 0.36;
                    topHeightFactor = 0.45;
                }
                else if (path.find("debris") != std::string::npos || path.find("stone") != std::string::npos) {
                    halfWidthFactor = 0.36;
                    topHeightFactor = 0.45;
                }
                else if (path.find("sandbags") != std::string::npos) {
                    halfWidthFactor = 0.38;
                    topHeightFactor = 0.45;
                }
                else if (path.find("generator") != std::string::npos) {
                    halfWidthFactor = 0.35;
                    topHeightFactor = 0.60;
                }

                double obsLeft = worldProps[pIdx].x - (renderW * halfWidthFactor);
                double obsRight = worldProps[pIdx].x + (renderW * halfWidthFactor);
                double obsTop = worldProps[pIdx].y + (renderH * topHeightFactor);

                double enemyLeft = enemies[i].x + 12.0;
                double enemyRight = enemies[i].x + (double)enemies[i].width - 12.0;

                if (enemyRight > obsLeft && enemyLeft < obsRight) {
                    if (enemies[i].y >= obsTop - 20.0 && enemies[i].y <= obsTop + 24.0) {
                        if (enemies[i].vy <= 0.0) {
                            enemies[i].y = obsTop;
                            enemies[i].vy = 0.0;
                            enemies[i].isGrounded = true;
                        }
                    }
                    else if (enemies[i].y < obsTop - 5.0) {
                        double enemyCenterX = enemies[i].x + ((double)enemies[i].width / 2.0);
                        if (enemyCenterX < worldProps[pIdx].x) {
                            enemies[i].x = obsLeft - ((double)enemies[i].width - 12.0);
                        }
                        else {
                            enemies[i].x = obsRight - 12.0;
                        }

                        if (enemies[i].state == ENEMY_CHASE || enemies[i].state == ENEMY_PATROL) {
                            if (enemies[i].isGrounded) {
                                double maxJumpPower = 520.0;
                                if (enemies[i].type == TYPE_RUNNER) maxJumpPower = 620.0;
                                else if (enemies[i].type == TYPE_RAIDER) maxJumpPower = 580.0;

                                double maxJumpHeight = (maxJumpPower * maxJumpPower) / 1800.0;
                                double heightDiff = obsTop - enemies[i].y;

                                if (heightDiff > 0 && heightDiff < maxJumpHeight - 10.0) {
                                    // Calculate exact velocity needed to clear this obstacle gracefully
                                    // v = sqrt(2 * g * h) where g = 900.0 (gravity), so 2g = 1800.0
                                    // Add a slight padding of 15px to make sure they cleanly clear it
                                    enemies[i].vy = std::sqrt(1800.0 * (heightDiff + 15.0));
                                    enemies[i].isGrounded = false;
                                } else if (heightDiff >= maxJumpHeight - 10.0) {
                                    enemies[i].avoidTimer = 45;
                                    enemies[i].avoidDirection = (enemyCenterX < worldProps[pIdx].x) ? -1 : 1;
                                }
                            }
                        }
                    }
                }
            }
            // Sync boss HP variables
            if (enemies[i].type == TYPE_ABOMINATION) {
                bossHp = enemies[i].hp;
                if (bossHp <= 0) {
                    bossDefeated = true;
                }
            }

            // Damage player if enemy is in attacking state and collides
            if (enemies[i].state == ENEMY_ATTACK) {
                if (enemies[i].type == TYPE_ABOMINATION && enemies[i].bruteAttack == BRUTE_SLAM) {
                    double slamDist = std::abs(player.x - enemies[i].x);
                    double slamDy = std::abs(player.y - enemies[i].y);
                    if (slamDist < 160.0 && slamDy < 80.0) {
                        player.TakeDamage(enemies[i].damage + 10);
                    }
                }
                else {
                    double atkExtra = (enemies[i].type == TYPE_RUNNER) ? 40.0 : ((enemies[i].type == TYPE_HEAVY) ? 45.0 : 30.0);
                    double atkX = enemies[i].isFacingRight ? enemies[i].x : (enemies[i].x - atkExtra);
                    double atkW = enemies[i].width + atkExtra;
                    bool hitX = (atkX + atkW >= player.x) && (player.x + player.width >= atkX);
                    bool hitY = (enemies[i].y + enemies[i].height >= player.y) && (player.y + player.height >= enemies[i].y);

                    if (hitX && hitY && !enemies[i].hasDealtDamage) {
                        int currentAtkFrame = enemies[i].animAttack.GetCurrentFrame();
                        int totalAtkFrames = enemies[i].animAttack.GetFrameCount();
                        bool isAtkActiveFrame = (totalAtkFrames <= 1) || (currentAtkFrame >= 1 && currentAtkFrame < totalAtkFrames - 1);

                        if (isAtkActiveFrame) {
                            player.TakeDamage(enemies[i].damage);
                            enemies[i].hasDealtDamage = true;
                        }
                    }
                }
            }

            // Mutated Brute Charge damage check
            if (enemies[i].type == TYPE_ABOMINATION && enemies[i].vx > 2.0) {
                if (enemies[i].CheckPlayerCollision(player.x, player.y, player.width, player.height)) {
                    player.TakeDamage(enemies[i].damage + 15);
                }
            }

            // Raider gunfire line-of-sight check
            if (enemies[i].type == TYPE_RAIDER && enemies[i].rangedShotFired) {
                bool facingPlayer = (enemies[i].isFacingRight && player.x > enemies[i].x) ||
                    (!enemies[i].isFacingRight && player.x < enemies[i].x);
                double shotDist = std::abs(player.x - enemies[i].x);
                double shotDy = std::abs(player.y - enemies[i].y);
                if (facingPlayer && shotDist < 500.0 && shotDy < 60.0) {
                    player.TakeDamage(enemies[i].damage);
                }
                enemies[i].rangedShotFired = false;
            }

            // Damage enemy if player attacks them during active katana slash window
            int currentFrame = player.animAttack.GetCurrentFrame();
            int totalPlayerAtkFrames = player.animAttack.GetFrameCount();
            bool isKatanaActiveFrame = (totalPlayerAtkFrames <= 1) || (currentFrame >= 0 && currentFrame <= 6);

            if (player.state == STATE_ATTACK_MELEE && isKatanaActiveFrame && enemies[i].lastHitAttackID != player.currentAttackID) {
                double hitboxX = player.isFacingRight ? (player.x + 15.0) : (player.x - 70.0);
                double hitboxW = 115.0;
                double hitboxY = player.y;
                double hitboxH = player.height;

                bool hitX = (hitboxX + hitboxW >= enemies[i].x) && (enemies[i].x + enemies[i].width >= hitboxX);
                bool hitY = (hitboxY + hitboxH >= enemies[i].y) && (enemies[i].y + enemies[i].height >= hitboxY);

                if (hitX && hitY) {
                    enemies[i].TakeDamage(35);
                    score += 50;
                    enemies[i].lastHitAttackID = player.currentAttackID;
                    player.hasDealtDamageThisAttack = true;

                    // Play Katana Hit Sound (Heavy Enemy / Mutated Brute vs Normal Enemy)
                    bool isHeavyEnemy = (enemies[i].type == TYPE_HEAVY || enemies[i].type == TYPE_ABOMINATION);
                    std::string hitSound = isHeavyEnemy ? "katana_hit_heavy.wav" : "katana_hit_enemy.wav";
                    std::string fallbackSound = isHeavyEnemy ? "Assets/Sound/Arin/Katana Attack/Katana_Hit_Heavy.wav" : "Assets/Sound/Arin/Katana Attack/katana hit enemy.wav";
                    PlayAudioFile("Sounds/Katana/" + hitSound, fallbackSound);
                }
            }
        }
        else {
            // Process death frames for enemy
            enemies[i].Update(player.x, player.y);

            // Check if boss died
            if (enemies[i].type == TYPE_ABOMINATION || enemies[i].type == TYPE_ALPHA_HUNTER) {
                bossDefeated = true;
                bossHp = 0;
            }
        }
    }

    // Smooth boss HP interpolation for trailing damage hit lag bar
    if (bossSpawned) {
        if (displayedBossHp > (double)bossHp) {
            displayedBossHp -= (displayedBossHp - (double)bossHp) * 0.08;
            if (displayedBossHp < (double)bossHp) displayedBossHp = (double)bossHp;
        } else if (displayedBossHp < (double)bossHp) {
            displayedBossHp = (double)bossHp;
        }
    }

    // 8. Exit Gate Ending Trigger
    double exitPosTrigger = (currentLevel == 2) ? 14350.0 : 12900.0;
    if (player.x >= exitPosTrigger && bossDefeated) {
        if (currentLevel == 1 && !hasKeycard) {
            if (currentState == STATE_PLAYING) {
                currentState = STATE_DIALOGUE;
                sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"The steel gate is locked. I need a NovaGen keycard from the quarantine checkpoint.\"");
            }
        }
        else if (currentLevel == 1 && !ribbonCollected) {
            if (currentState == STATE_PLAYING) {
                currentState = STATE_DIALOGUE;
                sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Luna's silver-blue ribbon! It's caught on the steel gate latch... She survived. I will find you, Luna!\"");
                ribbonCollected = true;
            }
        }
        else if (currentState != STATE_DIALOGUE && currentState != STATE_VICTORY) {
            currentState = STATE_VICTORY;
            menuTransitionAlpha = 1.0;
            leaderboard.AddScore("Arin", score);
        }
    }

    // 9. Update Contextual Interaction Prompt
    activePromptText = "";
    double camX = gameMap.GetCameraX();
    double camY = gameMap.GetCameraY();

    for (size_t i = 0; i < collectibles.size(); ++i) {
        if (collectibles[i].active) {
            double dist = std::abs(player.x - collectibles[i].x);
            if (dist < 70.0) {
                switch (collectibles[i].type) {
                case COL_NOTE: activePromptText = "[E] READ DOCUMENT"; break;
                case COL_KEYCARD: activePromptText = "[E] COLLECT NOVAGEN KEYCARD"; break;
                case COL_RUSTY_KEY: activePromptText = "[E] PICK UP GATE KEY"; break;
                case COL_MEDKIT: activePromptText = "[E] PICK UP MEDKIT"; break;
                case COL_AMMO: activePromptText = "[E] PICK UP AMMO"; break;
                case COL_BATTERY: activePromptText = "[E] PICK UP BATTERY"; break;
                case COL_FOOD: activePromptText = "[E] PICK UP RATION"; break;
                case COL_WATER: activePromptText = "[E] PICK UP WATER BOTTLE"; break;
                default: activePromptText = "[E] PICK UP ITEM"; break;
                }
                activePromptX = (int)(collectibles[i].x - camX);
                activePromptY = (int)(collectibles[i].y - camY + collectibles[i].height + 30.0);
                break;
            }
        }
    }

    if (activePromptText.empty() && player.x >= 13000 && bossDefeated) {
        activePromptText = "[E] Interact with Exit Gate";
        activePromptX = (int)(player.x - camX);
        activePromptY = (int)(player.y - camY + player.height + 30.0);
    }

    // 10. Update Dynamic Encounter System (zones, ambushes, difficulty director)
    m_encounterManager.Update(player, gameMap, *this, 0.016f);
}

// ============================================================================
// Main Dispatch Renderer
// ============================================================================
void GameManager::Render() {
    switch (currentState) {
    case STATE_MENU:
        RenderMenu();
        break;
    case STATE_PLAYING:
        RenderPlaying();
        break;
    case STATE_DIALOGUE:
        RenderDialogue();
        break;
    case STATE_PAUSED:
        RenderPlaying(); // Render gameplay frame underneath in frozen state
        UI::DrawPauseMenu(mouseX, mouseY, isMouseDown, uiAnimTime, pauseSubMenu);
        break;
    case STATE_GAMEOVER:
        UI::DrawGameOver(mouseX, mouseY, isMouseDown, uiAnimTime);
        break;
    case STATE_VICTORY:
        UI::DrawLevelComplete(mouseX, mouseY, isMouseDown, uiAnimTime);
        break;
    case STATE_LEADERBOARD:
        RenderLeaderboard();
        break;
    }

    // Render active notifications
    UI::DrawNotification();

    // Always render custom game crosshair cursor on top
    RenderCursor();
}

// ============================================================================
// State Specific Renderers (Menu, Gameplay, Dialogue, End Screens, Archive)
// ============================================================================
void GameManager::RenderMenu() {
    // 1. Draw Main Menu Background Graphic
    if (g_texMainMenuBg != 0) {
        iShowImage(0, 0, 1280, 720, g_texMainMenuBg);
    } else {
        iSetColor(10, 12, 18);
        iFilledRectangle(0, 0, 1280, 720);
    }

    // 2. Top Carved Banner Slot: Title Header
    const char* titleStr = "GENESIS: WHISPERS OF THE INFECTED";
    DrawOutlinedText(445, 525, titleStr, GLUT_BITMAP_TIMES_ROMAN_24, 0, 220, 255);

    // 3. Slot 1: Play Game
    RenderMenuButtonSlot(1, 540, 440, "1. START SURVIVAL", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 4. Slot 2: Leaderboard
    RenderMenuButtonSlot(2, 555, 362, "2. LEADERBOARD", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 5. Slot 3: Exit Game
    RenderMenuButtonSlot(3, 565, 285, "3. EXIT GAME", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 6. Bottom Detail Slot: Footer Instructions
    const char* footerStr = "Press [1], [2], [3] or Click Options to Select";
    DrawShadowText(510, 148, footerStr, GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
}

// ============================================================================
// Independent Environment Prop System Implementation
// ============================================================================
void GameManager::AddWorldProp(const std::string& assetPath, double x, double y, double width, double height, PropLayer layer, bool isObstacle) {
    WorldProp wp;
    wp.x = x;
    wp.y = y;
    wp.width = width;
    wp.height = height;
    wp.assetPath = assetPath;
    wp.textureID = ResourceManager::GetInstance().GetTexture(assetPath);
    wp.layer = layer;
    wp.visible = true;
    wp.isObstacle = isObstacle;

    worldProps.push_back(wp);
    printf("[Prop System] Registered WorldProp: %s at (%.1f, %.1f) scale (%.1f x %.1f) obstacle: %d\n", assetPath.c_str(), x, y, width, height, isObstacle ? 1 : 0);
}

double GameManager::GetPropWorldScale(const std::string& assetPath) const {
    // 1. Small props (Scrap, papers, notes, first aid, small bushes)
    if (assetPath.find("Posters/") != std::string::npos ||
        assetPath.find("Items/") != std::string::npos ||
        assetPath.find("dry_bush") != std::string::npos ||
        assetPath.find("paper") != std::string::npos ||
        assetPath.find("note") != std::string::npos ||
        assetPath.find("scrap") != std::string::npos) {
        return kPropScaleSmall; // 1.1x
    }

    // 2. Human-scale props (Chair, Table, Bed, Shopping cart, Furniture)
    if (assetPath.find("chair") != std::string::npos ||
        assetPath.find("shopping_cart") != std::string::npos ||
        assetPath.find("Furniture/") != std::string::npos ||
        assetPath.find("table") != std::string::npos ||
        assetPath.find("bed") != std::string::npos ||
        assetPath.find("bench") != std::string::npos ||
        assetPath.find("cabinet") != std::string::npos ||
        assetPath.find("shelf") != std::string::npos) {
        return 1.8;
    }

    // 3. Industrial props - Vehicles & Large Structures
    // Ambulance: Significantly larger than Arin (roof ~295px vs Arin 195px, length ~527px)
    if (assetPath.find("ambulance") != std::string::npos) {
        return 3.1;
    }

    // Military pickup / trucks: Human/world scale (roof ~243px vs Arin 195px, length ~432px)
    if (assetPath.find("pickup") != std::string::npos ||
        assetPath.find("truck") != std::string::npos) {
        return 2.7;
    }

    // Destroyed cars: Realistic vehicle size (roof ~192px vs Arin 195px, length ~360px)
    if (assetPath.find("destroyed_car") != std::string::npos ||
        assetPath.find("car_") != std::string::npos) {
        return 2.4;
    }

    // Large buildings / checkpoints / shelters
    if (assetPath.find("bld_military_checkpoint") != std::string::npos ||
        assetPath.find("Quarantine_Checkpoint") != std::string::npos) {
        return 2.6;
    }
    if (assetPath.find("bld_grocery_store") != std::string::npos) {
        return 2.4;
    }

    // Dedicated calibrated scale for Dead Trees (Taller and larger relative to Arin)
    if (assetPath.find("nature_dead_tree") != std::string::npos) {
        return 2.5;
    }

    // Tall structures / nature (Poles, lamps, watchtower)
    if (assetPath.find("telephone_pole") != std::string::npos ||
        assetPath.find("street_lamp") != std::string::npos ||
        assetPath.find("watchtower") != std::string::npos) {
        return kPropScaleTall; // 1.8x
    }

    // Dedicated calibrated scale for Exit Gate (586px width x 440px height)
    if (assetPath.find("Exit_Gate") != std::string::npos) {
        return 1.0;
    }

    // Other large vehicle/building fallbacks
    if (assetPath.find("Vehicles/") != std::string::npos ||
        assetPath.find("Military/") != std::string::npos ||
        assetPath.find("Buildings/") != std::string::npos ||
        assetPath.find("veh_") != std::string::npos ||
        assetPath.find("bld_") != std::string::npos) {
        return 2.5;
    }

    // 4. Industrial props (Default: Barrels, oil drums, crates, fences, sandbags, generators, stones/rubble)
    return 1.8;
}

double GameManager::GetPropGroundOffset(const std::string& assetPath) const {
    // 1. Burning barrels and oil drums
    if (assetPath.find("burning_barrel") != std::string::npos ||
        assetPath.find("oil_drum") != std::string::npos ||
        assetPath.find("drum.png") != std::string::npos) {
        return 13.0; // Lift barrel/drum base flush onto top dirt surface
    }

    // 2. Street lamps & Telephone poles
    if (assetPath.find("street_lamp") != std::string::npos ||
        assetPath.find("telephone_pole") != std::string::npos) {
        return 14.0; // Lift lamp/pole base flush onto top dirt surface
    }

    // 3. Raider Watchtower & Military Buildings
    if (assetPath.find("watchtower") != std::string::npos ||
        assetPath.find("bld_military_checkpoint") != std::string::npos ||
        assetPath.find("Quarantine_Checkpoint") != std::string::npos) {
        return 16.0; // Lift watchtower legs & building bases flush onto top dirt surface
    }

    // 4. Shopping Cart
    if (assetPath.find("shopping_cart") != std::string::npos) {
        return 12.0; // Lift shopping cart so wheels align flush on top of the ground line
    }

    // 5. Sandbags, Generators & Fences
    if (assetPath.find("sandbags") != std::string::npos ||
        assetPath.find("generator") != std::string::npos ||
        assetPath.find("broken_fence") != std::string::npos) {
        return 10.0;
    }

    return 0.0;
}

void GameManager::RenderWorldProps(PropLayer layer, double camX, double camY) {
    // Enable OpenGL Alpha Blending for clean PNG transparency across all prop textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const std::vector<VehicleSpawnPoint>& vehicles = m_encounterManager.GetVehicles();

    for (size_t i = 0; i < worldProps.size(); ++i) {
        if (!worldProps[i].visible || worldProps[i].layer != layer) continue;

        double scale = GetPropWorldScale(worldProps[i].assetPath);
        double renderW = worldProps[i].width * scale;
        double renderH = worldProps[i].height * scale;

        // Bottom-Center Ground Anchor Calculation:
        // Position (x, y) represents the bottom-center point touching the ground.
        double renderX = (worldProps[i].x - camX) - (renderW / 2.0);
        double renderY = worldProps[i].y - camY;

        // Configurable Ground Alignment Offset:
        if (std::abs(worldProps[i].y - kLevel1GroundY) < 1.0) {
            double groundOffset = GetPropGroundOffset(worldProps[i].assetPath);
            renderY += (-6.0 + groundOffset);
        }

        // Viewport frustum culling check (-100 to 1380)
        if (renderX + renderW >= -100 && renderX <= 1380) {
            // Check if this vehicle prop is currently in warning/shake state
            bool isWarning = false;
            double shakeOffsetX = 0.0;

            if (worldProps[i].assetPath.find("veh_") != std::string::npos ||
                worldProps[i].assetPath.find("car") != std::string::npos ||
                worldProps[i].assetPath.find("ambulance") != std::string::npos ||
                worldProps[i].assetPath.find("pickup") != std::string::npos) {
                
                for (size_t vIdx = 0; vIdx < vehicles.size(); ++vIdx) {
                    if (vehicles[vIdx].isWarningActive && std::abs(vehicles[vIdx].x - worldProps[i].x) < 100.0) {
                        isWarning = true;
                        // Fast jitter shake animation (±4px offset)
                        shakeOffsetX = (double)((rand() % 9) - 4);
                        break;
                    }
                }
            }

            double drawX = renderX + shakeOffsetX;

            // Render grounded contact shadow underneath Exit Gate base
            if (worldProps[i].assetPath.find("Exit_Gate") != std::string::npos) {
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                // Soft dark ambient occlusion contact shadow on terrain line
                glColor4f(0.02f, 0.04f, 0.06f, 0.55f);
                glVertex2f((float)(drawX + 15.0), (float)(renderY + 4.0));
                glVertex2f((float)(drawX + renderW - 15.0), (float)(renderY + 4.0));
                glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
                glVertex2f((float)(drawX + renderW - 5.0), (float)(renderY - 10.0));
                glVertex2f((float)(drawX + 5.0), (float)(renderY - 10.0));
                glEnd();
                glEnable(GL_TEXTURE_2D);
            }

            if (worldProps[i].textureID != 0) {
                iShowImage((int)drawX, (int)renderY, (int)renderW, (int)renderH, worldProps[i].textureID);
            }

            // Draw warning hazard effect overlay (flashing red alert box & warning header)
            if (isWarning) {
                glDisable(GL_TEXTURE_2D);
                glLineWidth(2.0f);
                
                double alpha = 0.5 + 0.5 * sin(uiAnimTime * 20.0);
                glColor4f(1.0f, 0.2f, 0.1f, (float)alpha);
                
                glBegin(GL_LINE_LOOP);
                glVertex2f((float)drawX, (float)renderY);
                glVertex2f((float)(drawX + renderW), (float)renderY);
                glVertex2f((float)(drawX + renderW), (float)(renderY + renderH));
                glVertex2f((float)drawX, (float)(renderY + renderH));
                glEnd();

                DrawShadowText((int)(drawX + (renderW / 2.0) - 75.0), (int)(renderY + renderH + 18.0), "! VEHICLE AMBUSH !", GLUT_BITMAP_HELVETICA_12, 255, 60, 60);

                glEnable(GL_TEXTURE_2D);
            }
        }
    }
}

void GameManager::RenderPlaying() {
    double camX = gameMap.GetCameraX();
    double camY = gameMap.GetCameraY();

    // ========================================================================
    // LAYER 1: FAR BACKGROUND (Parallax Factor 0.15 - Sky & Distant Horizon)
    // ========================================================================
    gameMap.RenderFarBackground(camX, bossDefeated);

    // ========================================================================
    // LAYER 2: MIDGROUND (Parallax Factor 0.45 - Medium-Distance Trees & Scenery)
    // ========================================================================
    gameMap.RenderMidground(camX, bossDefeated);

    // ========================================================================
    // LAYER 3: GAMEPLAY WORLD GROUND SURFACE (Parallax Factor 1.00 - Synchronized Ground)
    // ========================================================================
    gameMap.RenderGroundSurface(camX);

    // ========================================================================
    // LAYER 3 (Cont): WATER / RIVER & BRIDGE STRUCTURE (Factor 1.00)
    // ========================================================================
    gameMap.RenderWater(camX, camY);
    gameMap.RenderBridgeAndEnvironmentSprites(camX, camY);
    RenderWorldProps(PROP_LAYER_BACKGROUND, camX, camY);

    // Render level props (Environmental obstacles & burning barrels - Factor 1.00)
    for (size_t i = 0; i < props.size(); ++i) {
        double screenPx = props[i].x - camX;
        double screenPy = props[i].y - camY;

        if (screenPx + props[i].width >= -100 && screenPx <= 1380) {
            int px = (int)screenPx;
            int py = (int)screenPy;
            int pw = (int)props[i].width;
            int ph = (int)props[i].height;

            switch (props[i].type) {
            case PROP_BARREL_FIRE: {
                // Animated flickering fire flames on top of barrel
                double fTime = uiAnimTime * 12.0 + i;
                int fHeight = 24 + (int)(sin(fTime) * 6.0);
                iSetColor(255, 140, 0); // Orange outer fire
                iFilledRectangle(px + 8, py + ph - 4, pw - 16, fHeight);
                iSetColor(255, 220, 0); // Yellow inner core fire
                iFilledRectangle(px + 14, py + ph - 4, pw - 28, fHeight - 8);
                break;
            }
            case PROP_RIBBON: {
                // Luna's silver-blue ribbon fluttering at exit gate
                double flutter = sin(uiAnimTime * 8.0) * 6.0;
                iSetColor(0, 220, 255); // Silver-blue glow
                iFilledRectangle(px, py, pw, ph);
                iSetColor(255, 255, 255);
                iLine(px + 4, py + 4, px + pw - 4 + (int)flutter, py + ph - 4);
                DrawOutlinedText(px - 20, py + ph + 8, "LUNA'S RIBBON", GLUT_BITMAP_HELVETICA_10, 0, 240, 255);
                break;
            }
            default:
                break;
            }
        }
    }

    // Render Collectibles aligned statically to ground baseline (Factor 1.00)
    for (size_t i = 0; i < collectibles.size(); ++i) {
        if (collectibles[i].active) {
            double screenPx = collectibles[i].x - camX;
            double groundY = collectibles[i].y - (std::abs(collectibles[i].y - kLevel1GroundY) < 1.0 ? 6.0 : 0.0);
            double bobY = groundY;

            if (screenPx + 80 >= -100 && screenPx - 80 <= 1380) {
                int px = (int)screenPx;
                int py = (int)bobY;

                unsigned int itemTex = 0;
                const char* itemLabel = "ITEM";
                int lR = 255, lG = 255, lB = 255;
                int itemDrawW = 44;
                int itemDrawH = 44;

                switch (collectibles[i].type) {
                case COL_MEDKIT:
                    itemTex = (collectibles[i].subType == 1 && g_texItemBandage != 0) ? g_texItemBandage : g_texItemFirstAid;
                    itemLabel = (collectibles[i].subType == 1) ? "BANDAGE" : "FIRST AID";
                    lR = 255; lG = 100; lB = 100;
                    itemDrawW = 44; itemDrawH = 44;
                    break;
                case COL_FOOD:
                    itemTex = (collectibles[i].subType == 1 && g_texItemApple != 0) ? g_texItemApple : g_texItemBread;
                    itemLabel = (collectibles[i].subType == 1) ? "APPLE" : "BREAD";
                    lR = 255; lG = 180; lB = 0;
                    itemDrawW = 40; itemDrawH = 40;
                    break;
                case COL_WATER:
                    itemTex = g_texItemWaterBottle;
                    itemLabel = "WATER";
                    lR = 0; lG = 220; lB = 255;
                    itemDrawW = 36; itemDrawH = 48;
                    break;
                case COL_SCRAP:
                    itemTex = g_texItemScrapMetal;
                    itemLabel = "SCRAP METAL";
                    lR = 200; lG = 210; lB = 220;
                    itemDrawW = 44; itemDrawH = 44;
                    break;
                case COL_RUSTY_KEY:
                    itemTex = g_texItemRustyKey;
                    itemLabel = "GATE KEY";
                    lR = 255; lG = 215; lB = 0;
                    itemDrawW = 42; itemDrawH = 42;
                    break;
                case COL_KEYCARD:
                    if (g_texItemNovagenKeycard == 0) {
                        g_texItemNovagenKeycard = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/Novagen_keycard.png").c_str());
                        if (g_texItemNovagenKeycard == 0) {
                            g_texItemNovagenKeycard = ResourceManager::GetInstance().GetTexture("Assets/Items/KeyItems/Novagen_keycard.png");
                        }
                    }
                    itemTex = g_texItemNovagenKeycard;
                    itemLabel = "NOVAGEN KEYCARD";
                    lR = 255; lG = 215; lB = 0;
                    itemDrawW = 48; itemDrawH = 48;
                    break;
                case COL_COIN:
                    itemTex = g_texItemCoin;
                    itemLabel = "OLD CURRENCY";
                    lR = 255; lG = 215; lB = 0;
                    itemDrawW = 38; itemDrawH = 38;
                    break;
                case COL_AMMO:
                    itemTex = g_texItemCoin;
                    itemLabel = (collectibles[i].subType == 1) ? "12GA SHELLS" : "9MM AMMO";
                    lR = 255; lG = 215; lB = 0;
                    itemDrawW = 42; itemDrawH = 42;
                    break;
                case COL_BATTERY:
                    if (g_texItemBattery == 0) {
                        g_texItemBattery = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/battery.png").c_str());
                        if (g_texItemBattery == 0) {
                            g_texItemBattery = ResourceManager::GetInstance().GetTexture("Assets/Items/KeyItems/battery.png");
                        }
                    }
                    itemTex = g_texItemBattery;
                    itemLabel = "BATTERY";
                    lR = 0; lG = 255; lB = 200;
                    itemDrawW = 38; itemDrawH = 48;
                    break;
                case COL_NOTE:
                    if (g_texItemMissionNote == 0) {
                        g_texItemMissionNote = iLoadImage((char*)GetAssetPath("Assets/Items/Documents/Mission_note.png").c_str());
                        if (g_texItemMissionNote == 0) {
                            g_texItemMissionNote = ResourceManager::GetInstance().GetTexture("Assets/Items/Documents/Mission_note.png");
                        }
                    }
                    itemTex = g_texItemMissionNote;
                    itemLabel = "MISSION NOTE";
                    lR = 0; lG = 230; lB = 255;
                    itemDrawW = 44; itemDrawH = 44;
                    break;
                default:
                    break;
                }

                int drawX = px - itemDrawW / 2;
                int drawY = py;

                // Render item sprite texture or clean gold marker fallback
                if (itemTex != 0) {
                    iShowImage(drawX, drawY, itemDrawW, itemDrawH, itemTex);
                } else {
                    iSetColor(255, 215, 0);
                    iFilledRectangle(drawX, drawY + 6, itemDrawW, itemDrawH - 12);
                    iSetColor(20, 20, 30);
                    iRectangle(drawX, drawY + 6, itemDrawW, itemDrawH - 12);
                }

                // Outlined text label above item
                int labelX = px - ((int)strlen(itemLabel) * 6) / 2;
                DrawOutlinedText(labelX, drawY + itemDrawH + 6, itemLabel, GLUT_BITMAP_HELVETICA_10, lR, lG, lB);
            }
        }
    }

    // ========================================================================
    // LAYER 3: CHARACTERS (Enemies & Player Arin - Factor 1.00)
    // ========================================================================
    for (size_t i = 0; i < enemies.size(); ++i) {
        double screenEx = enemies[i].x - camX;
        double screenEy = enemies[i].y - camY;

        if (screenEx + enemies[i].width >= -100 && screenEx <= 1380) {
            enemies[i].Render(camX, camY);

            // Draw Health Bar if damaged and alive
            if (enemies[i].hp > 0 && enemies[i].hp < enemies[i].maxHp) {
                iSetColor(30, 30, 40);
                iFilledRectangle(screenEx, screenEy + enemies[i].height + 5, enemies[i].width, 6);
                iSetColor(220, 40, 40);
                double hpPercent = (double)enemies[i].hp / enemies[i].maxHp;
                iFilledRectangle(screenEx, screenEy + enemies[i].height + 5, enemies[i].width * hpPercent, 6);
            }
        }
    }

    player.Render(camX, camY);

    // ========================================================================
    // LAYER 4: FOREGROUND OBJECTS (Foreground props in front of characters - Factor 1.00)
    // ========================================================================
    RenderWorldProps(PROP_LAYER_FOREGROUND, camX, camY);

    // ========================================================================
    // LAYER 5: FOREGROUND ATMOSPHERIC PARALLAX (Parallax Factor 1.15 - Rain & Foliage)
    // ========================================================================
    gameMap.RenderForeground(camX, uiAnimTime);

    // ========================================================================
    // EFFECTS (Floating popups, particle effects, HUD overlays)
    // ========================================================================
    if (g_pickupTimer > 0.0) {
        g_pickupTimer -= 0.016;
        g_pickupY += 0.8;
        double screenPx = g_pickupX - camX;
        double screenPy = g_pickupY - camY;
        if (screenPx >= -100 && screenPx <= 1380) {
            DrawOutlinedText((int)screenPx - 30, (int)screenPy, g_pickupText, GLUT_BITMAP_HELVETICA_12, g_pickupR, g_pickupG, g_pickupB);
        }
    }

    // ========================================================================
    // SURVIVAL GAME HUD (Screen Anchored Layout)
    // ========================================================================
    if (hudAlpha >= 0.05) {
        const char* activeObjText = "Escape the Fallen Village";
        if (!hasKeycard && player.x >= 8000) {
            activeObjText = "Find NovaGen Keycard in Quarantine Zone";
        }
        else if (hasKeycard && !bossDefeated && !bossSpawned) {
            activeObjText = "Reach the Steel Exit Gate";
        }
        if (bossSpawned && !bossDefeated) {
            activeObjText = "DEFEAT MUTATED BRUTE (FINAL BOSS)";
        }
        else if (bossDefeated && !ribbonCollected) {
            activeObjText = "Reach the Steel Exit Gate";
        }
        else if (bossDefeated && ribbonCollected) {
            activeObjText = "Press ENTER to Escape";
        }

        // Draw Full In-Game Gameplay HUD
        UI::DrawHUD(player, score, activeObjText, GetAreaName(currentAreaIndex), missionNotifyTimer, areaBannerAlpha, GetCurrentChapterName());

        // Render Contextual Interaction Prompt if active
        if (!activePromptText.empty()) {
            UI::DrawInteractionPrompt(activePromptText.c_str(), activePromptX, activePromptY);
        }

        // Render Boss Health Bar centered at top if Boss fight active
        if (bossSpawned && !bossDefeated) {
            const char* bName = (currentLevel == 2) ? "ALPHA HUNTER" : "MUTATED BRUTE";
            UI::DrawBossHealthBar(bName, bossHp, bossMaxHp, displayedBossHp);
        }
    }

    // ------------------------------------------------------------------------
    // INVENTORY OVERLAY (Rendered when TAB or [I] is pressed)
    // ------------------------------------------------------------------------
    if (showInventory) {
        // Ensure textures are loaded
        LoadInventoryTextures();

        // Semi-transparent dark background dimmer
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, 1280, 720);

        // Center Inventory Panel (using ui_inventory_panel.png / inventory__panel.png)
        int panelX = 340, panelY = 140, panelW = 600, panelH = 440;
        if (g_texInventoryPanel != 0) {
            iShowImage(panelX, panelY, panelW, panelH, g_texInventoryPanel);
        } else {
            iSetColor(12, 16, 24);
            iFilledRectangle(panelX, panelY, panelW, panelH);
            iSetColor(0, 180, 220);
            iRectangle(panelX, panelY, panelW, panelH);
        }

        // --------------------------------------------------------------------
        // EMBEDDED RUSTED METAL TITLE PLATE (ATTACHED TO INVENTORY FRAME)
        // --------------------------------------------------------------------
        int plateW = 340;
        int plateH = 34;
        int plateX = panelX + (panelW - plateW) / 2;
        int plateY = panelY + panelH - 42;

        // 1. Dark engraved inset groove in top frame
        iSetColor(10, 12, 14);
        iFilledRectangle(plateX, plateY, plateW, plateH);

        // 2. Weathered gunmetal & rusted iron label plate
        iSetColor(32, 36, 42);
        iFilledRectangle(plateX + 2, plateY + 2, plateW - 4, plateH - 4);

        // 3. Post-apocalyptic rust details & heavy worn bevel
        iSetColor(58, 64, 72);
        iRectangle(plateX + 1, plateY + 1, plateW - 2, plateH - 2);
        iSetColor(125, 55, 18); // Natural rust stain highlights
        iFilledRectangle(plateX + 8, plateY + 2, 28, 2);
        iFilledRectangle(plateX + plateW - 36, plateY + 2, 28, 2);
        iFilledRectangle(plateX + 14, plateY + plateH - 4, 20, 2);

        // 4. Corner rivets/bolts (physically attaching label to panel frame)
        iSetColor(140, 135, 125);
        iFilledRectangle(plateX + 5, plateY + plateH - 7, 4, 4);
        iFilledRectangle(plateX + plateW - 9, plateY + plateH - 7, 4, 4);
        iFilledRectangle(plateX + 5, plateY + 3, 4, 4);
        iFilledRectangle(plateX + plateW - 9, plateY + 3, 4, 4);

        iSetColor(18, 16, 14);
        iRectangle(plateX + 5, plateY + plateH - 7, 4, 4);
        iRectangle(plateX + plateW - 9, plateY + plateH - 7, 4, 4);
        iRectangle(plateX + 5, plateY + 3, 4, 4);
        iRectangle(plateX + plateW - 9, plateY + 3, 4, 4);

        // 5. Left side Survival Backpack Icon
        int iconSize = 22;
        int iconX = plateX + 18;
        int iconY = plateY + (plateH - iconSize) / 2;

        if (g_texBackpackIcon != 0) {
            iShowImage(iconX, iconY, iconSize, iconSize, g_texBackpackIcon);
        } else {
            iSetColor(45, 52, 46);
            iFilledRectangle(iconX + 2, iconY + 2, 18, 18);
            iSetColor(70, 80, 72);
            iFilledRectangle(iconX + 4, iconY + 12, 14, 6);
            iSetColor(28, 32, 30);
            iRectangle(iconX + 2, iconY + 2, 18, 18);
        }

        // 6. Title Text: "SURVIVAL INVENTORY"
        // Weathered dirty white / silver text engraved into metal plate
        int titleX = iconX + iconSize + 16;
        int titleY = plateY + 9;

        // Dark engraved inset shadow
        iSetColor(12, 10, 8);
        iText(titleX + 1, titleY - 1, "SURVIVAL INVENTORY", GLUT_BITMAP_HELVETICA_18);
        iText(titleX + 2, titleY - 2, "SURVIVAL INVENTORY", GLUT_BITMAP_HELVETICA_18);

        // Dirty white / silver engraved text
        iSetColor(210, 205, 195);
        iText(titleX, titleY, "SURVIVAL INVENTORY", GLUT_BITMAP_HELVETICA_18);

        // Render Inventory Grid Slots (using ui_inventory_slot.png / single_empty_inventory_slot.png)
        int cols = 4, rows = 3;
        int slotW = 85, slotH = 85;
        int startX = panelX + 70;
        int startY = panelY + panelH - 160;
        int gapX = 35, gapY = 20;

        int hoveredSlotIdx = -1;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int slotIdx = r * cols + c;
                int slotX = startX + c * (slotW + gapX);
                int slotY = startY - r * (slotH + gapY);

                // 1. Draw Slot Background Box
                if (g_texInventorySlot != 0) {
                    iShowImage(slotX, slotY, slotW, slotH, g_texInventorySlot);
                } else {
                    iSetColor(20, 25, 35);
                    iFilledRectangle(slotX, slotY, slotW, slotH);
                    iSetColor(0, 150, 180);
                    iRectangle(slotX, slotY, slotW, slotH);
                }

                // Check Hover on Slot
                if (mouseX >= slotX && mouseX <= slotX + slotW && mouseY >= slotY && mouseY <= slotY + slotH) {
                    // Hover highlight frame
                    iSetColor(0, 220, 255);
                    iRectangle(slotX - 1, slotY - 1, slotW + 2, slotH + 2);

                    if (slotIdx < 12 && inventory[slotIdx].isOccupied) {
                        hoveredSlotIdx = slotIdx;
                    }
                }

                // 2. Draw Occupied Item inside slot
                if (slotIdx < 12 && inventory[slotIdx].isOccupied) {
                    const InventoryItem& item = inventory[slotIdx];

                    // Scale & Center item icon inside slot box (52x52 inside 85x85)
                    int iconSize = 52;
                    int iconX = slotX + (slotW - iconSize) / 2;
                    int iconY = slotY + (slotH - iconSize) / 2;

                    if (item.textureID != 0) {
                        iShowImage(iconX, iconY, iconSize, iconSize, item.textureID);
                    } else {
                        // Fallback shape if texture failed to load
                        iSetColor(0, 180, 220);
                        iFilledRectangle(iconX, iconY, iconSize, iconSize);
                    }

                    // Stack Count Badge (bottom-right corner)
                    if (item.count > 1) {
                        char countStr[16];
                        sprintf_s(countStr, sizeof(countStr), "x%d", item.count);

                        // Count badge background pill for high legibility
                        iSetColor(5, 10, 18);
                        iFilledRectangle(slotX + slotW - 28, slotY + 4, 24, 16);
                        iSetColor(0, 200, 240);
                        iRectangle(slotX + slotW - 28, slotY + 4, 24, 16);

                        iSetColor(255, 255, 255);
                        iText(slotX + slotW - 24, slotY + 8, countStr, GLUT_BITMAP_HELVETICA_10);
                    }
                }
            }
        }

        // 3. Render Hover Tooltip Overlay
        if (hoveredSlotIdx >= 0 && hoveredSlotIdx < 12 && inventory[hoveredSlotIdx].isOccupied) {
            const InventoryItem& item = inventory[hoveredSlotIdx];

            int boxW = 220;
            int boxH = 56;
            int boxX = mouseX + 15;
            int boxY = mouseY - 45;

            // Clamp tooltip within screen boundaries
            if (boxX + boxW > 1260) boxX = mouseX - boxW - 15;
            if (boxY < 15) boxY = 15;
            if (boxY + boxH > 705) boxY = 705 - boxH;

            // Background Frame
            iSetColor(8, 12, 22);
            iFilledRectangle(boxX, boxY, boxW, boxH);
            iSetColor(0, 220, 255);
            iRectangle(boxX, boxY, boxW, boxH);
            iSetColor(0, 100, 140);
            iRectangle(boxX + 2, boxY + 2, boxW - 4, boxH - 4);

            // Item Name Header (e.g., "MEDKIT")
            iSetColor(255, 220, 0);
            iText(boxX + 12, boxY + boxH - 22, (char*)item.name.c_str(), GLUT_BITMAP_HELVETICA_12);

            // Item Description (e.g., "Restores 40 HP")
            iSetColor(200, 225, 245);
            iText(boxX + 12, boxY + 12, (char*)item.description.c_str(), GLUT_BITMAP_HELVETICA_10);
        }

        // Bottom instruction label
        iSetColor(200, 210, 220);
        iText(panelX + 180, panelY + 30, "Press [TAB] or [ESC] to Close", GLUT_BITMAP_HELVETICA_12);
    }
}

void GameManager::RenderDialogue() {
    RenderPlaying(); // Render gameplay backdrop

    static double s_dialogueTimer = 0.0;
    static char s_lastDialogueText[512] = "";

    if (strcmp(g_dialogueText, s_lastDialogueText) != 0) {
        strcpy_s(s_lastDialogueText, sizeof(s_lastDialogueText), g_dialogueText);
        s_dialogueTimer = 0.0;
    }
    s_dialogueTimer += 0.016; // Approx delta time for 60FPS

    bool isCharacter = (strcmp(g_dialogueSpeaker, "Arin") == 0 || strcmp(g_dialogueSpeaker, "Luna") == 0 || strcmp(g_dialogueSpeaker, "Dr. Kael") == 0);

    // Fade in animation
    double fadeAlpha = s_dialogueTimer * 2.0;
    if (fadeAlpha > 1.0) fadeAlpha = 1.0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (isCharacter) {
        // --- DIALOGUE UI (Cinematic Text Box) ---
        int panelX = 90;
        int panelW = 1100;
        int panelH = 150;
        int panelY = 40;

        // Dark gradient cinematic background
        glColor4f(0.02f, 0.02f, 0.02f, 0.85f * fadeAlpha);
        iFilledRectangle(panelX, panelY, panelW, panelH);

        // Subtle bottom/top cinematic lines
        glColor4f(0.15f, 0.15f, 0.15f, fadeAlpha);
        iLine(panelX, panelY, panelX + panelW, panelY);
        iLine(panelX, panelY + panelH, panelX + panelW, panelY + panelH);

        // Character Name Box
        int nameW = 200;
        int nameH = 35;
        glColor4f(0.08f, 0.08f, 0.08f, 0.9f * fadeAlpha);
        iFilledRectangle(panelX + 20, panelY + panelH, nameW, nameH);
        glColor4f(0.3f, 0.3f, 0.3f, fadeAlpha);
        iLine(panelX + 20, panelY + panelH + nameH, panelX + 20 + nameW, panelY + panelH + nameH);
        iLine(panelX + 20 + nameW, panelY + panelH, panelX + 20 + nameW, panelY + panelH + nameH);

        UI::DrawAlphaShadowText(panelX + 35, panelY + panelH + 10, g_dialogueSpeaker, GLUT_BITMAP_HELVETICA_18, 220, 220, 220, fadeAlpha, 1);

        // Body Text Multi-line with Typing Effect
        int textStartX = panelX + 40;
        int textStartY = panelY + panelH - 40;
        int maxPixelWidth = panelW - 80;
        int lineHeight = 28;

        std::string textStr(g_dialogueText);
        
        // Calculate typing length
        int charactersToReveal = (int)(s_dialogueTimer * 40.0); // 40 chars per second
        if (charactersToReveal > (int)textStr.length()) charactersToReveal = (int)textStr.length();
        
        std::string typedText = textStr.substr(0, charactersToReveal);
        std::vector<std::string> lines;
        
        // Max characters per line for GLUT_BITMAP_HELVETICA_18 (~9.5px per char)
        int maxCharsPerLine = (int)(maxPixelWidth / 9.5); 
        if (maxCharsPerLine < 20) maxCharsPerLine = 20;

        std::stringstream ss(typedText);
        std::string segment;

        while (std::getline(ss, segment, '\n')) {
            if (segment.empty()) {
                lines.push_back("");
                continue;
            }
            std::stringstream wordStream(segment);
            std::string word;
            std::string currentLine = "";
            while (wordStream >> word) {
                if (currentLine.empty()) {
                    currentLine = word;
                } else if ((int)(currentLine.length() + 1 + word.length()) <= maxCharsPerLine) {
                    currentLine += " " + word;
                } else {
                    lines.push_back(currentLine);
                    currentLine = word;
                }
            }
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
        }

        int currentY = textStartY;
        for (size_t i = 0; i < lines.size(); ++i) {
            UI::DrawAlphaShadowText(textStartX, currentY, lines[i].c_str(), GLUT_BITMAP_HELVETICA_18, 245, 245, 245, fadeAlpha, 1);
            currentY -= lineHeight;
        }

        // Footer prompt
        double promptBlink = (sin(s_dialogueTimer * 5.0) + 1.0) / 2.0;
        UI::DrawAlphaText(panelX + panelW - 200, panelY + 15, "Press [ENTER] to Continue", GLUT_BITMAP_HELVETICA_12, 120, 120, 120, fadeAlpha * (0.5 + promptBlink * 0.5));
    } else {
        // --- NOTE READING UI (Document/Item) ---
        int panelW = 700;
        int panelH = 500;
        int panelX = (1280 - panelW) / 2;
        int panelY = (720 - panelH) / 2;

        // Dark worn metal frame background
        glColor4f(0.08f, 0.08f, 0.09f, 0.92f * fadeAlpha);
        iFilledRectangle(panelX, panelY, panelW, panelH);

        // Damaged edges & inner paper feel
        glColor4f(0.12f, 0.12f, 0.13f, 0.85f * fadeAlpha);
        iFilledRectangle(panelX + 15, panelY + 15, panelW - 30, panelH - 30);
        
        // Subtle rust border
        glColor4f(0.35f, 0.15f, 0.1f, 0.7f * fadeAlpha);
        iRectangle(panelX, panelY, panelW, panelH);
        iRectangle(panelX + 2, panelY + 2, panelW - 4, panelH - 4);
        
        glColor4f(0.4f, 0.4f, 0.4f, 0.3f * fadeAlpha);
        iRectangle(panelX + 15, panelY + 15, panelW - 30, panelH - 30);

        // Title Header
        // Muted orange/red highlights for old emergency interface style
        glColor4f(0.7f, 0.25f, 0.15f, fadeAlpha);
        iFilledRectangle(panelX + 15, panelY + panelH - 70, panelW - 30, 55);

        UI::DrawAlphaShadowText(panelX + 35, panelY + panelH - 52, g_dialogueSpeaker, GLUT_BITMAP_TIMES_ROMAN_24, 250, 240, 230, fadeAlpha, 2);

        // Document Body
        int textStartX = panelX + 45;
        int textStartY = panelY + panelH - 110;
        int maxPixelWidth = panelW - 90;
        int lineHeight = 30;

        std::string textStr(g_dialogueText);
        std::vector<std::string> lines;
        int maxCharsPerLine = (int)(maxPixelWidth / 9.5);
        if (maxCharsPerLine < 20) maxCharsPerLine = 20;

        std::stringstream ss(textStr);
        std::string segment;

        while (std::getline(ss, segment, '\n')) {
            if (segment.empty()) {
                lines.push_back("");
                continue;
            }
            std::stringstream wordStream(segment);
            std::string word;
            std::string currentLine = "";
            while (wordStream >> word) {
                if (currentLine.empty()) {
                    currentLine = word;
                } else if ((int)(currentLine.length() + 1 + word.length()) <= maxCharsPerLine) {
                    currentLine += " " + word;
                } else {
                    lines.push_back(currentLine);
                    currentLine = word;
                }
            }
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
        }

        // Render clean readable text in faded grey/off-white
        int currentY = textStartY;
        for (size_t i = 0; i < lines.size(); ++i) {
            UI::DrawAlphaText(textStartX, currentY, lines[i].c_str(), GLUT_BITMAP_HELVETICA_18, 190, 190, 190, fadeAlpha);
            currentY -= lineHeight;
        }

        // Footer prompt
        double promptBlink = (sin(s_dialogueTimer * 5.0) + 1.0) / 2.0;
        UI::DrawAlphaText(panelX + panelW - 220, panelY + 25, "Press [ENTER] to Close", GLUT_BITMAP_HELVETICA_12, 140, 60, 40, fadeAlpha * (0.6 + promptBlink * 0.4));
    }
}

void GameManager::RenderGameOver() {
    // 1. Display ui_game_over_background.png / game_over_screen.png image asset
    if (g_texGameOverBg != 0) {
        iShowImage(0, 0, 1280, 720, g_texGameOverBg);
    } else {
        iSetColor(15, 5, 5);
        iFilledRectangle(0, 0, 1280, 720);
    }

    // 2. High contrast title header in carved banner slot
    const char* goTitle = "YOU HAVE DIED";
    DrawOutlinedText(530, 525, goTitle, GLUT_BITMAP_TIMES_ROMAN_24, 240, 30, 30);

    // 3. Option 1: Restart Game (Slot 1)
    RenderMenuButtonSlot(1, 515, 440, "1. RESTART GAME [ENTER]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 4. Option 2: Exit to Menu (Slot 2)
    RenderMenuButtonSlot(2, 530, 362, "2. EXIT TO MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 5. Option 3: Exit Game (Slot 3)
    RenderMenuButtonSlot(3, 535, 285, "3. EXIT GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 6. Bottom Detail Slot: Instructions
    const char* goFooter = "Press [ENTER], [M], [ESC] or Click Options to Select";
    DrawShadowText(470, 148, goFooter, GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
}

void GameManager::RenderVictory() {
    // 1. Display ui_level_complete_background.png / level_complete_screen.png image asset
    if (g_texLevelCompleteBg != 0) {
        iShowImage(0, 0, 1280, 720, g_texLevelCompleteBg);
    } else {
        iSetColor(5, 15, 10);
        iFilledRectangle(0, 0, 1280, 720);
    }

    // 2. Title Header in Top Carved Banner Slot
    const char* vicTitle = (currentLevel == 2) ? "LEVEL 2 COMPLETE" : "LEVEL 1 COMPLETE";
    DrawOutlinedText(515, 545, vicTitle, GLUT_BITMAP_TIMES_ROMAN_24, 0, 255, 120);

    const char* chapterSub = (currentLevel == 2) ? "BLACKWOOD FOREST" : "THE FALLEN VILLAGE";
    DrawShadowText(525, 505, chapterSub, GLUT_BITMAP_HELVETICA_18, 200, 200, 200);

    const char* updatedMission = (currentLevel == 2) ? "Mission Updated: REACH NOVAGEN FACILITY B" : "Mission Updated: REACH BLACKWOOD FOREST";
    DrawOutlinedText(485, 475, updatedMission, GLUT_BITMAP_HELVETICA_12, 0, 230, 255);

    // 3. Option 1: Next Level / Main Menu
    if (currentLevel == 1) {
        RenderMenuButtonSlot(1, 485, 412, "1. NEXT LEVEL [ENTER]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);
    } else {
        RenderMenuButtonSlot(1, 515, 412, "1. MAIN MENU [ENTER]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);
    }

    // 4. Option 2: Restart Level
    RenderMenuButtonSlot(2, 530, 357, "2. RESTART LEVEL [R]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 5. Option 3: Exit Game
    RenderMenuButtonSlot(3, 535, 302, "3. EXIT GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 6. Bottom Detail Slot: Instructions
    const char* vicFooter = "Press [ENTER], [R], [ESC] or Click Options to Select";
    DrawShadowText(470, 148, vicFooter, GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
}

void GameManager::RenderCursor() {
    int cx = mouseX;
    int cy = mouseY;

    // Drop shadow under cursor
    iSetColor(0, 0, 0);
    iFilledCircle(cx + 1, cy - 1, 4);

    // Crosshair diamond cursor frame
    if (isMouseDown) {
        iSetColor(255, 220, 0); // Gold click pulse
        iFilledCircle(cx, cy, 5);
        iSetColor(255, 255, 255);
        iRectangle(cx - 7, cy - 7, 15, 15);
    } else {
        iSetColor(0, 240, 255); // Cyan active crosshair
        iFilledCircle(cx, cy, 3);
        iLine(cx - 10, cy, cx - 4, cy);
        iLine(cx + 4, cy, cx + 10, cy);
        iLine(cx, cy - 10, cx, cy - 4);
        iLine(cx, cy + 4, cx, cy + 10);
        iRectangle(cx - 6, cy - 6, 13, 13);
    }
}

void GameManager::RenderLeaderboard() {
    iSetColor(10, 10, 15);
    iFilledRectangle(0, 0, 1280, 720);

    DrawOutlinedText(440, 650, "GENESIS ARCHIVE - LEADERBOARD", GLUT_BITMAP_TIMES_ROMAN_24, 0, 230, 255);

    const std::vector<ScoreEntry>& entries = leaderboard.GetEntries();

    if (entries.empty()) {
        DrawShadowText(540, 400, "No records found.", GLUT_BITMAP_HELVETICA_18, 255, 255, 255);
    }
    else {
        for (size_t i = 0; i < entries.size() && i < 5; ++i) {
            char row[128];
            sprintf_s(row, sizeof(row), "%d.  %-20s   Score: %07d", (int)(i + 1), entries[i].name, entries[i].score);
            iText(350, 500 - (i * 60), row, GLUT_BITMAP_HELVETICA_18);
        }
    }

    iSetColor(0, 230, 120);
    iText(400, 100, "Press ESC to Return to Menu", GLUT_BITMAP_HELVETICA_18);
}

// ============================================================================
// Input Event Handlers
// ============================================================================
void GameManager::HandleKeyPress(unsigned char key) {
    if (currentState == STATE_MENU) {
        if (key == 13 || key == '1') { // Enter or 1 = Start Survival
            Initialize(); // Fresh start
            currentState = STATE_PLAYING;
            menuTransitionAlpha = 1.0;
        }
        else if (key == '2') {
            currentState = STATE_LEADERBOARD;
            menuTransitionAlpha = 1.0;
        }
        else if (key == '3') {
            exit(0);
        }
    }
    else if (currentState == STATE_PLAYING) {
        if (key == 27) { // ESC Key
            if (showInventory) {
                showInventory = false;
            } else {
                currentState = STATE_PAUSED;
                pauseSubMenu = 0;
                menuTransitionAlpha = 1.0;
            }
        }
        else if (key == 9 || key == '\t' || key == 'i' || key == 'I') {
            showInventory = !showInventory;
        }
        else if (key == 'e' || key == 'E') {
            if (!showInventory) {
                bool itemInteracted = false;
                for (size_t i = 0; i < collectibles.size(); ++i) {
                    if (collectibles[i].active && std::abs(player.x - collectibles[i].x) < 70.0) {
                        collectibles[i].active = false;
                        score += 100;
                        itemInteracted = true;

                        switch (collectibles[i].type) {
                        case COL_KEYCARD:
                            hasKeycard = true;
                            AddInventoryItem("keycard", 1);
                            currentState = STATE_DIALOGUE;
                            sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                            sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"A NovaGen command keycard! This will grant me access to open the steel gate checkpoint.\"");
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "NOVAGEN KEYCARD ACQUIRED");
                            g_pickupR = 255; g_pickupG = 215; g_pickupB = 0;
                            g_pickupTimer = 2.5;
                            g_pickupX = collectibles[i].x;
                            g_pickupY = collectibles[i].y + 40.0;
                            break;
                        case COL_NOTE:
                            AddInventoryItem("mission_note", 1);
                            currentState = STATE_DIALOGUE;
                            sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Survivor's Clue Note");
                            if (collectibles[i].x < 10000) {
                                sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"NovaGen Memo:\nEvacuation path compromised.\nConvoy heading East to Blackwood Forest.\nSubject Luna immune.\"");
                            } else {
                                sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"A crumpled note:\nDr. Kael took the silver-haired girl through the forest checkpoint.\nShe is our only hope...\"");
                            }
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "MISSION NOTE DISCOVERED");
                            g_pickupR = 0; g_pickupG = 230; g_pickupB = 255;
                            g_pickupTimer = 2.5;
                            g_pickupX = collectibles[i].x;
                            g_pickupY = collectibles[i].y + 40.0;
                            break;
                        case COL_MEDKIT:
                            player.medkits++;
                            AddInventoryItem("medkit", 1);
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 FIRST AID MEDKIT");
                            g_pickupR = 255; g_pickupG = 100; g_pickupB = 100;
                            g_pickupTimer = 2.0; g_pickupX = collectibles[i].x; g_pickupY = collectibles[i].y + 40.0;
                            break;
                        case COL_BATTERY:
                            player.batteryCount++;
                            AddInventoryItem("battery", 1);
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 BATTERY");
                            g_pickupR = 0; g_pickupG = 255; g_pickupB = 200;
                            g_pickupTimer = 2.0; g_pickupX = collectibles[i].x; g_pickupY = collectibles[i].y + 40.0;
                            break;
                        case COL_FOOD:
                            player.foodCount++;
                            AddInventoryItem("food_can", 1);
                            player.hp = (player.hp + 15 > player.maxHp) ? player.maxHp : player.hp + 15;
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 RATION (+15 HP)");
                            g_pickupR = 255; g_pickupG = 180; g_pickupB = 0;
                            g_pickupTimer = 2.0; g_pickupX = collectibles[i].x; g_pickupY = collectibles[i].y + 40.0;
                            break;
                        case COL_WATER:
                            player.waterBottleCount++;
                            AddInventoryItem("water_bottle", 1);
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 WATER BOTTLE");
                            g_pickupR = 0; g_pickupG = 220; g_pickupB = 255;
                            g_pickupTimer = 2.0; g_pickupX = collectibles[i].x; g_pickupY = collectibles[i].y + 40.0;
                            break;
                        case COL_AMMO:
                            player.ammo += 15;
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "+15 PISTOL AMMO");
                            g_pickupR = 255; g_pickupG = 215; g_pickupB = 0;
                            g_pickupTimer = 2.0; g_pickupX = collectibles[i].x; g_pickupY = collectibles[i].y + 40.0;
                            break;
                        default:
                            player.scrapCount++;
                            AddInventoryItem("scrap_metal", 1);
                            sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 ITEM ACQUIRED");
                            g_pickupR = 200; g_pickupG = 210; g_pickupB = 220;
                            g_pickupTimer = 2.0; g_pickupX = collectibles[i].x; g_pickupY = collectibles[i].y + 40.0;
                            break;
                        }
                        break;
                    }
                }

                // If no item was near, check Exit Gate interaction
                if (!itemInteracted && player.x >= 12900 && bossDefeated) {
                    if (!hasKeycard) {
                        currentState = STATE_DIALOGUE;
                        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"The steel gate is locked. I need a NovaGen keycard from the quarantine checkpoint.\"");
                    }
                    else if (!ribbonCollected) {
                        currentState = STATE_DIALOGUE;
                        sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                        sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Luna's silver-blue ribbon! It's caught on the steel gate latch... She survived. I will find you, Luna!\"");
                        ribbonCollected = true;
                    }
                    else {
                        currentState = STATE_VICTORY;
                        menuTransitionAlpha = 1.0;
                        leaderboard.AddScore("Arin", score);
                    }
                }
            }
        }
        else if (key == 'j' || key == 'J') {
            if (!showInventory) player.AttackMelee();
        }
        else if (key == 'k' || key == 'K') {
            if (!showInventory) player.AttackRanged();
        }
        else if (key == 'h' || key == 'H') {
            if (!showInventory) player.UseHeal();
        }
        else if (key == 'f' || key == 'F') {
            if (!showInventory) player.UseFood();
        }
        else if (key == 'b' || key == 'B') {
            if (!showInventory) {
                for (int s = 0; s < 12; ++s) {
                    if (inventory[s].isOccupied && inventory[s].id == "water_bottle") {
                        UseInventorySlot(s);
                        break;
                    }
                }
            }
        }
    }
    else if (currentState == STATE_PAUSED) {
        if (key == 27) { // ESC resumes or closes sub menu
            if (pauseSubMenu > 0) {
                pauseSubMenu = 0;
            } else {
                currentState = STATE_PLAYING;
                menuTransitionAlpha = 1.0;
                player.ResetInputState();
            }
        }
        else if (key == '1') {
            currentState = STATE_PLAYING;
            pauseSubMenu = 0;
            menuTransitionAlpha = 1.0;
            player.ResetInputState();
        }
        else if (key == '2' || key == 9 || key == '\t' || key == 'i' || key == 'I') {
            currentState = STATE_PLAYING;
            showInventory = true;
            pauseSubMenu = 0;
            player.ResetInputState();
        }
        else if (key == '3' || key == 'c' || key == 'C') {
            pauseSubMenu = 1; // Controls panel
        }
        else if (key == '4' || key == 's' || key == 'S') {
            pauseSubMenu = 2; // Settings panel
        }
        else if (key == '5') {
            Initialize();
            currentState = STATE_PLAYING;
            pauseSubMenu = 0;
            player.ResetInputState();
        }
        else if (key == '6' || key == 'm' || key == 'M') { // Quit to menu
            currentState = STATE_MENU;
            pauseSubMenu = 0;
            menuTransitionAlpha = 1.0;
        }
    }
    else if (currentState == STATE_DIALOGUE) {
        if (key == 13 || key == 'e' || key == 'E' || key == 32 || key == 27) { // Enter, E, Space, or ESC key
            if (ribbonCollected || (player.x >= 12800 && hasKeycard && bossDefeated)) {
                currentState = STATE_VICTORY;
                menuTransitionAlpha = 1.0;
                leaderboard.AddScore("Arin", score);
            } else {
                currentState = STATE_PLAYING;
                menuTransitionAlpha = 1.0;
                player.ResetInputState();
            }
        }
    }
    else if (currentState == STATE_GAMEOVER) {
        if (key == 13 || key == '1') { // Enter or 1 = Restart Game
            Initialize();
            currentState = STATE_PLAYING;
            menuTransitionAlpha = 1.0;
        }
        else if (key == 'm' || key == 'M' || key == '2') { // M or 2 = Exit to Menu
            currentState = STATE_MENU;
            menuTransitionAlpha = 1.0;
        }
        else if (key == 27 || key == '3') { // ESC or 3 = Exit Game
            exit(0);
        }
    }
    else if (currentState == STATE_VICTORY) {
        if (key == 13 || key == '1') { // Enter or 1 = Next Level or Main Menu
            if (currentLevel == 1) {
                LoadLevel2();
                currentState = STATE_PLAYING;
                menuTransitionAlpha = 1.0;
            } else {
                currentState = STATE_MENU;
                menuTransitionAlpha = 1.0;
            }
        }
        else if (key == 'm' || key == 'M') {
            currentState = STATE_MENU;
            menuTransitionAlpha = 1.0;
        }
        else if (key == 'r' || key == 'R' || key == '2') { // R or 2 = Restart Level
            if (currentLevel == 2) {
                LoadLevel2();
            } else {
                LoadLevel1();
            }
            currentState = STATE_PLAYING;
            menuTransitionAlpha = 1.0;
        }
        else if (key == 27 || key == '3') { // ESC or 3 = Exit Game
            exit(0);
        }
    }
    else if (currentState == STATE_LEADERBOARD) {
        if (key == 27) {
            currentState = STATE_MENU;
            menuTransitionAlpha = 1.0;
        }
    }
}

void GameManager::HandleSpecialKeyPress(unsigned char key) {
    // Handled dynamically in Player state updates
}

void GameManager::HandleMouseMove(int mx, int my) {
    mouseX = mx;
    mouseY = my;
}

void GameManager::HandleMouseClick(int button, int state, int mx, int my) {
    mouseX = mx;
    mouseY = my;

    if (button == 0) { // GLUT_LEFT_BUTTON == 0
        if (state == 0) { // GLUT_DOWN == 0
            isMouseDown = true;
        }
        else if (state == 1) { // GLUT_UP == 1
            isMouseDown = false;

            if (currentState == STATE_MENU) {
                // Slot 1: Start Survival (Level 1)
                if (mx >= 440 && mx <= 840 && my >= 420 && my <= 470) {
                    Initialize();
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 2: Leaderboard
                else if (mx >= 440 && mx <= 840 && my >= 345 && my <= 390) {
                    currentState = STATE_LEADERBOARD;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 3: Exit
                else if (mx >= 440 && mx <= 840 && my >= 265 && my <= 310) {
                    exit(0);
                }
            }
            else if (currentState == STATE_PAUSED) {
                if (pauseSubMenu > 0) {
                    pauseSubMenu = 0;
                } else {
                    // Slot 1: Resume Game (430 - 475)
                    if (mx >= 440 && mx <= 840 && my >= 430 && my <= 475) {
                        currentState = STATE_PLAYING;
                        menuTransitionAlpha = 1.0;
                    }
                    // Slot 2: Inventory (375 - 420)
                    else if (mx >= 440 && mx <= 840 && my >= 375 && my <= 420) {
                        currentState = STATE_PLAYING;
                        showInventory = true;
                    }
                    // Slot 3: Controls (320 - 365)
                    else if (mx >= 440 && mx <= 840 && my >= 320 && my <= 365) {
                        pauseSubMenu = 1;
                    }
                    // Slot 4: Settings (265 - 310)
                    else if (mx >= 440 && mx <= 840 && my >= 265 && my <= 310) {
                        pauseSubMenu = 2;
                    }
                    // Slot 5: Restart Level (210 - 255)
                    else if (mx >= 440 && mx <= 840 && my >= 210 && my <= 255) {
                        if (currentLevel == 2) {
                            LoadLevel2();
                        } else {
                            LoadLevel1();
                        }
                        currentState = STATE_PLAYING;
                        menuTransitionAlpha = 1.0;
                    }
                    // Slot 6: Quit to Menu (155 - 200)
                    else if (mx >= 440 && mx <= 840 && my >= 155 && my <= 200) {
                        currentState = STATE_MENU;
                        menuTransitionAlpha = 1.0;
                    }
                }
            }
            else if (currentState == STATE_DIALOGUE) {
                if (ribbonCollected || (player.x >= 12800 && hasKeycard && bossDefeated)) {
                    currentState = STATE_VICTORY;
                    menuTransitionAlpha = 1.0;
                    leaderboard.AddScore("Arin", score);
                } else {
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
            }
            else if (currentState == STATE_GAMEOVER) {
                // Slot 1: Restart Game
                if (mx >= 440 && mx <= 840 && my >= 420 && my <= 470) {
                    if (currentLevel == 2) {
                        LoadLevel2();
                    } else {
                        LoadLevel1();
                    }
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 2: Exit to Menu
                else if (mx >= 440 && mx <= 840 && my >= 345 && my <= 390) {
                    currentState = STATE_MENU;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 3: Exit Game
                else if (mx >= 440 && mx <= 840 && my >= 265 && my <= 310) {
                    exit(0);
                }
            }
            else if (currentState == STATE_VICTORY) {
                // Slot 1: Next Level (Level 1) or Main Menu (Level 2)
                if (mx >= 440 && mx <= 840 && my >= 400 && my <= 450) {
                    if (currentLevel == 1) {
                        LoadLevel2();
                        currentState = STATE_PLAYING;
                        menuTransitionAlpha = 1.0;
                    } else {
                        currentState = STATE_MENU;
                        menuTransitionAlpha = 1.0;
                    }
                }
                // Slot 2: Restart Level
                else if (mx >= 440 && mx <= 840 && my >= 345 && my <= 390) {
                    if (currentLevel == 2) {
                        LoadLevel2();
                    } else {
                        LoadLevel1();
                    }
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 3: Exit Game
                else if (mx >= 440 && mx <= 840 && my >= 265 && my <= 310) {
                    exit(0);
                }
            }
            else if (currentState == STATE_PLAYING) {
                if (showInventory) {
                    int cols = 4, rows = 3;
                    int slotW = 85, slotH = 85;
                    int panelX = 340, panelY = 140, panelH = 440;
                    int startX = panelX + 70;
                    int startY = panelY + panelH - 160;
                    int gapX = 35, gapY = 20;

                    for (int r = 0; r < rows; ++r) {
                        for (int c = 0; c < cols; ++c) {
                            int slotIdx = r * cols + c;
                            int slotX = startX + c * (slotW + gapX);
                            int slotY = startY - r * (slotH + gapY);

                            if (mx >= slotX && mx <= slotX + slotW && my >= slotY && my <= slotY + slotH) {
                                UseInventorySlot(slotIdx);
                                return;
                            }
                        }
                    }
                } else {
                    // Check if player clicked HUD quick bar slots (bottom-left)
                    int hudX = 20, hudY = 15, hudW = 370, hudH = 58;
                    if (mx >= hudX && mx <= hudX + hudW && my >= hudY && my <= hudY + hudH) {
                        int slotW = 84;
                        int colX[4] = { 28, 118, 208, 298 };

                        if (mx >= colX[0] && mx <= colX[0] + slotW) {
                            player.UseHeal();
                            return;
                        } else if (mx >= colX[1] && mx <= colX[1] + slotW) {
                            for (int s = 0; s < 12; ++s) {
                                if (inventory[s].isOccupied && (inventory[s].id == "food_can" || inventory[s].id == "bread" || inventory[s].id == "apple")) {
                                    UseInventorySlot(s);
                                    return;
                                }
                            }
                            player.UseFood();
                            return;
                        } else if (mx >= colX[2] && mx <= colX[2] + slotW) {
                            for (int s = 0; s < 12; ++s) {
                                if (inventory[s].isOccupied && inventory[s].id == "water_bottle") {
                                    UseInventorySlot(s);
                                    return;
                                }
                            }
                            player.UseWaterBottle();
                            return;
                        }
                    } else {
                        player.AttackMelee();
                    }
                }
            }
        }
    }
}

void GameManager::AddScore(int amount) {
    score += amount;
}

int GameManager::GetAreaFromPosition(double px) const {
    if (currentLevel == 2) {
        if (px < 1448.0)  return L2_AREA_FOREST_ENTRANCE;
        if (px < 2896.0)  return L2_AREA_ABANDONED_ROAD;
        if (px < 4344.0)  return L2_AREA_EVACUATION_CAMP;
        if (px < 5792.0)  return L2_AREA_DEEP_FOREST;
        if (px < 7240.0)  return L2_AREA_RIVER_CROSSING;
        if (px < 8688.0)  return L2_AREA_SURVIVOR_HIDEOUT;
        if (px < 10136.0) return L2_AREA_INFECTED_FOREST;
        if (px < 11584.0) return L2_AREA_NOVAGEN_OUTPOST;
        if (px < 13032.0) return L2_AREA_RESEARCH_FACILITY;
        if (px < 14480.0) return L2_AREA_BOSS_ARENA;
        return L2_AREA_LEVEL_COMPLETE;
    }
    if (px < 1448.0) return AREA_SPAWN_AREA;      // 1. SPAWN AREA / DESTROYED HOUSE (0 enemies)
    if (px < 2896.0) return AREA_VILLAGE_STREET;   // 2. VILLAGE STREET (3 Walkers)
    if (px < 4344.0) return AREA_VILLAGE_SQUARE;   // 3. VILLAGE SQUARE (4 Walkers, 1 Runner)
    if (px < 5792.0) return AREA_ABANDONED_MARKET; // 4. ABANDONED MARKET (2 Walkers, 1 Raider)
    if (px < 7240.0) return AREA_RAIDER_CAMP;      // 5. RAIDER CAMP (3 Raiders)
    if (px < 8688.0) return AREA_ABANDONED_CHURCH;// 6. ABANDONED CHURCH (2 Walkers, 1 Runner)
    if (px < 10136.0) return AREA_QUARANTINE_ZONE; // 7. QUARANTINE ZONE (2 Walkers, 1 Heavy Infected)
    if (px < 11584.0) return AREA_BROKEN_BRIDGE;   // 8. BROKEN BRIDGE (0 enemies)
    if (px < 13032.0) return AREA_MINI_BOSS_ARENA; // 9. MINI BOSS ARENA (1 Mutated Brute)
    if (px < 14480.0) return AREA_EXIT_GATE;       // 10. EXIT GATE (0 enemies)
    return AREA_LEVEL_COMPLETE;                    // 11. LEVEL COMPLETE
}