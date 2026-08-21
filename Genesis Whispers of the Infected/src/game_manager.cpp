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
};

struct FogParticle {
    double x, y, speed, alpha;
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
static unsigned int g_texItemCoin = 0;

// Instant Floating Item Pickup Notification Data
static char g_pickupText[64] = "";
static double g_pickupX = 0.0;
static double g_pickupY = 0.0;
static double g_pickupTimer = 0.0;
static int g_pickupR = 255, g_pickupG = 255, g_pickupB = 255;

// ============================================================================
// Constructor & Level Initialization
// ============================================================================
const char* GameManager::GetAreaName(Level1Area area) const {
    switch (area) {
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
    default:                     return "Unknown Area";
    }
}

GameManager::GameManager() {
    currentState = STATE_MENU;
    score = 0;
    currentLevel = 1;
    texPropsSheet = 0;

    currentArea = AREA_SPAWN_AREA;
    previousArea = AREA_SPAWN_AREA;
    areaBannerTimer = 4.0;
    areaBannerAlpha = 1.0;

    bossSpawned = false;
    bossDefeated = false;
    bossHp = 300;
    bossMaxHp = 300;
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
}

void GameManager::Initialize() {
    score = 0;
    currentLevel = 1;
    player.Initialize(200, 185); // Arin starting location inside destroyed house (x=200, groundY=185)
    gameMap.LoadLevel(currentLevel);
    leaderboard.LoadScores();

    currentArea = AREA_SPAWN_AREA;
    previousArea = AREA_SPAWN_AREA;
    areaBannerTimer = 4.0;
    areaBannerAlpha = 1.0;

    bossSpawned = false;
    bossDefeated = false;
    bossHp = 300;
    bossMaxHp = 300;
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

    // Initialize Independent Environment Prop System & Destroyed House Area Props (Arin's Family Home)
    worldProps.clear();

    // --- AREA 1 & 2: DESTROYED HOUSE (ARIN'S FAMILY HOME: x = 0 to 3500) ---
    // --- AREA 1 & 2: DESTROYED HOUSE (ARIN'S FAMILY HOME: x = 0 to 3500) ---
    // --- AREA 1 & 2: DESTROYED HOUSE (ARIN'S FAMILY HOME: x = 0 to 3500) ---
    // 1. Entrance / Living Room (x = 300 to 1000)
    AddWorldProp("Assets/Props/Furniture/furn_broken_chair_01.png", 420.0, 185.0, 56.0, 56.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_missing_luna.png", 650.0, 270.0, 48.0, 64.0, PROP_LAYER_BACKGROUND); // Story: Family photo / Missing Luna portrait
    AddWorldProp("Assets/Props/Furniture/furn_dining_table_01.png", 750.0, 185.0, 110.0, 70.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Items/Medicine/first_aid.png", 840.0, 185.0, 36.0, 36.0, PROP_LAYER_BACKGROUND); // Story: Personal belongings / medical supplies

    // 2. Kitchen / Storage Area (x = 1000 to 1700)
    AddWorldProp("Assets/Props/Furniture/furn_wooden_cabinet_01.png", 1120.0, 185.0, 85.0, 115.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_emergency_evacuation.png", 1320.0, 280.0, 48.0, 64.0, PROP_LAYER_BACKGROUND); // Story: Evacuation notice
    AddWorldProp("Assets/Props/Decorations/veh_shopping_cart_destroyed.png", 1520.0, 185.0, 75.0, 60.0, PROP_LAYER_BACKGROUND);

    // 3. Arin & Luna's Bedrooms (x = 1700 to 2400)
    AddWorldProp("Assets/Props/Furniture/furn_broken_bed_01.png", 1820.0, 185.0, 130.0, 75.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_missing_luna.png", 1950.0, 260.0, 40.0, 52.0, PROP_LAYER_BACKGROUND); // Story: Children's sketch / Luna's picture in bedroom
    AddWorldProp("Assets/Props/Furniture/furn_broken_bench_01.png", 2150.0, 185.0, 80.0, 45.0, PROP_LAYER_BACKGROUND);

    // 4. Second Floor Upper Platforms (x = 2400 to 3000)
    AddWorldProp("Assets/Props/Decorations/prop_wooden_crate_01.png", 2520.0, 300.0, 48.0, 48.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Furniture/furn_broken_chair_01.png", 2720.0, 450.0, 48.0, 48.0, PROP_LAYER_BACKGROUND);

    // 5. Exit Ledge / Destroyed Wall Boundary (x = 3000 to 3400)
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 3120.0, 185.0, 100.0, 65.0, PROP_LAYER_BACKGROUND);

    // --- AREA 3 & 4: VILLAGE STREET & SQUARE (x = 3500 to 7500) ---
    // 1. Entrance to Village Street (x = 3500 to 4400)
    AddWorldProp("Assets/Props/Decorations/prop_telephone_pole_01.png", 3650.0, 185.0, 60.0, 240.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_emergency_evacuation.png", 3655.0, 270.0, 44.0, 60.0, PROP_LAYER_BACKGROUND); // Story: Evacuation poster on pole
    AddWorldProp("Assets/Props/Nature/nature_dead_tree_01.png", 3900.0, 185.0, 120.0, 180.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Vehicles/veh_pickup_destroyed.png", 4200.0, 185.0, 160.0, 90.0, PROP_LAYER_BACKGROUND);

    // 2. Mid Street & Barricade Zone (x = 4400 to 5500)
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 4550.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 4555.0, 250.0, 44.0, 60.0, PROP_LAYER_BACKGROUND); // Story: Military warning sign on lamp
    AddWorldProp("Assets/Props/Decorations/prop_burning_barrel_01.png", 4750.0, 185.0, 48.0, 60.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/veh_shopping_cart_destroyed.png", 4900.0, 185.0, 75.0, 60.0, PROP_LAYER_BACKGROUND); // Story: Abandoned luggage / backpack cart
    AddWorldProp("Assets/Props/Vehicles/veh_destroyed_car_01.png", 5100.0, 185.0, 150.0, 80.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Nature/dry_bush.png", 5350.0, 185.0, 48.0, 36.0, PROP_LAYER_FOREGROUND); // Foreground Grass

    // 3. Village Square Approach (x = 5500 to 6700)
    AddWorldProp("Assets/Props/Decorations/prop_telephone_pole_01.png", 5600.0, 185.0, 60.0, 240.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Vehicles/veh_ambulance_burned.png", 5900.0, 185.0, 170.0, 95.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Nature/Assets__stone.png", 6200.0, 185.0, 56.0, 40.0, PROP_LAYER_FOREGROUND); // Foreground Rocks
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 6450.0, 185.0, 90.0, 60.0, PROP_LAYER_BACKGROUND);

    // 4. Village Square Edge (x = 6700 to 7400)
    AddWorldProp("Assets/Props/Nature/nature_dead_tree_01.png", 6850.0, 185.0, 130.0, 190.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 7150.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 7155.0, 250.0, 44.0, 60.0, PROP_LAYER_BACKGROUND); // Story: Military quarantine warning
    AddWorldProp("Assets/Props/Decorations/prop_oil_drum_01.png", 7300.0, 185.0, 44.0, 55.0, PROP_LAYER_BACKGROUND);

    // --- AREA 4: VILLAGE SQUARE & QUARANTINE (x = 7500 to 9000) ---
    // 1. Village Square West Entrance & Sandbag Perimeter (x = 7500 to 7900)
    AddWorldProp("Assets/Props/Decorations/prop_sandbags_01.png", 7600.0, 185.0, 110.0, 50.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 7850.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND);

    // 2. Central Fountain Plaza & Abandoned Ambulance (x = 7900 to 8400)
    AddWorldProp("Assets/Props/Vehicles/veh_ambulance_burned.png", 7950.0, 185.0, 170.0, 95.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Nature/Assets__stone.png", 8150.0, 220.0, 90.0, 55.0, PROP_LAYER_BACKGROUND); // Destroyed Fountain Highlight
    AddWorldProp("Assets/Props/Nature/Assets__stone.png", 8320.0, 185.0, 64.0, 45.0, PROP_LAYER_FOREGROUND); // Foreground Rubble

    // 3. Military Checkpoint Barricade & East Exit (x = 8400 to 9000)
    AddWorldProp("Assets/Props/Military/bld_military_checkpoint.png", 8550.0, 185.0, 130.0, 85.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_novagen_genesis.png", 8580.0, 235.0, 48.0, 64.0, PROP_LAYER_BACKGROUND); // Story: NovaGen propaganda poster on checkpoint
    AddWorldProp("Assets/Items/Medicine/first_aid.png", 8650.0, 185.0, 36.0, 36.0, PROP_LAYER_BACKGROUND); // Story: Abandoned medical equipment
    AddWorldProp("Assets/Props/Decorations/prop_street_lamp_01.png", 8750.0, 185.0, 40.0, 160.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 8800.0, 240.0, 44.0, 60.0, PROP_LAYER_BACKGROUND); // Story: Warning sign on checkpoint gate
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 8900.0, 185.0, 90.0, 60.0, PROP_LAYER_BACKGROUND);

    // --- AREA 5: ABANDONED MARKET (x = 9000 to 11000) ---
    // 1. Market Storefront & Entrance (x = 9000 to 9400)
    AddWorldProp("Assets/Props/Buildings/bld_grocery_store_abandoned.png", 9100.0, 185.0, 160.0, 130.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_novagen_genesis.png", 9150.0, 260.0, 48.0, 64.0, PROP_LAYER_BACKGROUND); // Story: NovaGen poster on store facade
    AddWorldProp("Assets/Props/Decorations/veh_shopping_cart_destroyed.png", 9320.0, 185.0, 75.0, 60.0, PROP_LAYER_BACKGROUND);

    // 2. Inner Market Aisles & Upper Shelf Platforms (x = 9400 to 10200)
    AddWorldProp("Assets/Props/Furniture/furn_grocery_shelf_01.png", 9450.0, 185.0, 90.0, 120.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_wooden_crate_01.png", 9520.0, 280.0, 48.0, 48.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Items/Medicine/first_aid.png", 9600.0, 185.0, 36.0, 36.0, PROP_LAYER_BACKGROUND); // Story: Abandoned medical kit in market aisle
    AddWorldProp("Assets/Props/Furniture/furn_grocery_shelf_01.png", 9820.0, 400.0, 70.0, 90.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/crate.png", 9950.0, 185.0, 56.0, 56.0, PROP_LAYER_BACKGROUND);

    // 3. Market Storage & Rear Exit (x = 10200 to 11000)
    AddWorldProp("Assets/Props/Furniture/furn_grocery_shelf_01.png", 10300.0, 185.0, 90.0, 120.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_oil_drum_01.png", 10600.0, 185.0, 44.0, 55.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 10850.0, 185.0, 90.0, 60.0, PROP_LAYER_BACKGROUND);

    // --- AREA 6: RAIDER CAMP & EXIT GATE (x = 11000 to 13500) ---
    // 1. West Camp Outpost & Perimeter Barricade (x = 11000 to 11400)
    AddWorldProp("Assets/Props/Decorations/prop_sandbags_01.png", 11100.0, 185.0, 110.0, 50.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Posters/poster_quarantine_warning.png", 11150.0, 240.0, 44.0, 60.0, PROP_LAYER_BACKGROUND); // Story: Biohazard warning sign
    AddWorldProp("Assets/Props/Decorations/prop_generator_01.png", 11350.0, 185.0, 70.0, 60.0, PROP_LAYER_BACKGROUND);

    // 2. Watchtower & Campfire Hub (x = 11400 to 12200)
    AddWorldProp("Assets/Props/Military/bld_raider_watchtower.png", 11500.0, 185.0, 180.0, 280.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_wooden_crate_01.png", 11550.0, 320.0, 48.0, 48.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_oil_drum_01.png", 11620.0, 450.0, 36.0, 45.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_burning_barrel_01.png", 11850.0, 185.0, 48.0, 60.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/crate.png", 12050.0, 185.0, 56.0, 56.0, PROP_LAYER_BACKGROUND);

    // 3. Exit Gate & Luna's Ribbon Checkpoint (x = 12200 to 13500)
    AddWorldProp("Assets/Props/Buildings/Quarantine_CheckpointQuarantine_Checkpoint.png", 12400.0, 185.0, 160.0, 120.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/drum.png", 12700.0, 185.0, 50.0, 60.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Decorations/prop_broken_fence_01.png", 13000.0, 185.0, 100.0, 65.0, PROP_LAYER_BACKGROUND);
    AddWorldProp("Assets/Props/Military/Exit_Gate.png", 13200.0, 185.0, 180.0, 220.0, PROP_LAYER_BACKGROUND); // Story: Steel Exit Gate
    AddWorldProp("Assets/Props/Decorations/prop_sandbags_01.png", 13300.0, 185.0, 110.0, 50.0, PROP_LAYER_BACKGROUND);

    // Load props texture sheet (4x4 gameplay atlas)
    if (texPropsSheet == 0) {
        texPropsSheet = iLoadImage((char*)GetAssetPath("Assets/Props/props_sheet.png").c_str());
    }

    // Preload UI HUD assets from existing Assets/UI folder structure
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
        g_texItemCoin = iLoadImage((char*)GetAssetPath("Assets/Collectibles/coin.png").c_str());
    }

    // Populate Level 1 Enemies per area specification (aligned with kLevel1GroundY)
    enemies.clear();

    // Section 1: Spawn Area / Destroyed House (Background 1: 0 - 1448px) - 0 enemies

    // Section 2: Village Street (Background 2: 1448 - 2896px: 3 Walkers)
    enemies.push_back(Enemy(1800, 1950, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(2200, 2350, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(2600, 2750, kLevel1GroundY, TYPE_SPITTER));

    // Section 3: Village Square (Background 3: 2896 - 4344px: 4 Walkers, 1 Runner)
    enemies.push_back(Enemy(3100, 3220, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3350, 3470, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3600, 3720, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(3850, 3970, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(4150, 4280, kLevel1GroundY, TYPE_RUNNER));

    // Section 4: Abandoned Market (Background 4: 4344 - 5792px: 2 Walkers, 1 Raider)
    enemies.push_back(Enemy(4600, 4750, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(5000, 5150, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(5400, 5550, kLevel1GroundY, TYPE_RAIDER));

    // Section 5: Raider Camp (Background 5: 5792 - 7240px: 3 Raiders)
    enemies.push_back(Enemy(6050, 6200, kLevel1GroundY, TYPE_RAIDER));
    enemies.push_back(Enemy(6450, 6600, kLevel1GroundY, TYPE_RAIDER));
    enemies.push_back(Enemy(6850, 7000, kLevel1GroundY, TYPE_RAIDER));

    // Section 6: Abandoned Church (Background 6: 7240 - 8688px: 2 Walkers, 1 Runner)
    enemies.push_back(Enemy(7500, 7650, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(7900, 8050, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(8350, 8500, kLevel1GroundY, TYPE_RUNNER));

    // Section 7: Quarantine Zone (Background 7: 8688 - 10136px: 2 Walkers, 1 Heavy Infected)
    enemies.push_back(Enemy(8950, 9100, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(9350, 9500, kLevel1GroundY, TYPE_SPITTER));
    enemies.push_back(Enemy(9750, 9950, kLevel1GroundY, TYPE_ABOMINATION));

    // Section 8: Broken Bridge (Background 8: 10136 - 11584px) - 0 normal enemies

    // Section 9: Mini Boss Arena (Background 9: 11584 - 13032px: 1 Mutated Brute)
    enemies.push_back(Enemy(12200, 12450, kLevel1GroundY, TYPE_ABOMINATION));

    // Section 10: Exit Gate (Background 10: 13032 - 14480px) - 0 enemies

    // Populate Props (X, Y, W, H, Type, animFrame) aligned with ground baseline
    props.clear();
    // Spawn Area - family photographs and NovaGen poster
    props.push_back({ 120, 245, 32, 32, PROP_POSTER_MISSING, 0 });
    props.push_back({ 420, 225, 32, 32, PROP_POSTER_NOVAGEN, 0 });
    props.push_back({ 720, kLevel1GroundY, 48, 48, PROP_CRATE, 0 });
    props.push_back({ 900, kLevel1GroundY, 64, 64, PROP_BARREL_FIRE, 0 });
    // Destroyed House - burned furniture debris
    props.push_back({ 2800, kLevel1GroundY, 48, 48, PROP_CRATE, 0 });
    props.push_back({ 3100, kLevel1GroundY, 64, 64, PROP_BARREL_FIRE, 0 });
    // Village street props
    props.push_back({ 3800, kLevel1GroundY, 128, 64, PROP_CAR, 0 });
    props.push_back({ 4400, kLevel1GroundY, 64, 64, PROP_BARREL_FIRE, 0 });
    props.push_back({ 5100, kLevel1GroundY, 48, 48, PROP_CRATE, 0 });
    // Village square ambulance and barricades
    props.push_back({ 7200, kLevel1GroundY, 192, 96, PROP_CAR, 0 });
    props.push_back({ 7600, kLevel1GroundY, 96, 48, PROP_SANDBAG, 0 });
    props.push_back({ 7400, 255, 48, 64, PROP_POSTER_NOVAGEN, 0 });

    // Populate Collectibles aligned with ground baseline
    collectibles.clear();
    // Section 1 (Spawn Area / Destroyed House)
    collectibles.push_back({ 350, kLevel1GroundY, 32, 32, COL_SCRAP, true, 0 });      // Scrap Metal
    // Section 2 (Village Street: Food, Ammo, Battery)
    collectibles.push_back({ 3500, kLevel1GroundY, 32, 32, COL_FOOD, true, 0 });     // Food (Bread)
    collectibles.push_back({ 4200, kLevel1GroundY, 32, 32, COL_AMMO, true, 0 });     // Ammo
    collectibles.push_back({ 4900, 270, 32, 32, COL_BATTERY, true, 0 });  // Battery (Floating platform)
    // Section 3 (Village Square: Mission Note, Medkit)
    collectibles.push_back({ 6925, 250, 32, 32, COL_NOTE, true, 0 });      // Mission Note (Fountain)
    collectibles.push_back({ 7300, kLevel1GroundY, 32, 32, COL_MEDKIT, true, 0 });    // Medkit (First Aid)

    // Initialize rain particle simulation
    rainParticles.clear();
    for (int i = 0; i < 80; ++i) {
        rainParticles.push_back({ (double)(rand() % 1280), (double)(rand() % 720), 6.0 + (rand() % 40) / 10.0 });
    }

    fogParticles.clear();
    for (int i = 0; i < 24; ++i) {
        fogParticles.push_back({ (double)(rand() % 1280), (double)(80 + rand() % 420), 0.4 + (rand() % 8) / 10.0, 0.15 + (rand() % 20) / 100.0 });
    }
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
    Level1Area newArea = GetAreaFromPosition(player.x);
    if (currentState == STATE_VICTORY) {
        newArea = AREA_LEVEL_COMPLETE;
    }

    if (newArea != currentArea) {
        previousArea = currentArea;
        currentArea = newArea;
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
    // Smooth HUD fade-in animation
    hudAlpha += 3.0 * 0.016;
    if (hudAlpha > 1.0) hudAlpha = 1.0;

    // Update current level area based on player position (decoupled from background slices)
    Level1Area newArea = GetAreaFromPosition(player.x);
    if (newArea != currentArea) {
        previousArea = currentArea;
        currentArea = newArea;
        areaBannerTimer = 3.0;
        areaBannerAlpha = 1.0;
    }

    // 1. Update Player Physics and animations
    player.Update(keys, specialKeys);

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

    // Default ground fallback if not in a pit
    bool inPit = (player.x > 17800 && player.x < 18100) ||
                 (player.x > 18500 && player.x < 18800) ||
                 (player.x > 19200 && player.x < 19400);

    if (!inPit && player.y <= 185.0) {
        player.y = 185.0;
        player.vy = 0.0;
        player.isGrounded = true;
    }
    else if (inPit && player.y < -100.0) {
        // Pit safety respawn
        player.x = 17700;
        player.y = 185.0;
        player.vy = 0.0;
        player.isGrounded = true;
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

    // 4. Horizontal camera tracking
    // Check boss spawning boundary trigger
    if (player.x >= 19600 && !bossSpawned) {
        bossSpawned = true;
        // Load boss stats dynamically
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (enemies[i].type == TYPE_ABOMINATION) {
                bossMaxHp = enemies[i].maxHp;
                bossHp = enemies[i].hp;
            }
        }
    }

    // If boss fight is active, lock the player camera inside the arena bounds
    if (bossSpawned && !bossDefeated) {
        double minCam = 19400;
        double maxCam = 20500;

        double targetCam = player.x - (1280 / 2.0);
        if (targetCam < minCam) targetCam = minCam;
        if (targetCam > maxCam) targetCam = maxCam;

        // Smooth camera track locked in arena
        gameMap.ApplyCameraTracking(player.x, player.y, 1280, 720);
        if (gameMap.GetCameraX() < minCam) {
            if (player.x < 19450) player.x = 19450;
            if (player.x > 21450) player.x = 21450;
        }
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

    // 6. Update active Collectibles interaction
    for (size_t i = 0; i < collectibles.size(); ++i) {
        if (collectibles[i].active) {
            // Check bounding collision between player and collectible item
            bool intersectX = (player.x + player.width >= collectibles[i].x) && (collectibles[i].x + collectibles[i].width >= player.x);
            bool intersectY = (player.y + player.height >= collectibles[i].y) && (collectibles[i].y + collectibles[i].height >= player.y);

            if (intersectX && intersectY) {
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
                    player.foodCount++;
                    player.hp = (player.hp + 10 > player.maxHp) ? player.maxHp : player.hp + 10;
                    sprintf_s(g_pickupText, sizeof(g_pickupText), "+1 WATER (+10 HP)");
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
            enemies[i].Update(player.x, player.y);

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
                    double atkExtra = (enemies[i].type == TYPE_RUNNER) ? 40.0 : 30.0;
                    double atkX = enemies[i].isFacingRight ? enemies[i].x : (enemies[i].x - atkExtra);
                    double atkW = enemies[i].width + atkExtra;
                    bool hitX = (atkX + atkW >= player.x) && (player.x + player.width >= atkX);
                    bool hitY = (enemies[i].y + enemies[i].height >= player.y) && (player.y + player.height >= enemies[i].y);

                    if (hitX && hitY && !enemies[i].hasDealtDamage) {
                        int currentAtkFrame = enemies[i].animAttack.GetCurrentFrame();
                        int totalAtkFrames = enemies[i].animAttack.GetFrameCount();
                        bool isAtkActiveFrame = (totalAtkFrames <= 1) || (currentAtkFrame >= 0 && currentAtkFrame <= 6);

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
                double hitboxX = player.isFacingRight ? player.x : (player.x - 70.0);
                double hitboxW = player.width + 70.0;
                double hitboxY = player.y;
                double hitboxH = player.height;

                bool hitX = (hitboxX + hitboxW >= enemies[i].x) && (enemies[i].x + enemies[i].width >= hitboxX);
                bool hitY = (hitboxY + hitboxH >= enemies[i].y) && (enemies[i].y + enemies[i].height >= hitboxY);

                if (hitX && hitY) {
                    enemies[i].TakeDamage(35);
                    score += 50;
                    enemies[i].lastHitAttackID = player.currentAttackID;
                }
            }
        }
        else {
            // Process death frames for enemy
            enemies[i].Update(player.x, player.y);

            // Check if boss died
            if (enemies[i].type == TYPE_ABOMINATION) {
                bossDefeated = true;
            }
        }
    }

    // 8. Exit Gate Ending Trigger
    if (player.x >= 21550 && bossDefeated) {
        if (!hasKeycard) {
            if (currentState == STATE_PLAYING && player.x >= 21550) {
                currentState = STATE_DIALOGUE;
                sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
                sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"The steel gate is locked. I need a NovaGen keycard from the quarantine checkpoint.\"");
            }
        }
        else if (!ribbonCollected) {
            currentState = STATE_DIALOGUE;
            sprintf_s(g_dialogueSpeaker, sizeof(g_dialogueSpeaker), "Arin");
            sprintf_s(g_dialogueText, sizeof(g_dialogueText), "\"Luna's silver-blue ribbon! It's caught on the steel gate latch... She survived. I will find you, Luna!\"");
            ribbonCollected = true;
        }
        else if (currentState != STATE_DIALOGUE && currentState != STATE_VICTORY) {
            currentState = STATE_VICTORY;
            menuTransitionAlpha = 1.0;
            leaderboard.AddScore("Arin", score);
        }
    }

    // Check Player Game Over: Play full death animation before showing Game Over menu screen
    if (player.hp <= 0 && currentState != STATE_GAMEOVER) {
        if (player.state == STATE_DEAD && player.animDeath.IsFinished()) {
            currentState = STATE_GAMEOVER;
            menuTransitionAlpha = 1.0;
        }
    }
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

        // Dark background dimmer
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, 1280, 720);

        if (g_texPauseOverlay != 0) {
            iShowImage(240, 110, 800, 500, g_texPauseOverlay);
        } else {
            iSetColor(12, 16, 24);
            iFilledRectangle(360, 240, 560, 240);
            iSetColor(0, 180, 220);
            iRectangle(360, 240, 560, 240);
        }

        // Top Carved Banner Slot: GAME PAUSED Title
        DrawOutlinedText(535, 525, "GAME PAUSED", GLUT_BITMAP_TIMES_ROMAN_24, 0, 230, 255);

        // Slot 1: Resume Game
        RenderMenuButtonSlot(1, 525, 440, "1. RESUME GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

        // Slot 2: Restart Level
        RenderMenuButtonSlot(2, 545, 362, "2. RESTART LEVEL", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

        // Slot 3: Quit to Menu
        RenderMenuButtonSlot(3, 530, 285, "3. QUIT TO MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

        // Bottom Detail Slot: Footer instructions
        DrawShadowText(500, 148, "Press ESC to Resume or Click Option", GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
        break;
    case STATE_GAMEOVER:
        RenderGameOver();
        break;
    case STATE_VICTORY:
        RenderVictory();
        break;
    case STATE_LEADERBOARD:
        RenderLeaderboard();
        break;
    }

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
void GameManager::AddWorldProp(const std::string& assetPath, double x, double y, double width, double height, PropLayer layer) {
    WorldProp wp;
    wp.x = x;
    wp.y = y;
    wp.width = width;
    wp.height = height;
    wp.assetPath = assetPath;
    wp.textureID = ResourceManager::GetInstance().GetTexture(assetPath);
    wp.layer = layer;
    wp.visible = true;

    worldProps.push_back(wp);
    printf("[Prop System] Registered WorldProp: %s at (%.1f, %.1f) scale (%.1f x %.1f)\n", assetPath.c_str(), x, y, width, height);
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

    // Tall structures / nature (Trees, poles, lamps, watchtower)
    if (assetPath.find("nature_dead_tree") != std::string::npos ||
        assetPath.find("telephone_pole") != std::string::npos ||
        assetPath.find("street_lamp") != std::string::npos ||
        assetPath.find("watchtower") != std::string::npos) {
        return kPropScaleTall; // 1.8x
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
    // 1. Chair & Shopping Cart: PNG transparent bottom padding (~12px)
    if (assetPath.find("chair") != std::string::npos ||
        assetPath.find("shopping_cart") != std::string::npos) {
        return 12.0;
    }

    // 2. Other Furniture (Table, Bed, Bench, Cabinet, Shelf): PNG padding (~8px)
    if (assetPath.find("Furniture/") != std::string::npos ||
        assetPath.find("table") != std::string::npos ||
        assetPath.find("bed") != std::string::npos ||
        assetPath.find("bench") != std::string::npos ||
        assetPath.find("cabinet") != std::string::npos ||
        assetPath.find("shelf") != std::string::npos) {
        return 8.0;
    }

    // 3. Barrels, Drums, Crates & Fences: PNG padding (~6px)
    if (assetPath.find("barrel") != std::string::npos ||
        assetPath.find("drum") != std::string::npos ||
        assetPath.find("crate") != std::string::npos ||
        assetPath.find("fence") != std::string::npos) {
        return 6.0;
    }

    // 4. Sandbags, Generators, Stones & Bushes (~4px)
    if (assetPath.find("sandbags") != std::string::npos ||
        assetPath.find("generator") != std::string::npos ||
        assetPath.find("stone") != std::string::npos ||
        assetPath.find("bush") != std::string::npos) {
        return 4.0;
    }

    // 5. Default ground offset for all other environment props
    return 4.0;
}

void GameManager::RenderWorldProps(PropLayer layer, double camX, double camY) {
    // Enable OpenGL Alpha Blending for clean PNG transparency across all prop textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
        // If prop rests on main ground baseline (y == 185.0), apply Arin's boot alignment reference (-6.0)
        // plus the asset's PNG transparent padding offset (groundOffset) so the visible base touches ground Y.
        if (std::abs(worldProps[i].y - kLevel1GroundY) < 1.0) {
            double groundOffset = GetPropGroundOffset(worldProps[i].assetPath);
            renderY += (-6.0 + groundOffset);
        }

        // Viewport frustum culling check (-100 to 1380)
        if (renderX + renderW >= -100 && renderX <= 1380) {
            if (worldProps[i].textureID != 0) {
                iShowImage((int)renderX, (int)renderY, (int)renderW, (int)renderH, worldProps[i].textureID);
            }
        }
    }
}

void GameManager::RenderPlaying() {
    double camX = gameMap.GetCameraX();
    double camY = gameMap.GetCameraY();

    // ========================================================================
    // LAYER 1: BACKGROUND (Parallax backdrop & surface tiles)
    // ========================================================================
    gameMap.RenderBackground(camX, bossDefeated);
    gameMap.RenderTiles(camX, camY);

    // ========================================================================
    // LAYER 2: LARGE ENVIRONMENT OBJECTS (Vehicles, buildings, trees, background props)
    // ========================================================================
    RenderWorldProps(PROP_LAYER_BACKGROUND, camX, camY);

    // Render level props (Environmental obstacles & burning barrels)
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
                // Dark oil barrel base
                iSetColor(25, 28, 35);
                iFilledRectangle(px, py, pw, ph);
                iSetColor(80, 85, 95);
                iRectangle(px, py, pw, ph);
                iSetColor(180, 40, 20); // Rust band
                iFilledRectangle(px + 2, py + ph / 2 - 3, pw - 4, 6);

                // Animated flickering fire flames on top of barrel
                double fTime = uiAnimTime * 12.0 + i;
                int fHeight = 24 + (int)(sin(fTime) * 6.0);
                iSetColor(255, 140, 0); // Orange outer fire
                iFilledRectangle(px + 8, py + ph - 4, pw - 16, fHeight);
                iSetColor(255, 220, 0); // Yellow inner core fire
                iFilledRectangle(px + 14, py + ph - 4, pw - 28, fHeight - 8);
                break;
            }
            case PROP_CRATE:
                iSetColor(55, 40, 25); // Wooden crate body
                iFilledRectangle(px, py, pw, ph);
                iSetColor(120, 90, 50); // Wood frame border
                iRectangle(px, py, pw, ph);
                iSetColor(90, 70, 40); // Diagonal braces
                iLine(px + 2, py + 2, px + pw - 2, py + ph - 2);
                iLine(px + pw - 2, py + 2, px + 2, py + ph - 2);
                break;
            case PROP_SANDBAG:
                iSetColor(70, 65, 50); // Sandbag burlap wall
                iFilledRectangle(px, py, pw, ph);
                iSetColor(140, 130, 100);
                iRectangle(px, py, pw, ph);
                iSetColor(40, 35, 25);
                iLine(px, py + ph / 2, px + pw, py + ph / 2);
                break;
            case PROP_DRUM:
                iSetColor(30, 40, 50); // Steel fuel drum
                iFilledRectangle(px, py, pw, ph);
                iSetColor(0, 180, 220); // Cyan hazmat stripe
                iRectangle(px, py, pw, ph);
                iSetColor(0, 200, 240);
                iFilledRectangle(px + 4, py + ph / 2 - 4, pw - 8, 8);
                break;
            case PROP_CAR:
                iSetColor(20, 24, 30); // Destroyed car chassis
                iFilledRectangle(px, py, pw, ph);
                iSetColor(60, 70, 85);
                iRectangle(px, py, pw, ph);
                iSetColor(180, 30, 30); // Broken tail lamps
                iFilledRectangle(px + 4, py + ph - 16, 12, 8);
                break;
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

    // Render Collectibles with floating animation, ground pod indicators, and high visibility
    for (size_t i = 0; i < collectibles.size(); ++i) {
        if (collectibles[i].active) {
            double screenPx = collectibles[i].x - camX;
            double groundY = collectibles[i].y - (std::abs(collectibles[i].y - kLevel1GroundY) < 1.0 ? 6.0 : 0.0);
            double bobY = groundY + (sin(uiAnimTime * 4.0 + i) * 3.0 + 3.0);

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
                    itemTex = g_texItemRustyKey;
                    itemLabel = "KEYCARD";
                    lR = 255; lG = 215; lB = 0;
                    itemDrawW = 42; itemDrawH = 42;
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
                    itemTex = 0;
                    itemLabel = "BATTERY";
                    lR = 0; lG = 255; lB = 200;
                    itemDrawW = 36; itemDrawH = 46;
                    break;
                case COL_NOTE:
                    itemTex = 0;
                    itemLabel = "MISSION NOTE";
                    lR = 0; lG = 230; lB = 255;
                    itemDrawW = 38; itemDrawH = 46;
                    break;
                default:
                    break;
                }

                int drawX = px - itemDrawW / 2;
                int drawY = py;

                // 1. Ground highlight aura pod (Visibility indicator on floor)
                int groundScreenY = (int)(groundY - camY);
                iSetColor(lR / 4, lG / 4, lB / 4);
                iFilledRectangle(drawX - 4, groundScreenY, itemDrawW + 8, 4);
                iSetColor(lR / 2, lG / 2, lB / 2);
                iFilledRectangle(drawX, groundScreenY + 1, itemDrawW, 2);

                // 2. Render item sprite texture or procedural fallback
                if (itemTex != 0) {
                    iShowImage(drawX, drawY, itemDrawW, itemDrawH, itemTex);
                }
                else {
                    // Procedural fallback rendering cleanly scaled to item dimensions
                    switch (collectibles[i].type) {
                    case COL_MEDKIT:
                        iSetColor(180, 20, 20);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 220, 255);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(255, 255, 255);
                        iFilledRectangle(drawX + itemDrawW / 3, drawY + 8, itemDrawW / 3, itemDrawH - 16);
                        iFilledRectangle(drawX + 8, drawY + itemDrawH / 3, itemDrawW - 16, itemDrawH / 3);
                        break;
                    case COL_AMMO:
                    case COL_COIN:
                        iSetColor(40, 45, 50);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(255, 215, 0);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(220, 180, 20);
                        iFilledRectangle(drawX + itemDrawW / 4, drawY + 8, itemDrawW / 2, itemDrawH - 16);
                        break;
                    case COL_WATER:
                        iSetColor(15, 60, 100);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 240, 255);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 200, 255);
                        iFilledRectangle(drawX + itemDrawW / 4, drawY + 6, itemDrawW / 2, itemDrawH - 12);
                        break;
                    case COL_BATTERY:
                        iSetColor(30, 30, 35);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 255, 200);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 255, 180);
                        iFilledRectangle(drawX + itemDrawW / 4, drawY + 6, itemDrawW / 2, itemDrawH - 12);
                        break;
                    case COL_FOOD:
                        iSetColor(60, 45, 20);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(255, 180, 0);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(240, 160, 40);
                        iFilledRectangle(drawX + itemDrawW / 4, drawY + 6, itemDrawW / 2, itemDrawH - 12);
                        break;
                    case COL_NOTE:
                        iSetColor(20, 25, 35);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 220, 255);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 240, 255);
                        iFilledRectangle(drawX + itemDrawW / 4, drawY + 6, itemDrawW / 2, itemDrawH - 12);
                        break;
                    case COL_KEYCARD:
                    case COL_RUSTY_KEY:
                        iSetColor(10, 30, 50);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(255, 215, 0);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(0, 220, 255);
                        iFilledRectangle(drawX + 6, drawY + itemDrawH / 3, itemDrawW - 12, itemDrawH / 3);
                        break;
                    case COL_SCRAP:
                        iSetColor(45, 50, 60);
                        iFilledRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(200, 210, 220);
                        iRectangle(drawX, drawY, itemDrawW, itemDrawH);
                        iSetColor(180, 190, 200);
                        iFilledRectangle(drawX + itemDrawW / 4, drawY + 6, itemDrawW / 2, itemDrawH - 12);
                        break;
                    }
                }

                // 3. Outlined text label above item
                int labelX = px - ((int)strlen(itemLabel) * 6) / 2;
                DrawOutlinedText(labelX, drawY + itemDrawH + 6, itemLabel, GLUT_BITMAP_HELVETICA_10, lR, lG, lB);
            }
        }
    }

    // ========================================================================
    // LAYER 3: CHARACTERS (Enemies & Player Arin)
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
    // LAYER 4: FOREGROUND OBJECTS (Foreground props in front of characters)
    // ========================================================================
    RenderWorldProps(PROP_LAYER_FOREGROUND, camX, camY);

    // ========================================================================
    // LAYER 5: EFFECTS (Floating popups, particle effects, HUD overlays)
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
    if (hudAlpha < 0.05) return;

    // Enable OpenGL Alpha Blending for clean PNG transparency across all elements
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ------------------------------------------------------------------------
    // TOP LEFT PANEL: Health & Stamina Bars (using ui_health_frame & ui_health_fill)
    // ------------------------------------------------------------------------
    // Only draw fallback panel if PNG texture frame is absent
    if (g_texHealthFrame == 0) {
        iSetColor(12, 16, 24);
        iFilledRectangle(20, 630, 300, 70);
        iSetColor(220, 40, 40); // Red accent border stroke
        iRectangle(20, 630, 300, 70);
    }

    // Smooth animated health percentage
    double playerHpPercent = player.displayedHp / (double)player.maxHp;
    if (playerHpPercent < 0.0) playerHpPercent = 0.0;
    if (playerHpPercent > 1.0) playerHpPercent = 1.0;

    // Health Fill Texture (Dynamically animated & clipped to playerHpPercent)
    if (g_texHealthFill != 0) {
        int fillWidth = (int)(220.0 * playerHpPercent);
        if (fillWidth > 0) {
            iShowImageSub(32, 665, fillWidth, 20, g_texHealthFill, 0.0, 0.0, playerHpPercent, 1.0);
        }
    } else {
        iSetColor(220, 35, 35);
        iFilledRectangle(32, 665, 220 * playerHpPercent, 20);
    }

    // Health Frame Texture
    if (g_texHealthFrame != 0) {
        iShowImage(20, 655, 270, 40, g_texHealthFrame);
    }

    // Health Percentage Text with drop shadow
    char hpPctStr[32];
    int hpPct = (int)((double)player.hp / player.maxHp * 100.0);
    sprintf_s(hpPctStr, sizeof(hpPctStr), "%d%%", hpPct);
    iSetColor(0, 0, 0);
    iText(261, 669, hpPctStr, GLUT_BITMAP_HELVETICA_12);
    iSetColor(255, 255, 255);
    iText(260, 670, hpPctStr, GLUT_BITMAP_HELVETICA_12);

    // Health Label Overlay with subtle shadow
    char hpLabelStr[32];
    sprintf_s(hpLabelStr, sizeof(hpLabelStr), "HEALTH  %d / %d", player.hp, player.maxHp);
    iSetColor(0, 0, 0);
    iText(41, 669, hpLabelStr, GLUT_BITMAP_HELVETICA_12);
    iSetColor(255, 255, 255);
    iText(40, 670, hpLabelStr, GLUT_BITMAP_HELVETICA_12);

    // Stamina Bar Frame & Fill (Dynamically animated & clipped to staminaPercent)
    double staminaPercent = player.displayedStamina / (double)player.maxStamina;
    if (staminaPercent < 0.0) staminaPercent = 0.0;
    if (staminaPercent > 1.0) staminaPercent = 1.0;

    if (g_texStaminaFill != 0) {
        int stamWidth = (int)(276.0 * staminaPercent);
        if (stamWidth > 0) {
            iShowImageSub(32, 642, stamWidth, 10, g_texStaminaFill, 0.0, 0.0, staminaPercent, 1.0);
        }
    } else {
        iSetColor(230, 190, 30);
        iFilledRectangle(32, 642, 276 * staminaPercent, 8);
    }

    if (g_texStaminaFrame != 0) {
        iShowImage(20, 635, 290, 22, g_texStaminaFrame);
    }


    // ------------------------------------------------------------------------
    // TOP RIGHT PANEL: Current Weapon (Katana), Medkits, Food, Battery
    // ------------------------------------------------------------------------
    iSetColor(0, 0, 0);
    iFilledRectangle(938, 608, 324, 94);
    iSetColor(12, 16, 24);
    iFilledRectangle(940, 610, 320, 90);
    iSetColor(0, 180, 220); // Anime cyan outline
    iRectangle(940, 610, 320, 90);

    // Weapon Icon & Status
    iSetColor(0, 210, 240);
    iText(952, 680, "WEAPON: KATANA [J]", GLUT_BITMAP_HELVETICA_12);

    // Survival Supplies (Medkit, Food, Battery)
    char medCountStr[32], foodCountStr[32], batCountStr[32];
    sprintf_s(medCountStr, sizeof(medCountStr), "Medkits : %d", player.medkits);
    sprintf_s(foodCountStr, sizeof(foodCountStr), "Food    : %d", player.foodCount);
    sprintf_s(batCountStr,  sizeof(batCountStr),  "Battery : %d", player.batteryCount);

    // Render clean procedural HUD icons alongside counts
    // Medkit Mini Icon
    iSetColor(180, 20, 20);
    iFilledRectangle(950, 634, 20, 20);
    iSetColor(255, 255, 255);
    iFilledRectangle(958, 637, 4, 14);
    iFilledRectangle(953, 642, 14, 4);

    // Food Mini Icon
    iSetColor(160, 100, 30);
    iFilledRectangle(1055, 634, 20, 20);
    iSetColor(240, 180, 40);
    iRectangle(1055, 634, 20, 20);

    // Battery Mini Icon
    iSetColor(30, 30, 40);
    iFilledRectangle(1160, 634, 20, 20);
    iSetColor(0, 255, 180);
    iRectangle(1160, 634, 20, 20);
    iFilledRectangle(1165, 638, 10, 12);

    iSetColor(0, 0, 0);
    iText(981, 639, medCountStr, GLUT_BITMAP_HELVETICA_12);
    iText(1086, 639, foodCountStr, GLUT_BITMAP_HELVETICA_12);
    iText(1191, 639, batCountStr, GLUT_BITMAP_HELVETICA_12);

    iSetColor(255, 255, 255);
    iText(980, 640, medCountStr, GLUT_BITMAP_HELVETICA_12);
    iText(1085, 640, foodCountStr, GLUT_BITMAP_HELVETICA_12);
    iText(1190, 640, batCountStr, GLUT_BITMAP_HELVETICA_12);

    // Score Banner
    iSetColor(0, 0, 0);
    iFilledRectangle(1058, 568, 204, 34);
    iSetColor(12, 16, 24);
    iFilledRectangle(1060, 570, 200, 30);
    iSetColor(255, 215, 0);
    char scoreStr[32];
    sprintf_s(scoreStr, sizeof(scoreStr), "SCORE: %07d", score);
    iText(1075, 578, scoreStr, GLUT_BITMAP_HELVETICA_12);


    // ------------------------------------------------------------------------
    // TOP LEFT PANEL: Mission Objective Box (Positioned directly below Health Bar)
    // ------------------------------------------------------------------------
    int missX = 20;
    int missY = 550;
    int missW = 300;
    int missH = 65;

    if (g_texMissionBox != 0) {
        iShowImage(missX, missY, missW, missH, g_texMissionBox);
    } else {
        iSetColor(12, 16, 24);
        iFilledRectangle(missX, missY, missW, missH);
        iSetColor(0, 180, 220);
        iRectangle(missX, missY, missW, missH);
    }

    // Active Objective Text evaluation
    const char* activeObjText = "Escape the Fallen Village";
    if (bossSpawned && !bossDefeated) {
        activeObjText = "DEFEAT MUTATED BRUTE";
    }
    else if (bossDefeated && !ribbonCollected) {
        activeObjText = "Reach the Steel Exit Gate";
    }
    else if (bossDefeated && ribbonCollected) {
        activeObjText = "Press ENTER to Escape";
    }

    // Animated glow pulse when objective updates
    if (missionNotifyTimer > 0.0) {
        double objPulse = 0.7 + 0.3 * sin(uiAnimTime * 12.0);
        int gCol = (int)(255 * objPulse);
        int bCol = (int)(255 * objPulse);

        // Glowing border highlight animation
        iSetColor(0, gCol, bCol);
        iRectangle(missX - 2, missY - 2, missW + 4, missH + 4);
        iRectangle(missX - 1, missY - 1, missW + 2, missH + 2);

        // Objective Header
        DrawOutlinedText(missX + 12, missY + missH - 22, "MISSION DIRECTIVE (UPDATED)", GLUT_BITMAP_HELVETICA_10, 0, 255, 220);
        // Objective Body Text with Gold Pulse Tint
        DrawShadowText(missX + 12, missY + 16, activeObjText, GLUT_BITMAP_HELVETICA_12, 255, 220, 0);
    } else {
    // Standard Objective Header with Current Area Tag
        char headerStr[64];
        sprintf_s(headerStr, sizeof(headerStr), "AREA: %s", GetAreaName(currentArea));
        DrawOutlinedText(missX + 12, missY + missH - 22, headerStr, GLUT_BITMAP_HELVETICA_10, 0, 220, 255);
        // Standard Objective Body Text
        DrawShadowText(missX + 12, missY + 16, activeObjText, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
    }

    // Internal enemy count evaluation (disabled from gameplay screen drawing)
    static bool g_showDebugOverlay = false;
    if (g_showDebugOverlay) {
        int expectedCount = 0;
        switch (currentArea) {
        case AREA_SPAWN_AREA:
        case AREA_DESTROYED_HOUSE:  expectedCount = 0; break;
        case AREA_VILLAGE_STREET:   expectedCount = 3; break;
        case AREA_VILLAGE_SQUARE:   expectedCount = 5; break;
        case AREA_ABANDONED_MARKET: expectedCount = 3; break;
        case AREA_RAIDER_CAMP:      expectedCount = 3; break;
        case AREA_ABANDONED_CHURCH: expectedCount = 3; break;
        case AREA_QUARANTINE_ZONE:  expectedCount = 3; break;
        case AREA_BROKEN_BRIDGE:    expectedCount = 0; break;
        case AREA_MINI_BOSS_ARENA:  expectedCount = 1; break;
        case AREA_EXIT_GATE:        expectedCount = 0; break;
        case AREA_LEVEL_COMPLETE:   expectedCount = 0; break;
        default:                    expectedCount = 0; break;
        }

        int actualCount = 0;
        for (size_t i = 0; i < enemies.size(); ++i) {
            if (enemies[i].hp > 0 && GetAreaFromPosition(enemies[i].x) == currentArea) {
                actualCount++;
            }
        }

        char debugEnemyStr[128];
        sprintf_s(debugEnemyStr, sizeof(debugEnemyStr), "[DEBUG] AREA: %s | EXPECTED: %d | ACTUAL: %d", GetAreaName(currentArea), expectedCount, actualCount);
        DrawOutlinedText(20, 528, debugEnemyStr, GLUT_BITMAP_HELVETICA_10, 0, 255, 200);
    }

    // ------------------------------------------------------------------------
    // SLEEK CINEMATIC AREA TITLE BANNER (Upper Center)
    // ------------------------------------------------------------------------
    if (areaBannerAlpha > 0.01) {
        const char* areaName = GetAreaName(currentArea);

        int textLen = (int)strlen(areaName);
        int bannerW = 320 + (textLen * 8);
        if (bannerW < 380) bannerW = 380;
        int bannerH = 48;
        int bannerX = 640 - (bannerW / 2);
        int bannerY = 605;

        // Dark glass background panel
        iSetColor(10, 14, 22);
        iFilledRectangle(bannerX, bannerY, bannerW, bannerH);

        // Dual cyan and gold accent border stroke
        iSetColor(0, 220, 255);
        iRectangle(bannerX, bannerY, bannerW, bannerH);
        iSetColor(255, 215, 0);
        iRectangle(bannerX + 2, bannerY + 2, bannerW - 4, bannerH - 4);

        // Level 1 Chapter Title (Upper Header)
        int headerX = 640 - 55;
        DrawOutlinedText(headerX, bannerY + 30, "THE FALLEN VILLAGE", GLUT_BITMAP_HELVETICA_10, 0, 240, 255);

        // Current Area Name (Main Title)
        int titleX = 640 - (textLen * 4);
        DrawShadowText(titleX, bannerY + 10, areaName, GLUT_BITMAP_HELVETICA_12, 255, 220, 0);
    }

    // ------------------------------------------------------------------------
    // FADING MISSION OBJECTIVE NOTIFICATION BANNER
    // ------------------------------------------------------------------------
    if (missionNotifyAlpha > 0.01) {
        // Dark banner backdrop with drop shadow
        iSetColor(0, 0, 0);
        iFilledRectangle(438, 638, 404, 54);
        iSetColor(12, 16, 24);
        iFilledRectangle(440, 640, 400, 50);

        // Glowing animated cyan border
        iSetColor(0, 220, 255);
        iRectangle(440, 640, 400, 50);

        // Banner text
        DrawOutlinedText(525, 670, "MISSION DIRECTIVE UPDATED", GLUT_BITMAP_HELVETICA_10, 0, 230, 255);
        DrawShadowText(480, 650, activeObjText, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
    }


    // ------------------------------------------------------------------------
    // BOTTOM RIGHT PANEL: Inventory & Pause Controls
    // ------------------------------------------------------------------------
    iSetColor(0, 0, 0);
    iFilledRectangle(1038, 18, 224, 69);
    iSetColor(12, 16, 24);
    iFilledRectangle(1040, 20, 220, 65);
    iSetColor(0, 180, 220);
    iRectangle(1040, 20, 220, 65);

    // Keycard / Inventory Mini Icon
    iSetColor(10, 30, 50);
    iFilledRectangle(1050, 35, 28, 28);
    iSetColor(0, 220, 255);
    iRectangle(1050, 35, 28, 28);
    iSetColor(255, 215, 0);
    iFilledRectangle(1054, 46, 20, 5);

    DrawOutlinedText(1090, 52, "[TAB] Inventory", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
    DrawOutlinedText(1090, 32, "[ESC] Pause", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);


    // ------------------------------------------------------------------------
    // TOP CENTER: Boss Health Bar (during Boss Fight)
    // ------------------------------------------------------------------------
    if (bossSpawned && !bossDefeated) {
        iSetColor(0, 0, 0);
        iFilledRectangle(438, 638, 404, 44);
        iSetColor(15, 15, 22);
        iFilledRectangle(440, 640, 400, 40);
        iSetColor(220, 40, 40);
        iRectangle(440, 640, 400, 40);

        iSetColor(40, 10, 10);
        iFilledRectangle(450, 648, 380, 14);
        iSetColor(220, 30, 30);
        double bossHpPercent = (double)bossHp / bossMaxHp;
        if (bossHpPercent < 0.0) bossHpPercent = 0.0;
        if (bossHpPercent > 1.0) bossHpPercent = 1.0;
        iFilledRectangle(450, 648, 380 * bossHpPercent, 14);

        iSetColor(0, 0, 0);
        iText(536, 665, "MUTATED BRUTE (MINI BOSS)", GLUT_BITMAP_HELVETICA_12);
        iSetColor(255, 255, 255);
        iText(535, 666, "MUTATED BRUTE (MINI BOSS)", GLUT_BITMAP_HELVETICA_12);
    }

    // ------------------------------------------------------------------------
    // INVENTORY OVERLAY (Rendered when TAB or [I] is pressed)
    // ------------------------------------------------------------------------
    if (showInventory) {
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

        // Header Title
        iSetColor(0, 220, 255);
        iText(panelX + 200, panelY + panelH - 40, "SURVIVAL INVENTORY", GLUT_BITMAP_HELVETICA_18);

        // Render Inventory Grid Slots (using ui_inventory_slot.png / single_empty_inventory_slot.png)
        int cols = 4, rows = 3;
        int slotW = 85, slotH = 85;
        int startX = panelX + 70;
        int startY = panelY + panelH - 160;
        int gapX = 35, gapY = 20;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int slotX = startX + c * (slotW + gapX);
                int slotY = startY - r * (slotH + gapY);

                if (g_texInventorySlot != 0) {
                    iShowImage(slotX, slotY, slotW, slotH, g_texInventorySlot);
                } else {
                    iSetColor(20, 25, 35);
                    iFilledRectangle(slotX, slotY, slotW, slotH);
                    iSetColor(0, 150, 180);
                    iRectangle(slotX, slotY, slotW, slotH);
                }
            }
        }

        // Bottom instruction label
        iSetColor(200, 210, 220);
        iText(panelX + 180, panelY + 30, "Press [TAB] or [ESC] to Close", GLUT_BITMAP_HELVETICA_12);
    }
}

void GameManager::RenderDialogue() {
    RenderPlaying(); // Render gameplay backdrop

    // Transparent dialog frame centered at bottom of window
    int panelX = 90;
    int panelY = 30;
    int panelW = 1100;
    int panelH = 180;

    // Dark semi-transparent dialogue background
    iSetColor(8, 12, 22);
    iFilledRectangle(panelX, panelY, panelW, panelH);

    // Glowing cyan outer frame
    iSetColor(0, 190, 220);
    iRectangle(panelX, panelY, panelW, panelH);
    iSetColor(0, 120, 150);
    iRectangle(panelX + 2, panelY + 2, panelW - 4, panelH - 4);

    // Header Title (Speaker / Note Title)
    iSetColor(0, 230, 255);
    iText(panelX + 30, panelY + panelH - 32, g_dialogueSpeaker, GLUT_BITMAP_HELVETICA_18);

    // Header accent divider line
    iSetColor(0, 140, 170);
    iLine(panelX + 25, panelY + panelH - 42, panelX + panelW - 25, panelY + panelH - 42);

    // Body Text Multi-line Word-Wrapping Engine
    int textStartX = panelX + 30;
    int textStartY = panelY + panelH - 70;
    int maxPixelWidth = panelW - 60; // 1040px text width
    int lineHeight = 24;

    std::string textStr(g_dialogueText);
    std::vector<std::string> lines;

    // Max characters per line for GLUT_BITMAP_HELVETICA_18 (~9.5px per char)
    int maxCharsPerLine = (int)(maxPixelWidth / 9.5); // ~109 chars
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

    // Render body text lines inside panel boundaries
    iSetColor(240, 245, 255);
    int currentY = textStartY;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (currentY >= panelY + 25) { // Ensure no text extends past bottom padding
            iText(textStartX, currentY, (char*)lines[i].c_str(), GLUT_BITMAP_HELVETICA_18);
        }
        currentY -= lineHeight;
    }

    // Footer prompt aligned in bottom right inside frame
    iSetColor(150, 165, 180);
    iText(panelX + panelW - 220, panelY + 16, "Press [ENTER] to Continue", GLUT_BITMAP_HELVETICA_12);
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
    const char* vicTitle = "LEVEL 1 COMPLETE";
    DrawOutlinedText(515, 525, vicTitle, GLUT_BITMAP_TIMES_ROMAN_24, 0, 255, 120);

    // 3. Option 1: Next Level (Slot 1)
    RenderMenuButtonSlot(1, 525, 440, "1. NEXT LEVEL [ENTER]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 4. Option 2: Main Menu (Slot 2)
    RenderMenuButtonSlot(2, 545, 362, "2. MAIN MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 5. Option 3: Exit Game (Slot 3)
    RenderMenuButtonSlot(3, 535, 285, "3. EXIT GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, uiAnimTime);

    // 6. Bottom Detail Slot: Instructions
    const char* vicFooter = "Press [ENTER], [M], [ESC] or Click Options to Select";
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
                menuTransitionAlpha = 1.0;
            }
        }
        else if (key == 9 || key == '\t' || key == 'i' || key == 'I') {
            showInventory = !showInventory;
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
    }
    else if (currentState == STATE_PAUSED) {
        if (key == 27) { // ESC resumes normal gameplay
            currentState = STATE_PLAYING;
            menuTransitionAlpha = 1.0;
        }
        else if (key == 'm' || key == 'M' || key == 13) { // Quit to menu
            currentState = STATE_MENU;
            menuTransitionAlpha = 1.0;
        }
    }
    else if (currentState == STATE_DIALOGUE) {
        if (key == 13) { // Enter key
            currentState = STATE_PLAYING;
            menuTransitionAlpha = 1.0;
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
        if (key == 13 || key == '1') { // Enter or 1 = Next Level / Restart Level
            Initialize();
            currentState = STATE_PLAYING;
            menuTransitionAlpha = 1.0;
        }
        else if (key == 'm' || key == 'M' || key == '2') { // M or 2 = Main Menu
            currentState = STATE_MENU;
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
                // Slot 1: Start Survival
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
                // Slot 1: Resume Game
                if (mx >= 440 && mx <= 840 && my >= 420 && my <= 470) {
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 2: Restart Level
                else if (mx >= 440 && mx <= 840 && my >= 345 && my <= 390) {
                    Initialize();
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 3: Quit to Menu
                else if (mx >= 440 && mx <= 840 && my >= 265 && my <= 310) {
                    currentState = STATE_MENU;
                    menuTransitionAlpha = 1.0;
                }
            }
            else if (currentState == STATE_GAMEOVER) {
                // Slot 1: Restart Game
                if (mx >= 440 && mx <= 840 && my >= 420 && my <= 470) {
                    Initialize();
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
                // Slot 1: Next Level
                if (mx >= 440 && mx <= 840 && my >= 420 && my <= 470) {
                    Initialize();
                    currentState = STATE_PLAYING;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 2: Main Menu
                else if (mx >= 440 && mx <= 840 && my >= 345 && my <= 390) {
                    currentState = STATE_MENU;
                    menuTransitionAlpha = 1.0;
                }
                // Slot 3: Exit Game
                else if (mx >= 440 && mx <= 840 && my >= 265 && my <= 310) {
                    exit(0);
                }
            }
            else if (currentState == STATE_PLAYING) {
                if (!showInventory) player.AttackMelee();
            }
        }
    }
}

void GameManager::AddScore(int amount) {
    score += amount;
}

Level1Area GameManager::GetAreaFromPosition(double px) const {
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