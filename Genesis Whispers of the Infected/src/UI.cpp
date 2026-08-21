#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include "UI.h"
#include "igraphics_declarations.h"
#include <cstdio>
#include <cstring>
#include <cmath>

// Static Texture Handle Definitions
unsigned int UI::texHealthFrame = 0;
unsigned int UI::texHealthFill = 0;
unsigned int UI::texStaminaFrame = 0;
unsigned int UI::texStaminaFill = 0;
unsigned int UI::texMissionBox = 0;
unsigned int UI::texInventoryPanel = 0;
unsigned int UI::texInventorySlot = 0;
unsigned int UI::texPauseOverlay = 0;
unsigned int UI::texGameOverBg = 0;
unsigned int UI::texLevelCompleteBg = 0;
unsigned int UI::texMainMenuBg = 0;
unsigned int UI::texBossFrame = 0;
unsigned int UI::texBossFill = 0;

// Item & HUD Icon Handles
unsigned int UI::texIconMedkit = 0;
unsigned int UI::texIconFood = 0;
unsigned int UI::texIconBattery = 0;
unsigned int UI::texIconScrap = 0;
unsigned int UI::texIconKatana = 0;

NotificationData UI::currentNotification = { "", "", 0.0, 3.5, false };

// ============================================================================
// TYPOGRAPHY HELPERS
// ============================================================================
void UI::DrawShadowText(int x, int y, const char* str, void* font, int r, int g, int b, int shadowOffset) {
    iSetColor(0, 0, 0);
    iText(x + shadowOffset, y - shadowOffset, (char*)str, font);
    iSetColor(r, g, b);
    iText(x, y, (char*)str, font);
}

void UI::DrawOutlinedText(int x, int y, const char* str, void* font, int r, int g, int b) {
    iSetColor(0, 0, 0);
    iText(x + 1, y, (char*)str, font);
    iText(x - 1, y, (char*)str, font);
    iText(x, y + 1, (char*)str, font);
    iText(x, y - 1, (char*)str, font);
    iSetColor(r, g, b);
    iText(x, y, (char*)str, font);
}

void UI::DrawButtonSlot(int slotIdx, int textX, int textY, const char* label, void* font, int mouseX, int mouseY, bool isMouseDown, double animTime) {
    int boxX = 440, boxW = 400;
    int yMin = 420, yMax = 470;
    
    if (slotIdx == 1) { yMin = 430; yMax = 475; }
    else if (slotIdx == 2) { yMin = 375; yMax = 420; }
    else if (slotIdx == 3) { yMin = 320; yMax = 365; }
    else if (slotIdx == 4) { yMin = 265; yMax = 310; }
    else if (slotIdx == 5) { yMin = 210; yMax = 255; }
    else if (slotIdx == 6) { yMin = 155; yMax = 200; }

    bool isHovered = (mouseX >= boxX && mouseX <= boxX + boxW && mouseY >= yMin && mouseY <= yMax);

    // Dark base slot panel
    iSetColor(12, 18, 28);
    iFilledRectangle(boxX, yMin, boxW, yMax - yMin);
    iSetColor(40, 50, 65);
    iRectangle(boxX, yMin, boxW, yMax - yMin);

    if (isHovered) {
        double pulse = 0.8 + 0.2 * sin(animTime * 8.0);
        int gVal = (int)(230 * pulse);
        int bVal = (int)(255 * pulse);

        iSetColor(0, gVal, bVal);
        iRectangle(boxX - 2, yMin - 2, boxW + 4, yMax - yMin + 4);
        iRectangle(boxX - 1, yMin - 1, boxW + 2, yMax - yMin + 2);

        if (isMouseDown) {
            DrawShadowText(textX + 2, textY - 2, label, font, 255, 220, 0);
        } else {
            DrawShadowText(textX, textY, label, font, 0, 240, 255);
        }
    } else {
        DrawShadowText(textX, textY, label, font, 220, 225, 235);
    }
}

// ============================================================================
// INITIALIZATION & ASSET LOADING
// ============================================================================
void UI::Initialize() {
    if (texHealthFrame == 0) {
        texHealthFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Health_Bar_Frame.png").c_str());
        if (texHealthFrame == 0) texHealthFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_health_frame.png").c_str());
    }
    if (texHealthFill == 0) {
        texHealthFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Health_Fill.png").c_str());
        if (texHealthFill == 0) texHealthFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_health_fill.png").c_str());
    }
    if (texStaminaFrame == 0) {
        texStaminaFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Stamina_Bar_Frame.png").c_str());
        if (texStaminaFrame == 0) texStaminaFrame = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_stamina_frame.png").c_str());
    }
    if (texStaminaFill == 0) {
        texStaminaFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/stamina_fill.png").c_str());
        if (texStaminaFill == 0) texStaminaFill = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/ui_stamina_fill.png").c_str());
    }
    if (texMissionBox == 0) {
        texMissionBox = iLoadImage((char*)GetAssetPath("Assets/UI/Mission/mission_update_box.png").c_str());
        if (texMissionBox == 0) texMissionBox = iLoadImage((char*)GetAssetPath("Assets/UI/Mission/ui_mission_box.png").c_str());
    }
    if (texInventoryPanel == 0) {
        texInventoryPanel = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/inventory__panel.png").c_str());
        if (texInventoryPanel == 0) texInventoryPanel = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/ui_inventory_panel.png").c_str());
    }
    if (texInventorySlot == 0) {
        texInventorySlot = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/single_empty_inventory_slot.png").c_str());
        if (texInventorySlot == 0) texInventorySlot = iLoadImage((char*)GetAssetPath("Assets/UI/Inventory/ui_inventory_slot.png").c_str());
    }
    if (texPauseOverlay == 0) {
        texPauseOverlay = iLoadImage((char*)GetAssetPath("Assets/UI/Pause/pause_menu.png").c_str());
        if (texPauseOverlay == 0) texPauseOverlay = iLoadImage((char*)GetAssetPath("Assets/UI/Pause/ui_pause_overlay.png").c_str());
    }
    if (texGameOverBg == 0) {
        texGameOverBg = iLoadImage((char*)GetAssetPath("Assets/UI/Game Over/game_over_screen.png").c_str());
        if (texGameOverBg == 0) texGameOverBg = iLoadImage((char*)GetAssetPath("Assets/UI/Game Over/ui_game_over_screen.png").c_str());
    }
    if (texLevelCompleteBg == 0) {
        texLevelCompleteBg = iLoadImage((char*)GetAssetPath("Assets/UI/Level Complete/level_complete_screen.png").c_str());
        if (texLevelCompleteBg == 0) texLevelCompleteBg = iLoadImage((char*)GetAssetPath("Assets/UI/Level Complete/ui_level_complete_screen.png").c_str());
    }
    if (texMainMenuBg == 0) {
        texMainMenuBg = iLoadImage((char*)GetAssetPath("Assets/UI/Main Menu/new_main_menu.png").c_str());
        if (texMainMenuBg == 0) texMainMenuBg = iLoadImage((char*)GetAssetPath("Assets/UI/Main Menu/main_menu_bg.png").c_str());
    }
    if (texBossFrame == 0) {
        texBossFrame = iLoadImage((char*)GetAssetPath("Assets/UI/Boss/boss_health_bar_frame.png").c_str());
    }
    if (texBossFill == 0) {
        texBossFill = iLoadImage((char*)GetAssetPath("Assets/UI/Boss/boss_health_fill.png").c_str());
    }

    // Item Icons
    if (texIconMedkit == 0) {
        texIconMedkit = iLoadImage((char*)GetAssetPath("Assets/Items/Medicine/first_aid.png").c_str());
    }
    if (texIconFood == 0) {
        texIconFood = iLoadImage((char*)GetAssetPath("Assets/Items/Food/Bread.png").c_str());
    }
    if (texIconBattery == 0) {
        texIconBattery = iLoadImage((char*)GetAssetPath("Assets/Items/Food/water_bottle.png").c_str());
    }
    if (texIconScrap == 0) {
        texIconScrap = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/Scrap_Metal.png").c_str());
    }
}

// ============================================================================
// HUD DRAWING ROUTINES (PHASE 1 VISUAL UPGRADE)
// ============================================================================
void UI::DrawHUD(const Player& player, int score, const char* objectiveText, const char* areaName, double notifyTimer, double areaBannerAlpha) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Upper-Left: Health & Stamina Bars
    DrawHealthBar(player.hp, player.maxHp, player.displayedHp);
    DrawStaminaBar(player.stamina, player.maxStamina, player.displayedStamina);

    // 2. Upper-Right: Mission Objective Panel
    DrawMissionPanel(objectiveText, areaName, notifyTimer);

    // 3. Lower-Right: Weapon Display Panel
    DrawWeaponDisplay("KATANA", player.ammo, false);

    // 4. Lower-Left: Icon-Based Survival Inventory Display
    DrawInventoryHUD(player, false, false);

    // 5. Score Banner (Upper Right Header)
    iSetColor(0, 0, 0);
    iFilledRectangle(948, 578, 314, 34);
    iSetColor(12, 16, 24);
    iFilledRectangle(950, 580, 310, 30);
    iSetColor(0, 180, 220);
    iRectangle(950, 580, 310, 30);
    
    char scoreStr[48];
    sprintf_s(scoreStr, sizeof(scoreStr), "SCORE: %07d | [H] HEAL", score);
    DrawShadowText(965, 588, scoreStr, GLUT_BITMAP_HELVETICA_12, 255, 215, 0);

    // 6. Sleek Cinematic Area & Village Title Banner
    DrawAreaBanner(areaName, areaBannerAlpha);
}

void UI::DrawAreaBanner(const char* areaName, double alpha) {
    if (alpha <= 0.01 || !areaName) return;

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

// ----------------------------------------------------------------------------
// 1. HEALTH BAR (PERFECT INSIDE SPEAR FRAME ALIGNMENT)
// ----------------------------------------------------------------------------
void UI::DrawHealthBar(int hp, int maxHp, double displayedHp) {
    int startX = 20;
    int startY = 638;
    int frameW = 420;
    int frameH = 34;

    double hpRatio = (double)hp / (double)maxHp;
    if (hpRatio < 0.0) hpRatio = 0.0;
    if (hpRatio > 1.0) hpRatio = 1.0;

    // Header Text placed cleanly ABOVE the spiked frame
    DrawShadowText(startX + 10, startY + frameH + 6, "VITAL STATUS", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    char hpStr[32];
    int pct = (int)(((double)hp / maxHp) * 100.0);
    sprintf_s(hpStr, sizeof(hpStr), "HP %d/%d (%d%%)", hp, maxHp, pct);
    DrawShadowText(startX + 260, startY + frameH + 6, hpStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    // STEP 1: Draw empty metal frame base
    if (texHealthFrame != 0) {
        iShowImage(startX, startY, frameW, frameH, texHealthFrame);
    }

    // STEP 2: Calculate fill width and inner clipping area (Centered 6px line inside spear channel)
    int innerBarWidth = 312;
    int innerBarHeight = 6;
    int healthFillWidth = (int)(innerBarWidth * hpRatio);

    int fillX = startX + 54;
    int fillY = startY + 14;

    // STEP 3 & 4: Draw red fill INSIDE clipping rectangle ONLY
    if (healthFillWidth > 0) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(fillX, fillY, healthFillWidth, innerBarHeight);

        // Base Vibrant Red Fill
        iSetColor(220, 25, 25);
        iFilledRectangle(fillX, fillY, healthFillWidth, innerBarHeight);
        
        // Top Highlight Streak
        iSetColor(255, 120, 120);
        iFilledRectangle(fillX, fillY + innerBarHeight - 2, healthFillWidth, 2);

        if (texHealthFill != 0) {
            iShowImageSub(fillX, fillY, healthFillWidth, innerBarHeight, texHealthFill, 0.0, 0.0, hpRatio, 1.0);
        }

        glDisable(GL_SCISSOR_TEST);
    }

    // STEP 5: Draw metal frame again as final overlay
    if (texHealthFrame != 0) {
        iShowImage(startX, startY, frameW, frameH, texHealthFrame);
    } else {
        iSetColor(40, 50, 64);
        iRectangle(startX, startY, frameW, frameH);
        iSetColor(65, 78, 98);
        iRectangle(startX + 1, startY + 1, frameW - 2, frameH - 2);
    }
}

// ----------------------------------------------------------------------------
// 2. STAMINA BAR (PERFECT INSIDE SPEAR FRAME ALIGNMENT)
// ----------------------------------------------------------------------------
void UI::DrawStaminaBar(int stamina, int maxStamina, double displayedStamina) {
    int startX = 20;
    int startY = 560;
    int frameW = 420;
    int frameH = 34;

    double staminaRatio = (double)stamina / (double)maxStamina;
    if (staminaRatio < 0.0) staminaRatio = 0.0;
    if (staminaRatio > 1.0) staminaRatio = 1.0;

    // Header Text placed cleanly ABOVE the spiked frame
    DrawShadowText(startX + 10, startY + frameH + 6, "ENERGY STATUS", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    char stmStr[32];
    int pct = (int)(((double)stamina / maxStamina) * 100.0);
    sprintf_s(stmStr, sizeof(stmStr), "STAMINA %d%%", pct);
    DrawShadowText(startX + 280, startY + frameH + 6, stmStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    // STEP 1: Draw empty metal frame base
    if (texStaminaFrame != 0) {
        iShowImage(startX, startY, frameW, frameH, texStaminaFrame);
    }

    // STEP 2: Calculate fill width and inner clipping area (Centered 6px line inside spear channel)
    int innerBarWidth = 312;
    int innerBarHeight = 6;
    int staminaFillWidth = (int)(innerBarWidth * staminaRatio);

    int fillX = startX + 54;
    int fillY = startY + 14;

    // STEP 3 & 4: Draw gold fill INSIDE clipping rectangle ONLY
    if (staminaFillWidth > 0) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(fillX, fillY, staminaFillWidth, innerBarHeight);

        // Base Deep Gold Fill
        iSetColor(240, 175, 15);
        iFilledRectangle(fillX, fillY, staminaFillWidth, innerBarHeight);

        // Top Highlight Streak
        iSetColor(255, 220, 50);
        iFilledRectangle(fillX, fillY + innerBarHeight - 2, staminaFillWidth, 2);

        if (texStaminaFill != 0) {
            iShowImageSub(fillX, fillY, staminaFillWidth, innerBarHeight, texStaminaFill, 0.0, 0.0, staminaRatio, 1.0);
        }

        glDisable(GL_SCISSOR_TEST);
    }

    // STEP 5: Draw metal frame again as final overlay
    if (texStaminaFrame != 0) {
        iShowImage(startX, startY, frameW, frameH, texStaminaFrame);
    } else {
        iSetColor(40, 50, 64);
        iRectangle(startX, startY, frameW, frameH);
        iSetColor(65, 78, 98);
        iRectangle(startX + 1, startY + 1, frameW - 2, frameH - 2);
    }
}

// ----------------------------------------------------------------------------
// 3. MISSION PANEL UPGRADE
// ----------------------------------------------------------------------------
void UI::DrawMissionPanel(const char* objectiveText, const char* areaName, double notifyTimer) {
    int missX = 940;
    int missY = 620;
    int missW = 320;
    int missH = 80;

    if (texMissionBox != 0) {
        iShowImage(missX, missY, missW, missH, texMissionBox);
    } else {
        iSetColor(10, 14, 22);
        iFilledRectangle(missX, missY, missW, missH);
        iSetColor(0, 180, 220);
        iRectangle(missX, missY, missW, missH);
        iSetColor(0, 140, 180);
        iRectangle(missX + 2, missY + 2, missW - 4, missH - 4);
    }

    // Small Mission Reticle Icon [!] on top left
    iSetColor(0, 240, 255);
    iRectangle(missX + 12, missY + missH - 22, 12, 12);
    iSetColor(255, 215, 0);
    iFilledCircle(missX + 18, missY + missH - 16, 3);

    // Animated glow pulse on objective update
    if (notifyTimer > 0.0) {
        double objPulse = 0.7 + 0.3 * sin(notifyTimer * 10.0);
        int gCol = (int)(255 * objPulse);
        int bCol = (int)(255 * objPulse);

        iSetColor(0, gCol, bCol);
        iRectangle(missX - 2, missY - 2, missW + 4, missH + 4);
        DrawOutlinedText(missX + 32, missY + missH - 22, "MISSION DIRECTIVE (UPDATED)", GLUT_BITMAP_HELVETICA_10, 0, 255, 220);
        DrawShadowText(missX + 14, missY + 20, objectiveText, GLUT_BITMAP_HELVETICA_12, 255, 220, 0);
    } else {
        DrawOutlinedText(missX + 32, missY + missH - 22, "MISSION DIRECTIVE", GLUT_BITMAP_HELVETICA_10, 0, 220, 255);
        
        char objFormattedStr[128];
        sprintf_s(objFormattedStr, sizeof(objFormattedStr), "Objective: %s", objectiveText ? objectiveText : "Escape the Fallen Village");
        DrawShadowText(missX + 14, missY + 20, objFormattedStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
    }
}

void UI::DrawMissionBox(const char* objectiveText, const char* areaName, double notifyTimer) {
    DrawMissionPanel(objectiveText, areaName, notifyTimer);
}

// ----------------------------------------------------------------------------
// 4. INVENTORY HUD UPGRADE
// ----------------------------------------------------------------------------
void UI::DrawInventoryHUD(const Player& player, bool hasKeycard, bool ribbonCollected) {
    // Positioned in Bottom-Left Position (Phase 1 Requirement 4)
    int startX = 20;
    int startY = 20;
    int barW = 360;
    int barH = 58;

    iSetColor(0, 0, 0);
    iFilledRectangle(startX - 2, startY - 2, barW + 4, barH + 4);
    iSetColor(10, 14, 22);
    iFilledRectangle(startX, startY, barW, barH);
    iSetColor(0, 180, 220);
    iRectangle(startX, startY, barW, barH);

    DrawOutlinedText(startX + 12, startY + barH - 18, "SURVIVAL SUPPLIES  [TAB]", GLUT_BITMAP_HELVETICA_10, 0, 220, 255);

    // Render 4 Item Columns: [Medkit Icon] x2, [Food Icon] x1, [Battery Icon] x1, [Scrap Icon] x1
    int colX[4] = { startX + 12, startX + 100, startX + 188, startX + 276 };
    int iconY = startY + 10;

    // 1. Medkit Icon & Count
    if (texIconMedkit != 0) {
        iShowImage(colX[0], iconY, 22, 22, texIconMedkit);
    } else {
        iSetColor(180, 20, 20);
        iFilledRectangle(colX[0], iconY, 20, 20);
        iSetColor(255, 255, 255);
        iFilledRectangle(colX[0] + 8, iconY + 3, 4, 14);
        iFilledRectangle(colX[0] + 3, iconY + 8, 14, 4);
    }
    char medStr[16];
    sprintf_s(medStr, sizeof(medStr), "x%d", player.medkits);
    DrawShadowText(colX[0] + 26, iconY + 5, medStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    // 2. Food Icon & Count
    if (texIconFood != 0) {
        iShowImage(colX[1], iconY, 22, 22, texIconFood);
    } else {
        iSetColor(160, 100, 30);
        iFilledRectangle(colX[1], iconY, 20, 20);
        iSetColor(240, 180, 40);
        iRectangle(colX[1], iconY, 20, 20);
    }
    char foodStr[16];
    sprintf_s(foodStr, sizeof(foodStr), "x%d", player.foodCount);
    DrawShadowText(colX[1] + 26, iconY + 5, foodStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    // 3. Battery Icon & Count
    if (texIconBattery != 0) {
        iShowImage(colX[2], iconY, 22, 22, texIconBattery);
    } else {
        iSetColor(30, 30, 40);
        iFilledRectangle(colX[2], iconY, 20, 20);
        iSetColor(0, 255, 180);
        iRectangle(colX[2], iconY, 20, 20);
        iFilledRectangle(colX[2] + 5, iconY + 4, 10, 12);
    }
    char batStr[16];
    sprintf_s(batStr, sizeof(batStr), "x%d", player.batteryCount);
    DrawShadowText(colX[2] + 26, iconY + 5, batStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    // 4. Scrap Icon & Count
    if (texIconScrap != 0) {
        iShowImage(colX[3], iconY, 22, 22, texIconScrap);
    } else {
        iSetColor(50, 55, 65);
        iFilledRectangle(colX[3], iconY, 20, 20);
        iSetColor(200, 210, 220);
        iRectangle(colX[3], iconY, 20, 20);
    }
    char scrapStr[16];
    sprintf_s(scrapStr, sizeof(scrapStr), "x%d", player.scrapCount);
    DrawShadowText(colX[3] + 26, iconY + 5, scrapStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
}

void UI::DrawInventoryIndicator(const Player& player, bool hasKeycard, bool ribbonCollected) {
    DrawInventoryHUD(player, hasKeycard, ribbonCollected);
}

// ----------------------------------------------------------------------------
// 5. WEAPON DISPLAY UPGRADE
// ----------------------------------------------------------------------------
void UI::DrawWeaponDisplay(const char* weaponName, int ammo, bool usesAmmo) {
    // Positioned in Bottom-Right Position (Phase 1 Requirement 5)
    int boxX = 1040;
    int boxY = 20;
    int boxW = 220;
    int boxH = 75;

    iSetColor(0, 0, 0);
    iFilledRectangle(boxX - 2, boxY - 2, boxW + 4, boxH + 4);
    iSetColor(10, 14, 22);
    iFilledRectangle(boxX, boxY, boxW, boxH);
    iSetColor(0, 180, 220);
    iRectangle(boxX, boxY, boxW, boxH);

    // Katana / Weapon Icon (Left side of panel)
    int iconX = boxX + 12;
    int iconY = boxY + 22;

    if (texIconKatana != 0) {
        iShowImage(iconX, iconY, 32, 32, texIconKatana);
    } else {
        // Sleek Katana Blade Vector Icon
        iSetColor(220, 230, 245); // Silver Katana Blade
        iLine(iconX + 2, iconY + 4, iconX + 26, iconY + 28);
        iLine(iconX + 3, iconY + 3, iconX + 27, iconY + 27);
        iSetColor(255, 215, 0); // Gold Tsuba (Guard)
        iFilledCircle(iconX + 9, iconY + 11, 4);
        iSetColor(180, 30, 30); // Red Wrapped Tsuka (Handle)
        iLine(iconX + 2, iconY + 4, iconX + 9, iconY + 11);
    }

    // Weapon Name & Type Text
    DrawOutlinedText(boxX + 48, boxY + boxH - 22, weaponName ? weaponName : "KATANA", GLUT_BITMAP_HELVETICA_18, 255, 255, 255);

    if (usesAmmo) {
        DrawAmmoCounter(ammo, 48);
    } else {
        DrawShadowText(boxX + 48, boxY + 16, "MELEE WEAPON [J]", GLUT_BITMAP_HELVETICA_10, 0, 220, 255);
    }
}

void UI::DrawAmmoCounter(int ammo, int reserveAmmo) {
    int boxX = 1040;
    int boxY = 20;

    char ammoStr[32];
    sprintf_s(ammoStr, sizeof(ammoStr), "AMMO %d / %d", ammo, reserveAmmo);
    DrawShadowText(boxX + 48, boxY + 16, ammoStr, GLUT_BITMAP_HELVETICA_12, 255, 215, 0);
}

void UI::DrawInteractionPrompt(const char* promptText, int screenX, int screenY) {
    if (!promptText || strlen(promptText) == 0) return;

    int textLen = (int)strlen(promptText);
    int boxW = textLen * 9 + 24;
    int boxH = 32;
    int boxX = screenX - (boxW / 2);
    int boxY = screenY;

    iSetColor(8, 12, 20);
    iFilledRectangle(boxX, boxY, boxW, boxH);
    iSetColor(0, 240, 255);
    iRectangle(boxX, boxY, boxW, boxH);
    iSetColor(255, 215, 0);
    iRectangle(boxX + 2, boxY + 2, boxW - 4, boxH - 4);

    DrawShadowText(boxX + 12, boxY + 10, promptText, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
}

// ============================================================================
// BOSS HEALTH BAR
// ============================================================================
void UI::DrawBossHealthBar(const char* bossName, int bossHp, int bossMaxHp) {
    int barW = 600;
    int barH = 48;
    int barX = 640 - (barW / 2);
    int barY = 645;

    double bossHpPercent = (double)bossHp / bossMaxHp;
    if (bossHpPercent < 0.0) bossHpPercent = 0.0;
    if (bossHpPercent > 1.0) bossHpPercent = 1.0;

    iSetColor(0, 0, 0);
    iFilledRectangle(barX - 4, barY - 4, barW + 8, barH + 8);
    iSetColor(15, 15, 22);
    iFilledRectangle(barX, barY, barW, barH);
    iSetColor(220, 40, 40);
    iRectangle(barX, barY, barW, barH);

    iSetColor(40, 10, 10);
    iFilledRectangle(barX + 10, barY + 10, barW - 20, 16);

    if (texBossFill != 0) {
        int fillW = (int)((barW - 20) * bossHpPercent);
        if (fillW > 0) {
            iShowImageSub(barX + 10, barY + 10, fillW, 16, texBossFill, 0.0, 0.0, bossHpPercent, 1.0);
        }
    } else {
        iSetColor(220, 30, 30);
        iFilledRectangle(barX + 10, barY + 10, (int)((barW - 20) * bossHpPercent), 16);
    }

    if (texBossFrame != 0) {
        iShowImage(barX, barY, barW, barH, texBossFrame);
    }

    int nameLen = (int)strlen(bossName);
    DrawOutlinedText(640 - (nameLen * 4), barY + 30, bossName, GLUT_BITMAP_HELVETICA_12, 255, 230, 230);
}

// ============================================================================
// PAUSE MENU
// ============================================================================
void UI::DrawPauseMenu(int mouseX, int mouseY, bool isMouseDown, double animTime, int pauseSubMenu) {
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, 1280, 720);

    if (texPauseOverlay != 0) {
        iShowImage(240, 90, 800, 540, texPauseOverlay);
    } else {
        iSetColor(12, 16, 24);
        iFilledRectangle(360, 120, 560, 480);
        iSetColor(0, 180, 220);
        iRectangle(360, 120, 560, 480);
    }

    DrawOutlinedText(545, 545, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24, 0, 230, 255);

    if (pauseSubMenu == 0) {
        DrawButtonSlot(1, 525, 442, "1. RESUME GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(2, 535, 387, "2. INVENTORY [TAB]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(3, 545, 332, "3. CONTROLS [C]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(4, 550, 277, "4. SETTINGS [S]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(5, 545, 222, "5. RESTART LEVEL", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(6, 530, 167, "6. QUIT TO MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);

        DrawShadowText(480, 125, "Press ESC to Resume or Click Options to Select", GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
    }
    else if (pauseSubMenu == 1) {
        iSetColor(18, 24, 36);
        iFilledRectangle(400, 160, 480, 360);
        iSetColor(0, 220, 255);
        iRectangle(400, 160, 480, 360);

        DrawOutlinedText(560, 480, "GAME CONTROLS", GLUT_BITMAP_HELVETICA_18, 0, 240, 255);
        DrawShadowText(430, 430, "A / D or LEFT / RIGHT  - Move Character", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(430, 390, "W / SPACE / UP         - Jump", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(430, 350, "J / LEFT CLICK         - Katana Slash", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(430, 310, "K / RIGHT CLICK        - Ranged Attack", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(430, 270, "E                      - Interact / Pick Up", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(430, 230, "H                      - Use First Aid Kit", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(430, 190, "TAB / I                - Open Inventory", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

        DrawShadowText(520, 130, "Press ESC or Click to Return", GLUT_BITMAP_HELVETICA_12, 255, 215, 0);
    }
    else if (pauseSubMenu == 2) {
        iSetColor(18, 24, 36);
        iFilledRectangle(400, 160, 480, 360);
        iSetColor(0, 220, 255);
        iRectangle(400, 160, 480, 360);

        DrawOutlinedText(555, 480, "AUDIO & DISPLAY", GLUT_BITMAP_HELVETICA_18, 0, 240, 255);
        DrawShadowText(440, 410, "Resolution        : 1280 x 720 (Native)", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(440, 360, "Display Mode      : Windowed", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(440, 310, "Master Volume     : [==========] 100%", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
        DrawShadowText(440, 260, "SFX & Music       : ENABLED", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

        DrawShadowText(520, 130, "Press ESC or Click to Return", GLUT_BITMAP_HELVETICA_12, 255, 215, 0);
    }
}

// ============================================================================
// GAME OVER
// ============================================================================
void UI::DrawGameOver(int mouseX, int mouseY, bool isMouseDown, double animTime) {
    if (texGameOverBg != 0) {
        iShowImage(0, 0, 1280, 720, texGameOverBg);
    } else {
        iSetColor(15, 5, 5);
        iFilledRectangle(0, 0, 1280, 720);
    }

    DrawOutlinedText(540, 545, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24, 240, 30, 30);
    DrawShadowText(515, 505, "THE FALLEN VILLAGE", GLUT_BITMAP_HELVETICA_18, 200, 200, 200);

    DrawButtonSlot(1, 515, 442, "1. RETRY LEVEL [ENTER]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
    DrawButtonSlot(2, 530, 387, "2. MAIN MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
    DrawButtonSlot(3, 535, 332, "3. EXIT GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);

    DrawShadowText(470, 148, "Press [ENTER], [M], [ESC] or Click Options to Select", GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
}

// ============================================================================
// LEVEL COMPLETE
// ============================================================================
void UI::DrawLevelComplete(int mouseX, int mouseY, bool isMouseDown, double animTime) {
    if (texLevelCompleteBg != 0) {
        iShowImage(0, 0, 1280, 720, texLevelCompleteBg);
    } else {
        iSetColor(5, 15, 10);
        iFilledRectangle(0, 0, 1280, 720);
    }

    DrawOutlinedText(515, 545, "LEVEL COMPLETE", GLUT_BITMAP_TIMES_ROMAN_24, 0, 255, 120);
    DrawShadowText(525, 505, "THE FALLEN VILLAGE", GLUT_BITMAP_HELVETICA_18, 200, 200, 200);
    DrawOutlinedText(485, 475, "Mission Updated: REACH BLACKWOOD FOREST", GLUT_BITMAP_HELVETICA_12, 0, 230, 255);

    DrawButtonSlot(1, 515, 412, "1. CONTINUE [ENTER]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
    DrawButtonSlot(2, 530, 357, "2. MAIN MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
    DrawButtonSlot(3, 535, 302, "3. EXIT GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);

    DrawShadowText(470, 148, "Press [ENTER], [M], [ESC] or Click Options to Select", GLUT_BITMAP_HELVETICA_12, 200, 210, 220);
}

// ============================================================================
// STORY UI NOTIFICATION SYSTEM
// ============================================================================
void UI::ShowNotification(const char* title, const char* message, double durationSeconds) {
    currentNotification.title = title ? title : "";
    currentNotification.message = message ? message : "";
    currentNotification.timer = durationSeconds;
    currentNotification.maxTimer = durationSeconds;
    currentNotification.active = true;
}

void UI::UpdateNotifications(float dt) {
    if (currentNotification.active) {
        currentNotification.timer -= dt;
        if (currentNotification.timer <= 0.0) {
            currentNotification.active = false;
        }
    }
}

void UI::DrawNotification() {
    if (!currentNotification.active || currentNotification.timer <= 0.0) return;

    double alpha = 1.0;
    if (currentNotification.timer > currentNotification.maxTimer - 0.5) {
        alpha = (currentNotification.maxTimer - currentNotification.timer) / 0.5;
    } else if (currentNotification.timer < 0.8) {
        alpha = currentNotification.timer / 0.8;
    }

    if (alpha <= 0.01) return;

    int boxW = 420;
    int boxH = 55;
    int boxX = 640 - (boxW / 2);
    int boxY = 640;

    iSetColor(10, 14, 22);
    iFilledRectangle(boxX, boxY, boxW, boxH);
    iSetColor(0, 220, 255);
    iRectangle(boxX, boxY, boxW, boxH);
    iSetColor(255, 215, 0);
    iRectangle(boxX + 2, boxY + 2, boxW - 4, boxH - 4);

    int titleLen = (int)currentNotification.title.length();
    DrawOutlinedText(640 - (titleLen * 4), boxY + 34, currentNotification.title.c_str(), GLUT_BITMAP_HELVETICA_10, 0, 240, 255);

    int msgLen = (int)currentNotification.message.length();
    DrawShadowText(640 - (msgLen * 4), boxY + 12, currentNotification.message.c_str(), GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
}
