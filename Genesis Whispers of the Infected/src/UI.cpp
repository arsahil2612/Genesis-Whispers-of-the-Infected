#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include "UI.h"
#include "ResourceManager.h"
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
unsigned int UI::texScoreLabel = 0;
unsigned int UI::texSeparator = 0;
unsigned int UI::texHealLabel = 0;
unsigned int UI::texScoreHeal = 0;

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

static int GetGlutStringWidth(void* font, const char* str) {
    if (!str) return 0;
    int w = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        w += glutBitmapWidth(font, (unsigned char)str[i]);
    }
    return w;
}

static void RenderButtonLabelWithShortcut(int boxX, int boxW, int targetCenterY, const char* label, void* font, bool isHovered, bool isMouseDown) {
    std::string fullStr(label);
    size_t bracketPos = fullStr.find('[');
    
    int labelWidth = GetGlutStringWidth(font, label);
    int startX = boxX + (boxW - labelWidth) / 2;
    int renderY = targetCenterY;

    if (isMouseDown) {
        startX += 1;
        renderY -= 1;
    }

    if (bracketPos != std::string::npos) {
        std::string mainText = fullStr.substr(0, bracketPos);
        std::string shortcutText = fullStr.substr(bracketPos);

        // Dark inset shadow line
        iSetColor(8, 10, 14);
        iText(startX + 1, renderY - 1, (char*)mainText.c_str(), font);

        // White metallic main text
        if (isHovered) {
            iSetColor(255, 255, 255);
        } else {
            iSetColor(215, 220, 225);
        }
        iText(startX, renderY, (char*)mainText.c_str(), font);

        // Calculate offset for shortcut text
        int mainWidth = GetGlutStringWidth(font, mainText.c_str());
        int shortcutX = startX + mainWidth;

        // Shadow offset for shortcut
        iSetColor(8, 10, 14);
        iText(shortcutX + 1, renderY - 1, (char*)shortcutText.c_str(), font);

        // Cyan highlight ONLY for keyboard shortcuts
        if (isHovered) {
            iSetColor(0, 230, 255);
        } else {
            iSetColor(0, 180, 215);
        }
        iText(shortcutX, renderY, (char*)shortcutText.c_str(), font);
    } else {
        // No shortcut
        iSetColor(8, 10, 14);
        iText(startX + 1, renderY - 1, (char*)label, font);

        if (isHovered) {
            iSetColor(255, 255, 255);
        } else {
            iSetColor(215, 220, 225);
        }
        iText(startX, renderY, (char*)label, font);
    }
}

void UI::DrawButtonSlot(int slotIdx, int textX, int textY, const char* label, void* font, int mouseX, int mouseY, bool isMouseDown, double animTime) {
    int boxW = 340;               // Reduced width by 15% (340px) leaving visible space from frame edges
    int boxX = (1280 - boxW) / 2; // 470 (Horizontally centered: buttonCenterX = 640)
    int yMin = 233, yMax = 275;
    
    // Compact vertical layout: 42px height + 8px uniform gap (50px stride)
    if (slotIdx == 1) { yMin = 483; yMax = 525; }
    else if (slotIdx == 2) { yMin = 433; yMax = 475; }
    else if (slotIdx == 3) { yMin = 383; yMax = 425; }
    else if (slotIdx == 4) { yMin = 333; yMax = 375; }
    else if (slotIdx == 5) { yMin = 283; yMax = 325; }
    else if (slotIdx == 6) { yMin = 233; yMax = 275; }

    int boxH = yMax - yMin; // 42px
    bool isHovered = (mouseX >= boxX && mouseX <= boxX + boxW && mouseY >= yMin && mouseY <= yMax);

    // 1. Dark steel button backing plate
    iSetColor(12, 14, 18);
    iFilledRectangle(boxX, yMin, boxW, boxH);

    // 2. Scratched gunmetal inner plate
    if (isHovered) {
        iSetColor(38, 46, 58); // Metallic highlight when hovered
    } else {
        iSetColor(24, 28, 35);
    }
    iFilledRectangle(boxX + 2, yMin + 2, boxW - 4, boxH - 4);

    // 3. Rusted weathered bevel & outer border
    if (isHovered) {
        // Selected / Hover state: brighter metal edge with subtle orange rust highlight
        iSetColor(160, 175, 190);
        iRectangle(boxX, yMin, boxW, boxH);
        iSetColor(200, 115, 25); // Subtle orange rust glow line
        iRectangle(boxX + 1, yMin + 1, boxW - 2, boxH - 2);
    } else {
        // Normal state: dark rusted steel border
        iSetColor(64, 70, 80);
        iRectangle(boxX, yMin, boxW, boxH);
        iSetColor(38, 42, 50);
        iRectangle(boxX + 1, yMin + 1, boxW - 2, boxH - 2);
    }

    // 4. Post-apocalyptic rust spots & metal scratches
    iSetColor(125, 55, 18);
    iFilledRectangle(boxX + 8, yMin + 2, 24, 2);
    iFilledRectangle(boxX + boxW - 32, yMin + boxH - 4, 24, 2);

    // 5. Corner metallic hex bolts / rivets
    int boltColor = isHovered ? 175 : 135;
    iSetColor(boltColor, boltColor - 5, boltColor - 10);
    iFilledRectangle(boxX + 5, yMin + boxH - 7, 4, 4);
    iFilledRectangle(boxX + boxW - 9, yMin + boxH - 7, 4, 4);
    iFilledRectangle(boxX + 5, yMin + 3, 4, 4);
    iFilledRectangle(boxX + boxW - 9, yMin + 3, 4, 4);

    iSetColor(15, 16, 18);
    iRectangle(boxX + 5, yMin + boxH - 7, 4, 4);
    iRectangle(boxX + boxW - 9, yMin + boxH - 7, 4, 4);
    iRectangle(boxX + 5, yMin + 3, 4, 4);
    iRectangle(boxX + boxW - 9, yMin + 3, 4, 4);

    // 6. Draw Text Label & Shortcut (Exact Vertical & Horizontal Centering)
    int targetCenterY = yMin + (boxH - 18) / 2 + 3;
    RenderButtonLabelWithShortcut(boxX, boxW, targetCenterY, label, font, isHovered, isMouseDown);
}

// ============================================================================
// PAUSE MENU
// ============================================================================
void UI::DrawPauseMenu(int mouseX, int mouseY, bool isMouseDown, double animTime, int pauseSubMenu) {
    // 1. Darkened gameplay backdrop dimmer
    iSetColor(0, 0, 0);
    iFilledRectangle(0, 0, 1280, 720);

    int frameX = 360, frameY = 120, frameW = 560, frameH = 480;

    // 2. Render Existing Pause Menu Base Frame Asset
    if (texPauseOverlay != 0) {
        iShowImage(240, 90, 800, 540, texPauseOverlay);
    } else {
        // Procedural Rusted Dark Steel Base Frame (Fallback when PNG overlay not loaded)
        iSetColor(14, 16, 20);
        iFilledRectangle(frameX, frameY, frameW, frameH);
        iSetColor(52, 58, 68);
        iRectangle(frameX, frameY, frameW, frameH);
        iSetColor(32, 36, 44);
        iRectangle(frameX + 1, frameY + 1, frameW - 2, frameH - 2);

        // Rust corner details on base frame
        iSetColor(115, 50, 15);
        iFilledRectangle(frameX + 4, frameY + 4, 18, 2);
        iFilledRectangle(frameX + frameW - 22, frameY + 4, 18, 2);
        iFilledRectangle(frameX + 4, frameY + frameH - 6, 18, 2);
        iFilledRectangle(frameX + frameW - 22, frameY + frameH - 6, 18, 2);
    }

    // 3. ENGRAVED TITLE: "PAUSED" (Centering calculation for 100% alignment)
    int titleW = GetGlutStringWidth(GLUT_BITMAP_TIMES_ROMAN_24, "PAUSED");
    int titleX = 640 - titleW / 2;
    int titleY = 544;

    // Dark inset engraved shadow
    iSetColor(10, 8, 6);
    iText(titleX + 1, titleY - 1, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    iText(titleX + 2, titleY - 2, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

    // Weathered silver / dirty white engraved title text
    iSetColor(215, 220, 225);
    iText(titleX, titleY, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

    // 4. Render Menu Buttons & Sub-menus
    if (pauseSubMenu == 0) {
        DrawButtonSlot(1, 470, 504, "1. RESUME GAME [ESC]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(2, 470, 454, "2. INVENTORY [TAB]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(3, 470, 404, "3. CONTROLS [C]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(4, 470, 354, "4. SETTINGS [S]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(5, 470, 304, "5. RESTART LEVEL", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);
        DrawButtonSlot(6, 470, 254, "6. QUIT TO MENU [M]", GLUT_BITMAP_HELVETICA_18, mouseX, mouseY, isMouseDown, animTime);

        // 5. ENGRAVED BOTTOM INSTRUCTION METAL LABEL (Slightly taller & 10px below Button 6)
        int lblW = 340;
        int lblH = 30;
        int lblX = (1280 - lblW) / 2; // 470 (Centered)
        int lblY = 193;              // 10px directly below Button 6 (yMin = 233)

        // Dark steel backing panel
        iSetColor(12, 14, 18);
        iFilledRectangle(lblX, lblY, lblW, lblH);

        // Scratched gunmetal inner plate fill
        iSetColor(24, 28, 35);
        iFilledRectangle(lblX + 2, lblY + 2, lblW - 4, lblH - 4);

        // Rusted iron borders
        iSetColor(58, 64, 74);
        iRectangle(lblX, lblY, lblW, lblH);
        iSetColor(36, 40, 48);
        iRectangle(lblX + 1, lblY + 1, lblW - 2, lblH - 2);

        // Corner rivets
        iSetColor(130, 125, 115);
        iFilledRectangle(lblX + 4, lblY + lblH - 5, 3, 3);
        iFilledRectangle(lblX + lblW - 7, lblY + lblH - 5, 3, 3);
        iFilledRectangle(lblX + 4, lblY + 3, 3, 3);
        iFilledRectangle(lblX + lblW - 7, lblY + 3, 3, 3);

        // Centered instruction text inside label
        const char* msgText = "Press ESC to Resume or Click Options to Select";
        int msgW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_12, msgText);
        int msgX = 640 - msgW / 2;
        int msgY = lblY + 10;

        iSetColor(8, 10, 12);
        iText(msgX + 1, msgY - 1, (char*)msgText, GLUT_BITMAP_HELVETICA_12);
        iSetColor(190, 195, 205);
        iText(msgX, msgY, (char*)msgText, GLUT_BITMAP_HELVETICA_12);
    }
    else if (pauseSubMenu == 1) {
        int subW = 480, subH = 360;
        int subX = (1280 - subW) / 2; // 400
        int subY = 160;

        iSetColor(14, 18, 24);
        iFilledRectangle(subX, subY, subW, subH);
        iSetColor(54, 62, 72);
        iRectangle(subX, subY, subW, subH);

        const char* headStr = "GAME CONTROLS";
        int headW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_18, headStr);
        int headX = 640 - headW / 2;
        iSetColor(220, 225, 230);
        iText(headX, 485, (char*)headStr, GLUT_BITMAP_HELVETICA_18);

        DrawShadowText(430, 430, "A / D or LEFT / RIGHT  - Move Character", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(430, 390, "W / SPACE / UP         - Jump", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(430, 350, "J / LEFT CLICK         - Katana Slash", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(430, 310, "K / RIGHT CLICK        - Ranged Attack", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(430, 270, "E                      - Interact / Pick Up", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(430, 230, "H                      - Use First Aid Kit", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(430, 190, "TAB / I                - Open Inventory", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);

        const char* retStr = "Press ESC or Click to Return";
        int retW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_12, retStr);
        int retX = 640 - retW / 2;
        DrawShadowText(retX, 180, retStr, GLUT_BITMAP_HELVETICA_12, 220, 180, 50);
    }
    else if (pauseSubMenu == 2) {
        int subW = 480, subH = 360;
        int subX = (1280 - subW) / 2; // 400
        int subY = 160;

        iSetColor(14, 18, 24);
        iFilledRectangle(subX, subY, subW, subH);
        iSetColor(54, 62, 72);
        iRectangle(subX, subY, subW, subH);

        const char* headStr = "AUDIO & DISPLAY";
        int headW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_18, headStr);
        int headX = 640 - headW / 2;
        iSetColor(220, 225, 230);
        iText(headX, 485, (char*)headStr, GLUT_BITMAP_HELVETICA_18);

        DrawShadowText(440, 410, "Resolution        : 1280 x 720 (Native)", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(440, 360, "Display Mode      : Windowed", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(440, 310, "Master Volume     : [==========] 100%", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);
        DrawShadowText(440, 260, "SFX & Music       : ENABLED", GLUT_BITMAP_HELVETICA_12, 220, 225, 230);

        const char* retStr = "Press ESC or Click to Return";
        int retW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_12, retStr);
        int retX = 640 - retW / 2;
        DrawShadowText(retX, 180, retStr, GLUT_BITMAP_HELVETICA_12, 220, 180, 50);
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
    if (texScoreLabel == 0) {
        texScoreLabel = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/score_label.png").c_str());
        if (texScoreLabel == 0) texScoreLabel = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/score_label.bmp").c_str());
    }
    if (texSeparator == 0) {
        texSeparator = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/separator.png").c_str());
        if (texSeparator == 0) texSeparator = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/separator.bmp").c_str());
    }
    if (texHealLabel == 0) {
        texHealLabel = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/heal_label.png").c_str());
        if (texHealLabel == 0) texHealLabel = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/heal_label.bmp").c_str());
    }
    if (texScoreHeal == 0) {
        texScoreHeal = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/Score_heal.png").c_str());
        if (texScoreHeal == 0) texScoreHeal = iLoadImage((char*)GetAssetPath("Assets/UI/HUD/score_heal.png").c_str());
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
    int hudX = 950;
    int hudY = 580;
    int hudW = 310;
    int hudH = 30;

    iSetColor(0, 0, 0);
    iFilledRectangle(hudX - 2, hudY - 2, hudW + 4, hudH + 4);
    iSetColor(12, 16, 24);
    iFilledRectangle(hudX, hudY, hudW, hudH);
    iSetColor(0, 180, 220);
    iRectangle(hudX, hudY, hudW, hudH);

    // Adjustable HUD text variables (Tactical 9x15 font, 15% larger)
    int hudTextX = 974;
    int hudTextY = 587;
    int hudFontSize = 15; // 9x15 compact tactical military font

    // 1) Render "SCORE:" label in warm gold
    DrawShadowText(hudTextX, hudTextY, "SCORE:", GLUT_BITMAP_9_BY_15, 255, 215, 0);

    // 2) Render dynamic C++ score value (%07d) in white
    char scoreNumStr[16];
    sprintf_s(scoreNumStr, sizeof(scoreNumStr), "%07d", score);
    DrawShadowText(hudTextX + 63, hudTextY, scoreNumStr, GLUT_BITMAP_9_BY_15, 255, 255, 255);

    // 3) Render "   |   " separator with tactical spacing in cyan accent
    DrawShadowText(hudTextX + 126, hudTextY, "   |   ", GLUT_BITMAP_9_BY_15, 0, 180, 220);

    // 4) Render "[H] HEAL" prompt in warm gold
    DrawShadowText(hudTextX + 189, hudTextY, "[H] HEAL", GLUT_BITMAP_9_BY_15, 255, 215, 0);

    // 6. Sleek Cinematic Area & Village Title Banner
    DrawAreaBanner(areaName, areaBannerAlpha);
}

void UI::DrawAreaBanner(const char* areaName, double alpha) {
    if (alpha <= 0.01 || !areaName) return;

    int textLen = (int)strlen(areaName);
    int bannerW = 340;
    int bannerH = 46;
    int bannerX = 470;
    int bannerY = 615;

    // Dark glass background panel
    iSetColor(10, 14, 22);
    iFilledRectangle(bannerX, bannerY, bannerW, bannerH);

    // Dual cyan and gold accent border stroke
    iSetColor(0, 220, 255);
    iRectangle(bannerX, bannerY, bannerW, bannerH);
    iSetColor(255, 215, 0);
    iRectangle(bannerX + 2, bannerY + 2, bannerW - 4, bannerH - 4);

    // Level 1 Chapter Title (Upper Header)
    int headerX = bannerX + (bannerW / 2) - 55;
    DrawOutlinedText(headerX, bannerY + 28, "THE FALLEN VILLAGE", GLUT_BITMAP_HELVETICA_10, 0, 240, 255);

    // Current Area Name (Main Title)
    int titleX = bannerX + (bannerW / 2) - (textLen * 4);
    DrawShadowText(titleX, bannerY + 8, areaName, GLUT_BITMAP_HELVETICA_12, 255, 220, 0);
}

// ----------------------------------------------------------------------------
// 1. HEALTH BAR (PERFECT INSIDE SPEAR FRAME ALIGNMENT)
// ----------------------------------------------------------------------------
void UI::DrawHealthBar(int hp, int maxHp, double displayedHp) {
    int startX = 20;
    int startY = 638;
    int frameW = 360;
    int frameH = 34;

    double hpRatio = (double)hp / (double)maxHp;
    if (hpRatio < 0.0) hpRatio = 0.0;
    if (hpRatio > 1.0) hpRatio = 1.0;

    // Header Text placed cleanly ABOVE the spiked frame
    DrawShadowText(startX + 10, startY + frameH + 6, "VITAL STATUS", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    char hpStr[32];
    int pct = (int)std::round(((double)hp / (double)maxHp) * 100.0);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    sprintf_s(hpStr, sizeof(hpStr), "HP %d/%d (%d%%)", hp, maxHp, pct);
    DrawShadowText(startX + 210, startY + frameH + 6, hpStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    // STEP 1: Draw empty metal frame base
    if (texHealthFrame != 0) {
        iShowImage(startX, startY, frameW, frameH, texHealthFrame);
    }

    // STEP 2: Calculate fill width and inner clipping area
    int innerBarWidth = 260;
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
// 2. STAMINA BAR (PERFECT INSIDE FRAME ALIGNMENT)
// ----------------------------------------------------------------------------
void UI::DrawStaminaBar(int stamina, int maxStamina, double displayedStamina) {
    int startX = 20;
    int startY = 560;
    int frameW = 360;
    int frameH = 34;

    double staminaRatio = displayedStamina / (double)maxStamina;
    if (staminaRatio < 0.0) staminaRatio = 0.0;
    if (staminaRatio > 1.0) staminaRatio = 1.0;

    // Header Text placed cleanly ABOVE the frame
    DrawShadowText(startX + 10, startY + frameH + 6, "ENERGY STATUS", GLUT_BITMAP_HELVETICA_12, 255, 255, 255);

    char stmStr[32];
    int pct = (int)std::round((displayedStamina / (double)maxStamina) * 100.0);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    if (pct <= 20) {
        sprintf_s(stmStr, sizeof(stmStr), "STAMINA %d%% (LOW)", pct);
        DrawShadowText(startX + 210, startY + frameH + 6, stmStr, GLUT_BITMAP_HELVETICA_12, 255, 100, 50);
    } else {
        sprintf_s(stmStr, sizeof(stmStr), "STAMINA %d%%", pct);
        DrawShadowText(startX + 210, startY + frameH + 6, stmStr, GLUT_BITMAP_HELVETICA_12, 255, 255, 255);
    }

    // STEP 1: Draw metal frame base FIRST (Underneath fill so fill is NEVER obscured!)
    if (texStaminaFrame != 0) {
        iShowImage(startX, startY, frameW, frameH, texStaminaFrame);
    } else {
        iSetColor(40, 50, 64);
        iRectangle(startX, startY, frameW, frameH);
        iSetColor(65, 78, 98);
        iRectangle(startX + 1, startY + 1, frameW - 2, frameH - 2);
    }

    // STEP 2: Calculate fill width and inner slot coordinates (Exact PNG texture metrics)
    int fillX = startX + (int)(frameW * 0.1172); // 62
    int maxFillW = (int)(frameW * 0.7630);      // 274
    int fillY = startY + (int)(frameH * 0.4873); // 576
    int fillH = 5;                              // 5px height fill

    int staminaFillWidth = (int)(maxFillW * staminaRatio);

    // Dark Track Base inside stamina bar slot
    iSetColor(12, 8, 10);
    iFilledRectangle(fillX, fillY, maxFillW, fillH);

    // STEP 3: Draw Gold Stamina Fill ON TOP of Frame Base so it is 100% visible!
    if (staminaFillWidth > 0) {
        // Base Deep Gold Fill
        iSetColor(240, 175, 15);
        iFilledRectangle(fillX, fillY, staminaFillWidth, fillH);

        // Mid-tone Bright Gold Highlight
        iSetColor(255, 205, 30);
        iFilledRectangle(fillX, fillY + 1, staminaFillWidth, fillH - 2);

        // Top Gloss Highlight Streak
        iSetColor(255, 240, 100);
        iFilledRectangle(fillX, fillY + fillH - 1, staminaFillWidth, 1);

        // Bottom Inset Shadow
        iSetColor(150, 100, 5);
        iFilledRectangle(fillX, fillY, staminaFillWidth, 1);

        if (texStaminaFill != 0) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(fillX, fillY, staminaFillWidth, fillH);

            iShowImageSub(fillX, fillY, staminaFillWidth, fillH, texStaminaFill, 0.0, 0.0, staminaRatio, 1.0);

            glDisable(GL_SCISSOR_TEST);
        }
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
    // Positioned in Bottom-Left Position - Post-Apocalyptic Military Survival Equipment Device
    int startX = 20;
    int startY = 20;
    int barW = 370;
    int barH = 66;

    // 1. MAIN PANEL BASE - Dark charcoal metal with blue-grey tint & worn surface layering
    // Deep shadow background casing
    iSetColor(8, 12, 16);
    iFilledRectangle(startX - 2, startY - 2, barW + 4, barH + 4);

    // Main dark charcoal body with slight blue-grey tint
    iSetColor(20, 26, 34);
    iFilledRectangle(startX, startY, barW, barH);

    // Weathered inner metal plate (slightly lighter blue-grey metal)
    iSetColor(28, 36, 46);
    iFilledRectangle(startX + 2, startY + 2, barW - 4, barH - 4);

    // Dark lower gradient plate simulation
    iSetColor(16, 20, 26);
    iFilledRectangle(startX + 4, startY + 4, barW - 8, (barH / 2) - 2);

    // Surface wear: subtle diagonal scratches & panel seam details
    iSetColor(58, 68, 78);
    iLine(startX + 15, startY + 14, startX + 35, startY + 10);
    iLine(startX + barW - 55, startY + barH - 12, startX + barW - 25, startY + barH - 16);
    iSetColor(42, 50, 60);
    iLine(startX + 16, startY + 13, startX + 36, startY + 9);

    // 2. BORDER - Rusted iron / aged metal appearance
    // Outer rusted iron border frame
    iSetColor(125, 55, 20);
    iRectangle(startX, startY, barW, barH);

    // Inner aged dark steel border frame
    iSetColor(60, 68, 78);
    iRectangle(startX + 1, startY + 1, barW - 2, barH - 2);

    // Dark brown-orange rusted edges & corner patches
    iSetColor(155, 75, 28); // Burnt rust brown corner accents
    iFilledRectangle(startX + 2, startY + barH - 6, 9, 4);
    iFilledRectangle(startX + barW - 11, startY + barH - 6, 9, 4);
    iFilledRectangle(startX + 2, startY + 2, 9, 4);
    iFilledRectangle(startX + barW - 11, startY + 2, 9, 4);

    // Subtle damaged notch cuts & rusted iron accents
    iSetColor(95, 40, 15); // Deep oxidized rust halos
    iRectangle(startX + 2, startY + barH - 6, 9, 4);
    iRectangle(startX + barW - 11, startY + barH - 6, 9, 4);

    // Corner rivets (Aged metal hardware look with rust halos)
    iSetColor(115, 122, 130);
    iFilledRectangle(startX + 4, startY + barH - 5, 2, 2);
    iFilledRectangle(startX + barW - 6, startY + barH - 5, 2, 2);
    iFilledRectangle(startX + 4, startY + 3, 2, 2);
    iFilledRectangle(startX + barW - 6, startY + 3, 2, 2);

    // Faded red warning accent mark (Small emergency gear tag on top right corner)
    iSetColor(160, 45, 45);
    iFilledRectangle(startX + barW - 32, startY + barH - 4, 12, 2);

    // 3. TITLE & HEADER - Military terminal style with warm text & burnt orange highlight
    // Muted military olive status LED
    iSetColor(75, 105, 60); // Dark olive base
    iFilledCircle(startX + 14, startY + barH - 13, 3);
    iSetColor(95, 135, 75); // Faded olive ring
    iCircle(startX + 14, startY + barH - 13, 3);

    // Title text in warm off-white / light grey military terminal style
    DrawShadowText(startX + 24, startY + barH - 17, "SURVIVAL SUPPLIES", GLUT_BITMAP_HELVETICA_10, 225, 220, 205);
    // [TAB] keybind tag highlighted in muted burnt orange
    DrawShadowText(startX + 138, startY + barH - 17, "[TAB]", GLUT_BITMAP_HELVETICA_10, 215, 115, 35);

    // 4. ITEM SLOTS - Dark metal containers with rusted frames & depth shadow
    int wellY = startY + 6;
    int wellH = 34;
    int wellW = 84;
    int colX[4] = { startX + 8, startX + 98, startX + 188, startX + 278 };

    for (int i = 0; i < 4; i++) {
        // Outer dark metal container bevel frame
        iSetColor(52, 60, 70);
        iRectangle(colX[i] - 1, wellY - 1, wellW + 2, wellH + 2);

        // Recessed deep dark metal container well
        iSetColor(10, 14, 20);
        iFilledRectangle(colX[i], wellY, wellW, wellH);

        // Top/left inset drop shadow for container depth
        iSetColor(6, 8, 12);
        iFilledRectangle(colX[i], wellY + wellH - 2, wellW, 2);
        iFilledRectangle(colX[i], wellY, 2, wellH);

        // Rusted iron frame border
        iSetColor(110, 55, 22);
        iRectangle(colX[i], wellY, wellW, wellH);

        // Inner highlight rim for metallic depth separation
        iSetColor(75, 85, 98);
        iRectangle(colX[i] + 1, wellY + 1, wellW - 2, wellH - 2);

        // Subtle rusted corner accent on each slot frame
        iSetColor(145, 65, 25);
        iFilledRectangle(colX[i] + 1, wellY + wellH - 3, 3, 2);
    }

    int iconSize = 22;
    int iconOffsetY = wellY + 6;

    // 1. Medkit Slot & Count
    int iconX0 = colX[0] + 6;
    if (texIconMedkit != 0) {
        iShowImage(iconX0, iconOffsetY, iconSize, iconSize, texIconMedkit);
    } else {
        iSetColor(150, 35, 35); // Weathered rusty red medkit
        iFilledRectangle(iconX0, iconOffsetY, iconSize, iconSize);
        iSetColor(70, 20, 20);
        iRectangle(iconX0, iconOffsetY, iconSize, iconSize);
        iSetColor(235, 230, 218); // Warm off-white medical cross
        iFilledRectangle(iconX0 + 9, iconOffsetY + 4, 4, 14);
        iFilledRectangle(iconX0 + 4, iconOffsetY + 9, 14, 4);
    }
    char medStr[16];
    sprintf_s(medStr, sizeof(medStr), "x%d", player.medkits);
    DrawShadowText(colX[0] + 35, iconOffsetY + 5, medStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);

    // 2. Food Slot & Count
    int iconX1 = colX[1] + 6;
    if (texIconFood != 0) {
        iShowImage(iconX1, iconOffsetY, iconSize, iconSize, texIconFood);
    } else {
        iSetColor(140, 90, 30); // Survival MRE ration pouch
        iFilledRectangle(iconX1, iconOffsetY, iconSize, iconSize);
        iSetColor(180, 140, 60);
        iRectangle(iconX1, iconOffsetY, iconSize, iconSize);
        iSetColor(210, 150, 40);
        iFilledRectangle(iconX1 + 4, iconOffsetY + 4, 14, 14);
        iSetColor(90, 125, 75); // Muted military olive label band
        iFilledRectangle(iconX1 + 6, iconOffsetY + 13, 10, 3);
    }
    char foodStr[16];
    sprintf_s(foodStr, sizeof(foodStr), "x%d", player.foodCount);
    DrawShadowText(colX[1] + 35, iconOffsetY + 5, foodStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);

    // 3. Battery / Power Slot & Count
    int iconX2 = colX[2] + 6;
    if (texIconBattery != 0) {
        iShowImage(iconX2, iconOffsetY, iconSize, iconSize, texIconBattery);
    } else {
        iSetColor(25, 32, 40); // Tactical power cell casing
        iFilledRectangle(iconX2, iconOffsetY, iconSize, iconSize);
        iSetColor(70, 80, 95);
        iRectangle(iconX2, iconOffsetY, iconSize, iconSize);
        iSetColor(190, 115, 28); // Amber power core
        iFilledRectangle(iconX2 + 6, iconOffsetY + 5, 10, 12);
        iSetColor(220, 160, 45); // Terminal top
        iFilledRectangle(iconX2 + 8, iconOffsetY + 17, 6, 2);
    }
    char batStr[16];
    sprintf_s(batStr, sizeof(batStr), "x%d", player.batteryCount);
    DrawShadowText(colX[2] + 35, iconOffsetY + 5, batStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);

    // 4. Scrap / Resources Slot & Count
    int iconX3 = colX[3] + 6;
    if (texIconScrap != 0) {
        iShowImage(iconX3, iconOffsetY, iconSize, iconSize, texIconScrap);
    } else {
        iSetColor(45, 50, 60); // Salvaged metallic hardware plate
        iFilledRectangle(iconX3, iconOffsetY, iconSize, iconSize);
        iSetColor(120, 130, 140);
        iRectangle(iconX3, iconOffsetY, iconSize, iconSize);
        iSetColor(160, 85, 40); // Rusty gear fragment
        iFilledRectangle(iconX3 + 4, iconOffsetY + 4, 8, 8);
        iSetColor(150, 160, 170); // Steel hardware
        iFilledRectangle(iconX3 + 10, iconOffsetY + 10, 8, 8);
    }
    char scrapStr[16];
    sprintf_s(scrapStr, sizeof(scrapStr), "x%d", player.scrapCount);
    DrawShadowText(colX[3] + 35, iconOffsetY + 5, scrapStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);
}

void UI::DrawInventoryIndicator(const Player& player, bool hasKeycard, bool ribbonCollected) {
    DrawInventoryHUD(player, hasKeycard, ribbonCollected);
}

// ----------------------------------------------------------------------------
// 5. WEAPON DISPLAY UPGRADE
// ----------------------------------------------------------------------------
void UI::DrawWeaponDisplay(const char* weaponName, int ammo, bool usesAmmo) {
    // Positioned in Bottom-Right Position
    int boxX = 1040;
    int boxY = 20;
    int boxW = 220;
    int boxH = 75;

    // 1. Dark Charcoal / Black Metal Base Panel (Post-Apocalyptic Survival Equipment Theme)
    iSetColor(0, 0, 0);
    iFilledRectangle(boxX - 2, boxY - 2, boxW + 4, boxH + 4);

    // Main dark charcoal body
    iSetColor(14, 18, 22);
    iFilledRectangle(boxX, boxY, boxW, boxH);

    // Weathered inner plate with slight vertical gradient simulation
    iSetColor(22, 28, 34);
    iFilledRectangle(boxX + 2, boxY + 2, boxW - 4, boxH - 4);
    iSetColor(18, 22, 28);
    iFilledRectangle(boxX + 4, boxY + 4, boxW - 8, boxH / 2 - 4);

    // Worn Dark Steel Border & Metallic Trim
    iSetColor(65, 75, 85);
    iRectangle(boxX, boxY, boxW, boxH);
    iSetColor(45, 52, 60);
    iRectangle(boxX + 1, boxY + 1, boxW - 2, boxH - 2);

    // Rust Brown & Dark Steel Accents on Panel Corners
    iSetColor(140, 70, 35); // Rust Brown accent corners
    iFilledRectangle(boxX + 2, boxY + boxH - 6, 8, 4);
    iFilledRectangle(boxX + boxW - 10, boxY + boxH - 6, 8, 4);
    iFilledRectangle(boxX + 2, boxY + 2, 8, 4);
    iFilledRectangle(boxX + boxW - 10, boxY + 2, 8, 4);

    // Corner rivets (Metallic military hardware look)
    iSetColor(110, 120, 130);
    iFilledRectangle(boxX + 4, boxY + boxH - 5, 2, 2);
    iFilledRectangle(boxX + boxW - 6, boxY + boxH - 5, 2, 2);
    iFilledRectangle(boxX + 4, boxY + 3, 2, 2);
    iFilledRectangle(boxX + boxW - 6, boxY + 3, 2, 2);

    // Weathering scratch marks for post-apocalyptic survivor look
    iSetColor(50, 60, 70);
    iLine(boxX + 10, boxY + 12, boxX + 25, boxY + 8);
    iLine(boxX + boxW - 35, boxY + boxH - 10, boxX + boxW - 15, boxY + boxH - 12);

    // 2. Weapon Icon Well / Frame (Left Side of Panel)
    int wellX = boxX + 10;
    int wellY = boxY + 10;
    int wellW = 64;
    int wellH = 55;

    // Recessed dark well for equipped Katana
    iSetColor(8, 10, 14);
    iFilledRectangle(wellX, wellY, wellW, wellH);
    iSetColor(40, 48, 56);
    iRectangle(wellX, wellY, wellW, wellH);
    iSetColor(90, 50, 25); // Subtle rust-brown inner border accent
    iRectangle(wellX + 1, wellY + 1, wellW - 2, wellH - 2);

    // Load & Render Katana Icon Centered in Well
    if (texIconKatana == 0) {
        texIconKatana = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/katana.png").c_str());
        if (texIconKatana == 0) {
            texIconKatana = ResourceManager::GetInstance().GetTexture("Assets/Items/KeyItems/katana.png");
        }
    }

    if (texIconKatana != 0) {
        // Katana resolution 1536x1024 (aspect ratio 1.5).
        // Center Katana image inside 64x55 well: 54px width x 36px height
        int imgW = 54;
        int imgH = 36;
        int imgX = wellX + (wellW - imgW) / 2;
        int imgY = wellY + (wellH - imgH) / 2;
        iShowImage(imgX, imgY, imgW, imgH, texIconKatana);
    } else {
        // Sleek Katana Blade Vector Icon Fallback
        int iconX = wellX + 18;
        int iconY = wellY + 14;
        iSetColor(220, 230, 245); // Silver Katana Blade
        iLine(iconX + 2, iconY + 4, iconX + 26, iconY + 28);
        iLine(iconX + 3, iconY + 3, iconX + 27, iconY + 27);
        iSetColor(215, 110, 40); // Muted Orange/Gold Guard
        iFilledCircle(iconX + 9, iconY + 11, 4);
        iSetColor(140, 40, 30); // Dark Red Handle Wrap
        iLine(iconX + 2, iconY + 4, iconX + 9, iconY + 11);
    }

    // 3. Text Presentation (Military / Survivor Equipment Style)
    int textX = boxX + 84;

    // Weapon Name: KATANA (Off-white / Steel Ivory)
    DrawOutlinedText(textX, boxY + boxH - 24, weaponName ? weaponName : "KATANA", GLUT_BITMAP_HELVETICA_18, 235, 230, 220);

    // Sub-text: MELEE WEAPON [J] or AMMO counter (Muted survival orange / rust accent)
    if (usesAmmo) {
        DrawAmmoCounter(ammo, 48);
    } else {
        // Muted Survival Amber/Orange indicator instead of bright cyan
        DrawShadowText(textX, boxY + 16, "MELEE WEAPON [J]", GLUT_BITMAP_HELVETICA_10, 215, 120, 45);
    }
}

void UI::DrawAmmoCounter(int ammo, int reserveAmmo) {
    int boxX = 1040;
    int boxY = 20;

    char ammoStr[32];
    sprintf_s(ammoStr, sizeof(ammoStr), "AMMO %d / %d", ammo, reserveAmmo);
    DrawShadowText(boxX + 84, boxY + 16, ammoStr, GLUT_BITMAP_HELVETICA_12, 215, 120, 45);
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
void UI::DrawBossHealthBar(const char* bossName, int bossHp, int bossMaxHp, double displayedHp) {
    if (displayedHp < 0.0) {
        displayedHp = (double)bossHp;
    }

    // Boss HUD dimensions & top-centered positioning
    int barW = 600;
    int barH = 40;
    int barX = 640 - (barW / 2); // 340 (left margin 340, right margin 340)
    int barY = 605;              // Positioned downward with generous top margin

    double bossHpPercent = (double)bossHp / (double)bossMaxHp;
    if (bossHpPercent < 0.0) bossHpPercent = 0.0;
    if (bossHpPercent > 1.0) bossHpPercent = 1.0;

    double displayedPercent = displayedHp / (double)bossMaxHp;
    if (displayedPercent < 0.0) displayedPercent = 0.0;
    if (displayedPercent > 1.0) displayedPercent = 1.0;

    // 1. Calculate Inner Health Slot Coordinates (Matching boss_health_bar_frame.png inner opening)
    int fillX = barX + 80;    // 420
    int maxFillW = barW - 150; // 450
    int fillY = barY + 16;    // 621
    int fillH = 7;            // 7px height fill

    int innerFillW = (int)(maxFillW * bossHpPercent);
    int innerLagFillW = (int)(maxFillW * displayedPercent);

    // 2. Render Frame Base / Asset Texture Frame FIRST (Underneath fill so fill is NEVER obscured!)
    if (texBossFrame != 0) {
        iShowImage(barX, barY, barW, barH, texBossFrame);
    } else {
        iSetColor(22, 24, 32);
        iRectangle(barX, barY, barW, barH);
        iSetColor(180, 30, 35);
        iRectangle(barX + 1, barY + 1, barW - 2, barH - 2);
    }

    // Dark Track Base inside health bar slot
    iSetColor(12, 8, 10);
    iFilledRectangle(fillX, fillY, maxFillW, fillH);

    // 3. Trailing Hit Lag Damage Bar (Amber/Yellow Catch-up Bar inside slot)
    if (innerLagFillW > innerFillW && innerLagFillW > 0) {
        int lagW = innerLagFillW - innerFillW;
        if (innerFillW + lagW > maxFillW) lagW = maxFillW - innerFillW;
        if (lagW > 0) {
            iSetColor(235, 155, 20);
            iFilledRectangle(fillX + innerFillW, fillY, lagW, fillH);
            iSetColor(255, 215, 60);
            iFilledRectangle(fillX + innerFillW, fillY + fillH - 2, lagW, 2);
        }
    }

    // 4. Primary Crimson Health Fill (Drawn ON TOP of Frame Base so it is 100% visible!)
    if (innerFillW > 0) {
        // Base Vibrant Red Fill
        iSetColor(220, 25, 30);
        iFilledRectangle(fillX, fillY, innerFillW, fillH);

        // Mid-tone Gradient Highlight
        iSetColor(245, 55, 55);
        iFilledRectangle(fillX, fillY + 2, innerFillW, fillH - 3);

        // Top Gloss Highlight Streak
        iSetColor(255, 160, 160);
        iFilledRectangle(fillX, fillY + fillH - 2, innerFillW, 2);

        // Bottom Shadow Streak
        iSetColor(130, 10, 15);
        iFilledRectangle(fillX, fillY, innerFillW, 1);

        // Leading Pulse Edge Tip
        iSetColor(255, 240, 180);
        iFilledRectangle(fillX + innerFillW - 2, fillY, 2, fillH);

        // If texture fill is loaded, overlay texBossFill precisely inside slot using glScissor
        if (texBossFill != 0) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(fillX, fillY, innerFillW, fillH);

            iShowImageSub(fillX, fillY, innerFillW, fillH, texBossFill, 0.0, 0.0, bossHpPercent, 1.0);

            glDisable(GL_SCISSOR_TEST);
        }
    }

    // 5. Left Boss Skull Emblem Badge
    int skullX = barX - 22;
    int skullY = barY + 2;
    int skullW = 36;
    int skullH = 36;

    iSetColor(10, 8, 12);
    iFilledRectangle(skullX, skullY, skullW, skullH);
    iSetColor(200, 30, 30);
    iRectangle(skullX, skullY, skullW, skullH);
    iSetColor(255, 80, 80);
    iRectangle(skullX + 2, skullY + 2, skullW - 4, skullH - 4);

    iSetColor(230, 220, 210);
    iFilledRectangle(skullX + 10, skullY + 14, 16, 14); // Skull Head
    iFilledRectangle(skullX + 13, skullY + 8, 10, 6);   // Skull Jaw
    iSetColor(20, 10, 10);
    iFilledRectangle(skullX + 12, skullY + 18, 4, 5);  // Left Eye Socket
    iFilledRectangle(skullX + 20, skullY + 18, 4, 5);  // Right Eye Socket
    iFilledRectangle(skullX + 15, skullY + 8, 2, 4);   // Teeth
    iFilledRectangle(skullX + 19, skullY + 8, 2, 4);
    iSetColor(255, 30, 30);
    iFilledRectangle(skullX + 13, skullY + 20, 2, 2);  // Glowing Pupil L
    iFilledRectangle(skullX + 21, skullY + 20, 2, 2);  // Glowing Pupil R

    // 6. Centered Header Display (FINAL BOSS & MUTATED BRUTE centered at X = 640)
    int tagW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_10, "FINAL BOSS");
    int tagX = 640 - (tagW / 2);
    DrawShadowText(tagX, 672, "FINAL BOSS", GLUT_BITMAP_HELVETICA_10, 255, 70, 70);

    int nameW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_12, bossName);
    int nameX = 640 - (nameW / 2);
    DrawOutlinedText(nameX, 652, bossName, GLUT_BITMAP_HELVETICA_12, 255, 220, 100);

    // 7. Centered Numerical Health & Percentage Text
    char hpStr[64];
    int pct = (int)(bossHpPercent * 100.0);
    sprintf_s(hpStr, sizeof(hpStr), "%d / %d (%d%%)", bossHp, bossMaxHp, pct);
    int hpW = GetGlutStringWidth(GLUT_BITMAP_HELVETICA_10, hpStr);
    int hpX = 640 - (hpW / 2);
    DrawShadowText(hpX, 631, hpStr, GLUT_BITMAP_HELVETICA_10, 240, 240, 250);
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
