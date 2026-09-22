#ifndef UI_H
#define UI_H

#include "player.h"
#include <string>

// ============================================================================
// UI System Module for GENESIS: WHISPERS OF THE INFECTED
// ============================================================================

struct NotificationData {
    std::string title;
    std::string message;
    double timer;
    double maxTimer;
    bool active;
};

class UI {
private:
    // Texture Handles
    static unsigned int texHealthFrame;
    static unsigned int texHealthFill;
    static unsigned int texStaminaFrame;
    static unsigned int texStaminaFill;
    static unsigned int texMissionBox;
    static unsigned int texSurvivalPanel;
    static unsigned int texInventoryPanel;
    static unsigned int texInventorySlot;
    static unsigned int texPauseOverlay;
    static unsigned int texGameOverBg;
    static unsigned int texLevelCompleteBg;
    static unsigned int texMainMenuBg;
    static unsigned int texBossFrame;
    static unsigned int texBossFill;
    static unsigned int texScoreLabel;
    static unsigned int texSeparator;
    static unsigned int texHealLabel;
    static unsigned int texScoreHeal;
    static unsigned int texControlsScreen;

    // Item & HUD Icon Handles
    static unsigned int texIconMedkit;
    static unsigned int texIconFood;
    static unsigned int texIconBattery;
    static unsigned int texIconWaterBottle;
    static unsigned int texIconScrap;
    static unsigned int texIconKatana;
    static unsigned int texIconPistol;
    static unsigned int texIconSMG;
    static unsigned int texIconGrenade;
    static unsigned int texIconShotgun;
    static unsigned int texIconShotgunAmmo;

    // Notification state
    static NotificationData currentNotification;

public:
    // Helper text & UI frame utilities
    static void DrawShadowText(int x, int y, const char* str, void* font, int r, int g, int b, int shadowOffset = 1);
    static void DrawOutlinedText(int x, int y, const char* str, void* font, int r, int g, int b);
    static int GetTextWidth(const char* str, void* font);
    static void DrawButtonSlot(int slotIdx, int textX, int textY, const char* label, void* font, int mouseX, int mouseY, bool isMouseDown, double animTime = 0.0);
    static void DrawKatanaStyleBox(int boxX, int boxY, int boxW, int boxH);

    static void DrawAlphaText(int x, int y, const char* str, void* font, int r, int g, int b, double alpha);
    static void DrawAlphaShadowText(int x, int y, const char* str, void* font, int r, int g, int b, double alpha, int shadowOffset = 1);
    static void DrawAlphaOutlinedText(int x, int y, const char* str, void* font, int r, int g, int b, double alpha);
    static void Initialize();

    // Core HUD Drawing Methods (Phase 1 Visual Upgrade)
    static void DrawHUD(const Player& player, int score, const char* objectiveText, const char* areaName, double notifyTimer, double areaBannerAlpha = 1.0, const char* chapterName = "THE FALLEN VILLAGE");
    static void DrawAreaBanner(const char* areaName, double alpha, const char* chapterName = "THE FALLEN VILLAGE");
    static void DrawHealthBar(int hp, int maxHp, double displayedHp);
    static void DrawStaminaBar(int stamina, int maxStamina, double displayedStamina);
    static void DrawMissionPanel(const char* objectiveText, const char* areaName, double notifyTimer);
    static void DrawMissionBox(const char* objectiveText, const char* areaName, double notifyTimer);
    static void DrawInventoryHUD(const Player& player, bool hasKeycard, bool ribbonCollected);
    static void DrawInventoryIndicator(const Player& player, bool hasKeycard, bool ribbonCollected);
    static void DrawWeaponDisplay(const char* weaponName, int ammo, int reserveAmmo = 0, bool usesAmmo = false);
    static void DrawAmmoCounter(int ammo, int reserveAmmo, const char* weaponName = NULL);
    static void DrawInteractionPrompt(const char* promptText, int screenX, int screenY);
    static void DrawBossHealthBar(const char* bossName, int bossHp, int bossMaxHp, double displayedHp = -1.0);
    
    // Menu & State Overlays
    static void DrawPauseMenu(int mouseX, int mouseY, bool isMouseDown, double animTime, int pauseSubMenu);
    static void DrawControlsScreen(float alpha = 1.0f);
    static void ResetControlsTexture() { texControlsScreen = 0; }
    static void DrawGameOver(int mouseX, int mouseY, bool isMouseDown, double animTime);
    static void DrawLevelComplete(int mouseX, int mouseY, bool isMouseDown, double animTime);
    static void DrawNotification();

    // Notification Dispatcher
    static void ShowNotification(const char* title, const char* message, double durationSeconds = 3.5);
    static void UpdateNotifications(float dt);
};

#endif // UI_H
