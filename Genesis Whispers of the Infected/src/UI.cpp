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
unsigned int UI::texIconWaterBottle = 0;
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

void UI::DrawAlphaText(int x, int y, const char* str, void* font, int r, int g, int b, double alpha) {
    if (alpha <= 0.005 || !str) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)alpha);
    iText(x, y, (char*)str, font);
}

void UI::DrawAlphaShadowText(int x, int y, const char* str, void* font, int r, int g, int b, double alpha, int shadowOffset) {
    if (alpha <= 0.005 || !str) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.90f * (float)alpha);
    iText(x + shadowOffset, y - shadowOffset, (char*)str, font);
    glColor4f((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)alpha);
    iText(x, y, (char*)str, font);
}

void UI::DrawAlphaOutlinedText(int x, int y, const char* str, void* font, int r, int g, int b, double alpha) {
    if (alpha <= 0.005 || !str) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.90f * (float)alpha);
    iText(x + 1, y, (char*)str, font);
    iText(x - 1, y, (char*)str, font);
    iText(x, y + 1, (char*)str, font);
    iText(x, y - 1, (char*)str, font);
    glColor4f((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)alpha);
    iText(x, y, (char*)str, font);
}

int UI::GetTextWidth(const char* str, void* font) {
    if (!str) return 0;
    int len = (int)strlen(str);
    if (font == GLUT_BITMAP_HELVETICA_18) return len * 10;
    if (font == GLUT_BITMAP_HELVETICA_12) return len * 7;
    if (font == GLUT_BITMAP_HELVETICA_10) return len * 6;
    if (font == GLUT_BITMAP_9_BY_15) return len * 9;
    if (font == GLUT_BITMAP_8_BY_13) return len * 8;
    return len * 8;
}

void UI::DrawKatanaStyleBox(int boxX, int boxY, int boxW, int boxH) {
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
    iFilledRectangle(boxX + 4, boxY + 4, boxW - 8, (boxH / 2) - 4);

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
    if (boxW >= 50 && boxH >= 20) {
        iLine(boxX + 10, boxY + 12, boxX + 25, boxY + 8);
        iLine(boxX + boxW - 35, boxY + boxH - 10, boxX + boxW - 15, boxY + boxH - 12);
    }
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
    if (texIconWaterBottle == 0) {
        texIconWaterBottle = iLoadImage((char*)GetAssetPath("Assets/Items/Food/water_bottle.png").c_str());
    }
    if (texIconScrap == 0) {
        texIconScrap = iLoadImage((char*)GetAssetPath("Assets/Items/KeyItems/Scrap_Metal.png").c_str());
    }
}

// ============================================================================
// HUD DRAWING ROUTINES (PHASE 1 VISUAL UPGRADE)
// ============================================================================
void UI::DrawHUD(const Player& player, int score, const char* objectiveText, const char* areaName, double notifyTimer, double areaBannerAlpha, const char* chapterName) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Draw Player VITAL STATUS (Health Bar)
    DrawHealthBar(player.hp, player.maxHp, player.displayedHp);

    // 2. Draw Player ENERGY STATUS (Stamina Bar)
    DrawStaminaBar(player.stamina, player.maxStamina, player.displayedStamina);

    // 3. Draw Mission / Objective Box (Top-Right)
    DrawMissionPanel(objectiveText, areaName, notifyTimer);

    // 4. Draw Inventory HUD (Bottom-Left)
    DrawInventoryHUD(player, false, false);

    // 5. Score Banner (Upper Right Header) - Redesigned SURVIVAL DATA panel
    int hudW = 260;
    int hudH = 110;
    int hudX = 1260 - hudW; // Right-aligned under the mission panel
    int hudY = 500;         // Placed nicely below the mission panel

    double alpha = 0.9;
    
    // Main dark metal panel
    glColor4f(0.10f, 0.15f, 0.20f, alpha);
    iFilledRectangle(hudX, hudY, hudW, hudH);
    
    // Top highlight
    glColor4f(0.15f, 0.22f, 0.30f, 0.35f * alpha);
    iFilledRectangle(hudX + 2, hudY + (hudH / 2), hudW - 4, (hudH / 2) - 2);

    // Weathered border
    glColor4f(0.35f, 0.42f, 0.50f, 0.90f * alpha);
    iRectangle(hudX, hudY, hudW, hudH);

    // Inner cyan tactical wireframe stroke
    glColor4f(0.0f, 0.85f, 1.0f, 0.65f * alpha);
    iRectangle(hudX + 2, hudY + 2, hudW - 4, hudH - 4);
    
    // Corner Rust Brackets
    glColor4f(0.58f, 0.28f, 0.14f, 0.88f * alpha);
    iFilledRectangle(hudX, hudY + hudH - 6, 8, 6);
    iFilledRectangle(hudX + hudW - 8, hudY + hudH - 6, 8, 6);
    iFilledRectangle(hudX, hudY, 8, 6);
    iFilledRectangle(hudX + hudW - 8, hudY, 8, 6);

    // Title: SURVIVAL DATA (Small cyan text)
    DrawAlphaText(hudX + 15, hudY + hudH - 22, "SURVIVAL DATA", GLUT_BITMAP_HELVETICA_10, 0, 216, 255, alpha);
    
    // Divider
    glColor4f(0.0f, 0.85f, 1.0f, 0.45f * alpha);
    iLine(hudX + 10, hudY + hudH - 28, hudX + hudW - 10, hudY + hudH - 28);
    
    // Layout Metrics
    int labelX = hudX + 15;
    int valueX = hudX + 140; // Align all values neatly
    
    // SCORE (Row 1)
    DrawAlphaText(labelX, hudY + hudH - 50, "SCORE", GLUT_BITMAP_HELVETICA_10, 200, 205, 210, alpha);
    char scoreNumStr[16];
    sprintf_s(scoreNumStr, sizeof(scoreNumStr), "%07d", score);
    DrawAlphaShadowText(valueX, hudY + hudH - 53, scoreNumStr, GLUT_BITMAP_HELVETICA_18, 240, 245, 250, alpha, 1);
    
    // ENEMIES DEFEATED (Row 2)
    DrawAlphaText(labelX, hudY + hudH - 75, "ENEMIES", GLUT_BITMAP_HELVETICA_10, 200, 205, 210, alpha);
    DrawAlphaShadowText(valueX, hudY + hudH - 78, "00", GLUT_BITMAP_HELVETICA_18, 240, 245, 250, alpha, 1);
    
    // RESOURCES (Row 3)
    DrawAlphaText(labelX, hudY + hudH - 100, "RESOURCES", GLUT_BITMAP_HELVETICA_10, 200, 205, 210, alpha);
    int resources = player.scrapCount + player.foodCount + player.batteryCount;
    
    // Muted orange warning if no resources
    int resR = 240, resG = 245, resB = 250;
    if (resources == 0) { resR = 210; resG = 80; resB = 40; }
    
    char resNumStr[16];
    sprintf_s(resNumStr, sizeof(resNumStr), "%02d", resources);
    DrawAlphaShadowText(valueX, hudY + hudH - 103, resNumStr, GLUT_BITMAP_HELVETICA_18, resR, resG, resB, alpha, 1);

    // 6. Sleek Cinematic Area & Village Title Banner
    DrawAreaBanner(areaName, areaBannerAlpha);
}

void UI::DrawAreaBanner(const char* areaName, double alpha, const char* chapterName) {
    if (alpha <= 0.005 || !areaName) return;

    const char* chapName = (chapterName && strlen(chapterName) > 0) ? chapterName : "THE FALLEN VILLAGE";

    int mainW = GetTextWidth(areaName, GLUT_BITMAP_HELVETICA_18);
    int chapW = GetTextWidth(chapName, GLUT_BITMAP_HELVETICA_10);
    int maxTextW = (mainW > chapW) ? mainW : chapW;

    // Dynamic width calculation so short and long location names fit perfectly centered
    int bannerW = (maxTextW + 170 < 440) ? 440 : (maxTextW + 170);
    int bannerH = 62;

    int centerX = 640;
    int targetY = 628;

    // Smooth entrance/exit vertical slide
    double slideOffset = (1.0 - alpha) * 18.0;
    int bannerY = (int)(targetY - slideOffset);

    // Subtle horizontal digital glitch offset during transition
    int glitchX = 0;
    if (alpha < 0.85) {
        glitchX = (int)(sin(alpha * 50.0) * 3.5 * (1.0 - alpha));
    }

    int bannerX = centerX - (bannerW / 2) + glitchX;

    float a = (float)alpha;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Translucent Dark Glass Background Panel (Post-Apocalyptic Survival Slate)
    glColor4f(0.04f, 0.07f, 0.11f, 0.88f * a);
    iFilledRectangle(bannerX, bannerY, bannerW, bannerH);

    // Subtle upper glass specular highlight
    glColor4f(0.15f, 0.22f, 0.30f, 0.35f * a);
    iFilledRectangle(bannerX + 2, bannerY + (bannerH / 2), bannerW - 4, (bannerH / 2) - 2);

    // Bottom dark accent shadow band
    glColor4f(0.02f, 0.03f, 0.05f, 0.45f * a);
    iFilledRectangle(bannerX + 2, bannerY + 2, bannerW - 4, 16);

    // 2. Weathered Metallic & Rust Trim Frame
    // Outer worn dark steel border
    glColor4f(0.35f, 0.42f, 0.50f, 0.90f * a);
    iRectangle(bannerX, bannerY, bannerW, bannerH);

    // Inner cyan tactical wireframe stroke
    glColor4f(0.0f, 0.85f, 1.0f, 0.65f * a);
    iRectangle(bannerX + 2, bannerY + 2, bannerW - 4, bannerH - 4);

    // Corner Rust / Copper Metallic Brackets (Post-Apocalyptic hardware style)
    glColor4f(0.58f, 0.28f, 0.14f, 0.88f * a);
    iFilledRectangle(bannerX, bannerY + bannerH - 8, 12, 8);
    iFilledRectangle(bannerX + bannerW - 12, bannerY + bannerH - 8, 12, 8);
    iFilledRectangle(bannerX, bannerY, 12, 8);
    iFilledRectangle(bannerX + bannerW - 12, bannerY, 12, 8);

    // Corner Fastener Rivets
    glColor4f(0.85f, 0.90f, 0.95f, 0.95f * a);
    iFilledRectangle(bannerX + 4, bannerY + bannerH - 6, 3, 3);
    iFilledRectangle(bannerX + bannerW - 7, bannerY + bannerH - 6, 3, 3);
    iFilledRectangle(bannerX + 4, bannerY + 3, 3, 3);
    iFilledRectangle(bannerX + bannerW - 7, bannerY + 3, 3, 3);

    // 3. Decorative Telemetry & Tactical Marker Details
    // Left side Tactical Reticle Icon [⌖]
    int lIconCenterX = bannerX + 22;
    int iconCenterY = bannerY + (bannerH / 2);

    glColor4f(0.0f, 0.85f, 1.0f, 0.85f * a);
    iRectangle(lIconCenterX - 6, iconCenterY - 6, 12, 12);
    iLine(lIconCenterX - 9, iconCenterY, lIconCenterX + 9, iconCenterY);
    iLine(lIconCenterX, iconCenterY - 9, lIconCenterX, iconCenterY + 9);
    glColor4f(1.0f, 1.0f, 1.0f, 0.95f * a);
    iFilledRectangle(lIconCenterX - 1, iconCenterY - 1, 3, 3);

    // Right side Tactical Reticle Icon
    int rIconCenterX = bannerX + bannerW - 22;
    glColor4f(0.0f, 0.85f, 1.0f, 0.85f * a);
    iRectangle(rIconCenterX - 6, iconCenterY - 6, 12, 12);
    iLine(rIconCenterX - 9, iconCenterY, rIconCenterX + 9, iconCenterY);
    iLine(rIconCenterX, iconCenterY - 9, rIconCenterX, iconCenterY + 9);
    glColor4f(1.0f, 1.0f, 1.0f, 0.95f * a);
    iFilledRectangle(rIconCenterX - 1, iconCenterY - 1, 3, 3);

    // Tactical Telemetry Text
    DrawAlphaText(bannerX + 38, bannerY + bannerH - 15, "[ SEC // 01 ]", GLUT_BITMAP_HELVETICA_10, 0, 200, 240, a);
    DrawAlphaText(bannerX + bannerW - 138, bannerY + bannerH - 15, "// INFECTED ZONE //", GLUT_BITMAP_HELVETICA_10, 160, 180, 200, a);

    // Horizontal Divider Line under chapter title
    glColor4f(0.0f, 0.80f, 1.0f, 0.45f * a);
    iLine(bannerX + 45, bannerY + 29, bannerX + bannerW - 45, bannerY + 29);
    iFilledRectangle(centerX - 2, bannerY + 28, 5, 3);

    // 4. Typography Hierarchy
    // Small Top Text: Chapter/Region (e.g. THE FALLEN VILLAGE)
    int chapX = centerX - (chapW / 2);
    int chapY = bannerY + 35;
    DrawAlphaOutlinedText(chapX, chapY, chapName, GLUT_BITMAP_HELVETICA_10, 0, 230, 255, a);

    // Large Main Text: Location Name Reveal (Focus of the UI)
    int mainX = centerX - (mainW / 2);
    int mainY = bannerY + 9;
    // Glow outline pass (cyan tint)
    DrawAlphaText(mainX + 1, mainY, areaName, GLUT_BITMAP_HELVETICA_18, 0, 210, 240, 0.45 * a);
    DrawAlphaText(mainX - 1, mainY, areaName, GLUT_BITMAP_HELVETICA_18, 0, 210, 240, 0.45 * a);
    // Main white focus text with deep shadow
    DrawAlphaShadowText(mainX, mainY, areaName, GLUT_BITMAP_HELVETICA_18, 255, 255, 255, a, 2);

    // 5. Subtle Glitch Scanline Effect during entrance/exit
    if (alpha < 0.85) {
        int scanY1 = bannerY + 14;
        int scanY2 = bannerY + bannerH - 18;
        glColor4f(0.0f, 0.90f, 1.0f, 0.35f * a);
        iLine(bannerX + 10, scanY1, bannerX + bannerW - 10, scanY1);
        iLine(bannerX + 15, scanY2, bannerX + bannerW - 15, scanY2);
    }
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
    int pct = (int)((((double)hp / (double)maxHp) * 100.0) + 0.5);
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
    int pct = (int)(((displayedStamina / (double)maxStamina) * 100.0) + 0.5);
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
    int missX = 930;
    int missY = 615;
    int missW = 330;
    int missH = 88;

    // Calculate animation alpha/pulse
    double alpha = 1.0;
    if (notifyTimer > 0.0) {
        alpha = 0.6 + 0.4 * sin(notifyTimer * 10.0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Main dark metal panel
    glColor4f(0.10f, 0.15f, 0.20f, (float)alpha);
    iFilledRectangle(missX, missY, missW, missH);
    
    // Top lighter highlight
    glColor4f(0.15f, 0.22f, 0.30f, (float)(0.35 * alpha));
    iFilledRectangle(missX + 2, missY + (missH / 2), missW - 4, (missH / 2) - 2);

    // Weathered border
    glColor4f(0.35f, 0.42f, 0.50f, (float)(0.90 * alpha));
    iRectangle(missX, missY, missW, missH);

    // Inner cyan tactical wireframe stroke
    glColor4f(0.0f, 0.85f, 1.0f, (float)(0.65 * alpha));
    iRectangle(missX + 2, missY + 2, missW - 4, missH - 4);

    // Corner Rust Brackets
    glColor4f(0.58f, 0.28f, 0.14f, (float)(0.88 * alpha));
    iFilledRectangle(missX, missY + missH - 8, 12, 8);
    iFilledRectangle(missX + missW - 12, missY + missH - 8, 12, 8);
    iFilledRectangle(missX, missY, 12, 8);
    iFilledRectangle(missX + missW - 12, missY, 12, 8);

    // Rivets
    glColor4f(0.85f, 0.90f, 0.95f, (float)(0.95 * alpha));
    iFilledRectangle(missX + 4, missY + missH - 6, 3, 3);
    iFilledRectangle(missX + missW - 7, missY + missH - 6, 3, 3);
    iFilledRectangle(missX + 4, missY + 3, 3, 3);
    iFilledRectangle(missX + missW - 7, missY + 3, 3, 3);

    // Small mission indicator icon (Reticle style)
    int iconX = missX + 22;
    int iconY = missY + missH - 15;
    glColor4f(0.0f, 0.85f, 1.0f, (float)(0.85 * alpha));
    iRectangle(iconX - 5, iconY - 5, 10, 10);
    iLine(iconX - 8, iconY, iconX + 8, iconY);
    iLine(iconX, iconY - 8, iconX, iconY + 8);
    glColor4f(1.0f, 1.0f, 1.0f, (float)(0.95 * alpha));
    iFilledRectangle(iconX - 1, iconY - 1, 3, 3);

    // Scanline effect during activation
    if (alpha < 0.95) {
        glColor4f(0.0f, 0.90f, 1.0f, (float)(0.25 * alpha));
        iLine(missX + 5, missY + 20, missX + missW - 5, missY + 20);
        iLine(missX + 5, missY + 60, missX + missW - 5, missY + 60);
    }

    // Texts
    const char* defaultObj = "ESCAPE THE FALLEN VILLAGE";
    const char* rawObjStr = objectiveText ? objectiveText : defaultObj;

    DrawAlphaText(missX + 38, missY + missH - 19, "MISSION // 001", GLUT_BITMAP_HELVETICA_10, 0, 210, 240, alpha);

    // Word wrap objective text if width exceeds maxTextW
    void* font = GLUT_BITMAP_HELVETICA_18;
    int maxTextW = missW - 36; // 294px available
    std::string fullStr = rawObjStr;
    std::string line1 = fullStr;
    std::string line2 = "";

    if (GetTextWidth(fullStr.c_str(), font) > maxTextW) {
        size_t lastSpace = std::string::npos;
        for (size_t i = 0; i < fullStr.length(); ++i) {
            if (fullStr[i] == ' ') {
                std::string testSub = fullStr.substr(0, i);
                if (GetTextWidth(testSub.c_str(), font) <= maxTextW) {
                    lastSpace = i;
                } else {
                    break;
                }
            }
        }
        if (lastSpace != std::string::npos) {
            line1 = fullStr.substr(0, lastSpace);
            line2 = fullStr.substr(lastSpace + 1);
        }
    }

    if (line2.empty()) {
        DrawAlphaShadowText(missX + 18, missY + 26, line1.c_str(), font, 240, 245, 250, alpha, 1);
    } else {
        DrawAlphaShadowText(missX + 18, missY + 38, line1.c_str(), font, 240, 245, 250, alpha, 1);
        DrawAlphaShadowText(missX + 18, missY + 16, line2.c_str(), font, 240, 245, 250, alpha, 1);
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

    // 1. Medkit Slot & Count [H]
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
    DrawShadowText(colX[0] + 32, iconOffsetY + 5, medStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);
    DrawShadowText(colX[0] + wellW - 20, iconOffsetY + 5, "[H]", GLUT_BITMAP_HELVETICA_10, 215, 115, 35);

    // 2. Food Slot & Count [F]
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
    DrawShadowText(colX[1] + 32, iconOffsetY + 5, foodStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);
    DrawShadowText(colX[1] + wellW - 18, iconOffsetY + 5, "[F]", GLUT_BITMAP_HELVETICA_10, 215, 115, 35);

    // 3. Water Bottle Slot & Count [B]
    int iconX2 = colX[2] + 6;
    if (texIconWaterBottle != 0) {
        iShowImage(iconX2, iconOffsetY, iconSize, iconSize, texIconWaterBottle);
    } else if (texIconBattery != 0) {
        iShowImage(iconX2, iconOffsetY, iconSize, iconSize, texIconBattery);
    } else {
        iSetColor(0, 150, 220); // Plastic bottle body
        iFilledRectangle(iconX2 + 5, iconOffsetY + 2, 12, 16);
        iSetColor(0, 200, 255); // Water level fill
        iFilledRectangle(iconX2 + 6, iconOffsetY + 3, 10, 11);
        iSetColor(220, 220, 240); // Cap
        iFilledRectangle(iconX2 + 7, iconOffsetY + 18, 8, 4);
    }
    char batStr[16];
    sprintf_s(batStr, sizeof(batStr), "x%d", player.waterBottleCount);
    DrawShadowText(colX[2] + 32, iconOffsetY + 5, batStr, GLUT_BITMAP_HELVETICA_12, 235, 230, 218);
    DrawShadowText(colX[2] + wellW - 19, iconOffsetY + 5, "[B]", GLUT_BITMAP_HELVETICA_10, 215, 115, 35);

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
    DrawKatanaStyleBox(boxX, boxY, boxW, boxH);

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

    // Subtle floating animation using tick count
    double animOffset = sin(GetTickCount() * 0.005) * 3.0;

    // Remove the "[E]" part for drawing specifically if we're making a key icon
    std::string actionStr = promptText;
    bool hasEKey = false;
    if (actionStr.find("[E] ") == 0) {
        actionStr = actionStr.substr(4);
        hasEKey = true;
    }

    int textLen = (int)actionStr.length();
    int textW = textLen * 7; // Approx width for Helvetica 12
    int boxW = textW + (hasEKey ? 45 : 20);
    int boxH = 28;
    int boxX = screenX - (boxW / 2);
    int boxY = screenY + (int)animOffset;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark worn background
    glColor4f(0.05f, 0.06f, 0.08f, 0.85f);
    iFilledRectangle(boxX, boxY, boxW, boxH);

    // Faded grey/metal border
    glColor4f(0.4f, 0.42f, 0.45f, 0.9f);
    iRectangle(boxX, boxY, boxW, boxH);

    int textDrawX = boxX + 10;

    if (hasEKey) {
        // Draw key icon box
        glColor4f(0.8f, 0.82f, 0.85f, 0.9f);
        iFilledRectangle(boxX + 6, boxY + 4, 20, 20);
        glColor4f(0.1f, 0.1f, 0.12f, 1.0f);
        iText(boxX + 12, boxY + 8, (char*)"E", GLUT_BITMAP_HELVETICA_12);
        textDrawX += 24;
    }

    // Readable action text (off white)
    DrawAlphaShadowText(textDrawX, boxY + 8, actionStr.c_str(), GLUT_BITMAP_HELVETICA_12, 230, 235, 240, 1.0, 1);
}

// ============================================================================
// BOSS HEALTH BAR
// ============================================================================
void UI::DrawBossHealthBar(const char* bossName, int bossHp, int bossMaxHp, double displayedHp) {
    if (displayedHp < 0.0) {
        displayedHp = (double)bossHp;
    }

    // Boss HUD dimensions & top-centered positioning
    int barW = 520;
    int barH = 40;
    int barX = 640 - (barW / 2); // 380 (left margin 380, right margin to mission panel 30)
    int barY = 605;              // Positioned downward with generous top margin

    double bossHpPercent = (double)bossHp / (double)bossMaxHp;
    if (bossHpPercent < 0.0) bossHpPercent = 0.0;
    if (bossHpPercent > 1.0) bossHpPercent = 1.0;

    double displayedPercent = displayedHp / (double)bossMaxHp;
    if (displayedPercent < 0.0) displayedPercent = 0.0;
    if (displayedPercent > 1.0) displayedPercent = 1.0;

    // 1. Calculate Inner Health Slot Coordinates (Matching boss_health_bar_frame.png inner opening)
    int fillX = barX + 68;
    int maxFillW = barW - 136;
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
    int skullX = barX - 18;
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
    
    // --- "FINAL BOSS" Text ---
    // Style: Muted red/orange warning color, subtle glow, glitch effect, Times Roman 24 for sharp edges
    void* tagFont = GLUT_BITMAP_TIMES_ROMAN_24;
    int tagW = GetGlutStringWidth(tagFont, "FINAL BOSS");
    int tagX = 640 - (tagW / 2);
    int tagY = 678;
    
    // Subtle glow (multiple offset shadows)
    DrawShadowText(tagX - 1, tagY - 1, "FINAL BOSS", tagFont, 180, 20, 20);
    DrawShadowText(tagX + 1, tagY + 1, "FINAL BOSS", tagFont, 180, 20, 20);
    // Chromatic aberration / glitch effect (cyan/red split)
    DrawShadowText(tagX - 2, tagY, "FINAL BOSS", tagFont, 255, 0, 50);  // Glitch Red
    DrawShadowText(tagX + 2, tagY, "FINAL BOSS", tagFont, 0, 200, 255); // Glitch Cyan
    // Core text
    DrawOutlinedText(tagX, tagY, "FINAL BOSS", tagFont, 220, 80, 60);

    // --- "MUTATED BRUTE" (bossName) Text ---
    // Style: Pale white/grey metallic text, dark shadow, slight infected texture, Helvetica 18
    void* nameFont = GLUT_BITMAP_HELVETICA_18;
    int nameW = GetGlutStringWidth(nameFont, bossName);
    int nameX = 640 - (nameW / 2);
    int nameY = 654;
    
    // Dark deep shadow
    DrawShadowText(nameX + 2, nameY - 2, bossName, nameFont, 10, 10, 15);
    // Infected texture/glow (sickly green/yellow offset)
    DrawShadowText(nameX - 1, nameY + 1, bossName, nameFont, 120, 140, 60);
    DrawShadowText(nameX + 1, nameY - 1, bossName, nameFont, 80, 100, 40);
    // Core metallic text
    DrawOutlinedText(nameX, nameY, bossName, nameFont, 210, 215, 220);

    // 7. Centered Numerical Health & Percentage Text
    // Style: Clean readable font (monospace 8x13), smaller size, military HUD feeling
    char hpStr[64];
    int pct = (int)(bossHpPercent * 100.0);
    sprintf_s(hpStr, sizeof(hpStr), "%d / %d (%d%%)", bossHp, bossMaxHp, pct);
    
    void* hpFont = GLUT_BITMAP_8_BY_13;
    int hpW = GetGlutStringWidth(hpFont, hpStr);
    int hpX = 640 - (hpW / 2);
    int hpY = 632;
    
    // HUD shadow & clean cyan/white color
    DrawShadowText(hpX + 1, hpY - 1, hpStr, hpFont, 20, 30, 30);
    DrawOutlinedText(hpX, hpY, hpStr, hpFont, 180, 220, 220);
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
    if (currentNotification.timer > currentNotification.maxTimer - 0.4) {
        alpha = (currentNotification.maxTimer - currentNotification.timer) / 0.4;
    } else if (currentNotification.timer < 0.5) {
        alpha = currentNotification.timer / 0.5;
    }

    if (alpha <= 0.01) return;

    void* titleFont = GLUT_BITMAP_HELVETICA_10;
    void* msgFont = GLUT_BITMAP_HELVETICA_18;

    int titleW = GetTextWidth(currentNotification.title.c_str(), titleFont);
    int msgW = GetTextWidth(currentNotification.message.c_str(), msgFont);
    int maxW = (titleW > msgW) ? titleW : msgW;

    int boxW = (maxW + 80 < 380) ? 380 : (maxW + 80);
    int boxH = 56;
    int boxX = 640 - (boxW / 2);
    int boxY = 635;

    float a = (float)alpha;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark glass body background
    glColor4f(0.06f, 0.09f, 0.14f, 0.92f * a);
    iFilledRectangle(boxX, boxY, boxW, boxH);

    // Subtle upper glass highlight streak
    glColor4f(0.18f, 0.25f, 0.35f, 0.35f * a);
    iFilledRectangle(boxX + 2, boxY + (boxH / 2), boxW - 4, (boxH / 2) - 2);

    // Outer border frame (Cyan / Tactical outline)
    glColor4f(0.0f, 0.85f, 1.0f, 0.85f * a);
    iRectangle(boxX, boxY, boxW, boxH);

    // Inner gold accent line
    glColor4f(1.0f, 0.84f, 0.0f, 0.65f * a);
    iRectangle(boxX + 2, boxY + 2, boxW - 4, boxH - 4);

    // Corner brackets
    glColor4f(0.85f, 0.45f, 0.15f, 0.90f * a);
    iFilledRectangle(boxX, boxY + boxH - 6, 8, 6);
    iFilledRectangle(boxX + boxW - 8, boxY + boxH - 6, 8, 6);
    iFilledRectangle(boxX, boxY, 8, 6);
    iFilledRectangle(boxX + boxW - 8, boxY, 8, 6);

    // Centered Title Text (Cyan / Tactical)
    int titleX = 640 - (titleW / 2);
    DrawAlphaOutlinedText(titleX, boxY + 34, currentNotification.title.c_str(), titleFont, 0, 230, 255, a);

    // Centered Message Text (Bright white)
    int msgX = 640 - (msgW / 2);
    DrawAlphaShadowText(msgX, boxY + 10, currentNotification.message.c_str(), msgFont, 255, 255, 255, a, 1);
}
