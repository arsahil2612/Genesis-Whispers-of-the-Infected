#define _CRT_SECURE_NO_WARNINGS
#include "StoryManager.h"
#include "ResourceManager.h"
#include "asset_loader.h"
#include "../iGraphics.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <sstream>
#include <windows.h>

// ============================================================================
// OpenGL Texture Rendering Helper with Alpha Transparency Support
// ============================================================================
static void iShowImageAlpha(int x, int y, int width, int height, unsigned int texture, float alpha) {
    if (texture == 0 || alpha <= 0.001f) return;
    if (alpha > 1.0f) alpha = 1.0f;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBegin(GL_QUADS);

    glTexCoord2f(0.001f, 0.999f);
    glVertex2f((float)x, (float)y);

    glTexCoord2f(0.999f, 0.999f);
    glVertex2f((float)(x + width), (float)y);

    glTexCoord2f(0.999f, 0.001f);
    glVertex2f((float)(x + width), (float)(y + height));

    glTexCoord2f(0.001f, 0.001f);
    glVertex2f((float)x, (float)(y + height));

    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

// ============================================================================
// CONSTRUCTOR & DESTRUCTOR
// ============================================================================
void StoryManager::EnsureWindowFocus() {
    HWND hwnd = GetActiveWindow();
    if (!hwnd) {
        hwnd = FindWindowA(NULL, "GENESIS: Whispers of the Infected");
    }
    if (hwnd) {
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        SetActiveWindow(hwnd);
    }
}

StoryManager::StoryManager()
    : m_currentIndex(0)
    , m_isActive(false)
    , m_isFinished(false)
    , m_phase(PHASE_IMAGE_FADE_IN)
    , m_phaseTimer(0.0f)
    , m_imageAlpha(0.0f)
    , m_textAlpha(0.0f)
    , m_previousTexture(0)
    , m_targetIndex(0)
    , m_animTime(0.0f)
    , m_screenWidth(1280)
    , m_screenHeight(720)
    , m_mouseX(640)
    , m_mouseY(360)
    , m_isMouseDown(false)
    , m_spacePrev(false)
    , m_enterPrev(false)
    , m_escPrev(false)
    , m_rightPrev(false)
    , m_leftPrev(false)
    , m_debounceTimer(0.0f)
    , m_continueGlowTimer(0.0f)
    , m_skipGlowTimer(0.0f)
    , m_isContinueHovered(false)
    , m_isSkipHovered(false)
{
}

StoryManager::~StoryManager() {
    ReleaseResources();
    m_panels.clear();
}

// ============================================================================
// INITIALIZATION & PRELOADING
// ============================================================================
void StoryManager::Initialize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
    m_currentIndex = 0;
    m_targetIndex = 0;
    m_isActive = false;
    m_isFinished = false;
    m_phase = PHASE_IMAGE_FADE_IN;
    m_phaseTimer = 0.0f;
    m_imageAlpha = 0.0f;
    m_textAlpha = 0.0f;
    m_previousTexture = 0;
    m_animTime = 0.0f;
    m_spacePrev = false;
    m_enterPrev = false;
    m_escPrev = false;
    m_rightPrev = false;
    m_leftPrev = false;
    m_debounceTimer = 0.0f;
    m_continueGlowTimer = 0.0f;
    m_skipGlowTimer = 0.0f;
    m_isContinueHovered = false;
    m_isSkipHovered = false;

    EnsureWindowFocus();

    m_panels.clear();

    // 10 Story Panels Configuration - Exact Opening Narration Text
    StoryPanel p1;
    p1.order = 1;
    p1.imagePath = "Assets/Story/Level1_Intro/story_01_before_the_fall.png";
    p1.title = "BEFORE THE FALL";
    p1.text = "Before the collapse...\nThis village was once a place filled with life, hope, and memories.";
    m_panels.push_back(p1);

    StoryPanel p2;
    p2.order = 2;
    p2.imagePath = "Assets/Story/Level1_Intro/story_02_project_genesis.png";
    p2.title = "PROJECT GENESIS";
    p2.text = "Humanity searched for a way to overcome its limits.\nNovaGen created Project Genesis.";
    m_panels.push_back(p2);

    StoryPanel p3;
    p3.order = 3;
    p3.imagePath = "Assets/Story/Level1_Intro/story_03_the_experiment.png";
    p3.title = "THE EXPERIMENT";
    p3.text = "A project designed to evolve humanity.\nA discovery that would change the world forever.";
    m_panels.push_back(p3);

    StoryPanel p4;
    p4.order = 4;
    p4.imagePath = "Assets/Story/Level1_Intro/story_04_the_fall.png";
    p4.title = "THE FALL";
    p4.text = "But the experiment failed.\nThe Genesis virus escaped.";
    m_panels.push_back(p4);

    StoryPanel p5;
    p5.order = 5;
    p5.imagePath = "Assets/Story/Level1_Intro/story_05_the_infected_world.png";
    p5.title = "THE INFECTED WORLD";
    p5.text = "The infected were once human.\nNow they exist only through instinct.";
    m_panels.push_back(p5);

    StoryPanel p6;
    p6.order = 6;
    p6.imagePath = "Assets/Story/Level1_Intro/story_06_evacuation.png";
    p6.title = "EVACUATION";
    p6.text = "As civilization collapsed, survival became the only rule.\nEven humans began fighting against each other.";
    m_panels.push_back(p6);

    StoryPanel p7;
    p7.order = 7;
    p7.imagePath = "Assets/Story/Level1_Intro/story_07_the_separation.png";
    p7.title = "LUNA LOST";
    p7.text = "During the evacuation, Arin lost the person he cared about most.\nHis sister, Luna.";
    m_panels.push_back(p7);

    StoryPanel p8;
    p8.order = 8;
    p8.imagePath = "Assets/Story/Level1_Intro/story_08_the_vow.png";
    p8.title = "THE PROMISE";
    p8.text = "But Arin refused to give up.\nI will find you.";
    m_panels.push_back(p8);

    StoryPanel p9;
    p9.order = 9;
    p9.imagePath = "Assets/Story/Level1_Intro/story_09_the_threat.png";
    p9.title = "EVOLVED THREAT";
    p9.text = "The infection continued evolving.\nSomething far worse was waiting.";
    m_panels.push_back(p9);

    StoryPanel p10;
    p10.order = 10;
    p10.imagePath = "Assets/Story/Level1_Intro/story_10_the_journey_begins.png";
    p10.title = "LEVEL 1 BEGINS";
    p10.text = "Now, Arin begins his journey.\nSearching for Luna.\nSearching for the truth behind Genesis.";
    m_panels.push_back(p10);
}

void StoryManager::PreloadAllStoryTextures() {
    m_panelTextures.clear();
    m_panelTextures.resize(m_panels.size(), 0);

    for (size_t i = 0; i < m_panels.size(); ++i) {
        std::string primaryPath = GetAssetPath(m_panels[i].imagePath);
        unsigned int tex = ResourceManager::GetInstance().GetTexture(primaryPath);

        // Fallback file name checks if primary path does not load
        if (tex == 0) {
            std::string altPath = "";
            if (i == 6) altPath = GetAssetPath("Assets/Story/Level1_Intro/story_07_luna_lost.png");
            else if (i == 7) altPath = GetAssetPath("Assets/Story/Level1_Intro/story_08_the_promise.png");
            else if (i == 8) altPath = GetAssetPath("Assets/Story/Level1_Intro/story_09_evolved_threat.png");
            else if (i == 9) altPath = GetAssetPath("Assets/Story/Level1_Intro/story_10_level1_begin.png");

            if (!altPath.empty()) {
                tex = ResourceManager::GetInstance().GetTexture(altPath);
            }
        }

        m_panelTextures[i] = tex;
        printf("[StoryManager] Preloaded Story Panel %d texture (ID: %u)\n", (int)i + 1, tex);
    }
}

void StoryManager::ReleaseResources() {
    m_panelTextures.clear();
}

// ============================================================================
// LIFECYCLE & STATE CONTROLS
// ============================================================================
void StoryManager::StartStory() {
    m_currentIndex = 0;
    m_targetIndex = 0;
    m_previousTexture = 0;
    m_phase = PHASE_IMAGE_FADE_IN;
    m_phaseTimer = 0.0f;
    m_imageAlpha = 0.0f;
    m_textAlpha = 0.0f;
    m_isActive = true;
    m_isFinished = false;
    m_spacePrev = false;
    m_enterPrev = false;
    m_escPrev = false;
    m_rightPrev = false;
    m_leftPrev = false;
    m_debounceTimer = 0.0f;

    // Preload all 10 story panel textures when story starts
    PreloadAllStoryTextures();

    // Enable OS cursor visibility during Story Mode
    glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

    // Ensure application window has foreground input focus immediately on launch
    EnsureWindowFocus();
}

void StoryManager::NextPanel() {
    if (!m_isActive) return;

    // If current panel is still fading in, instantly make image & text fully visible on first press
    if (m_phase != PHASE_IDLE && m_phase != PHASE_FADE_OUT) {
        m_phase = PHASE_IDLE;
        m_imageAlpha = 1.0f;
        m_textAlpha = 1.0f;
        return;
    }

    if (m_currentIndex < static_cast<int>(m_panels.size()) - 1) {
        m_previousTexture = (m_currentIndex < static_cast<int>(m_panelTextures.size())) ? m_panelTextures[m_currentIndex] : 0;
        m_currentIndex++;
        m_targetIndex = m_currentIndex;
        m_phase = PHASE_IMAGE_FADE_IN;
        m_phaseTimer = 0.0f;
        m_imageAlpha = 0.0f;
        m_textAlpha = 0.0f;
    } else {
        // Final panel: immediately complete story intro and start Level 1
        FinishStory();
    }
}

void StoryManager::PreviousPanel() {
    if (!m_isActive || m_phase == PHASE_FADE_OUT) return;

    if (m_currentIndex > 0) {
        m_targetIndex = m_currentIndex - 1;
        m_phase = PHASE_FADE_OUT;
        m_phaseTimer = 0.0f;
    }
}

void StoryManager::SkipStory() {
    FinishStory();
}

void StoryManager::FinishStory() {
    m_isActive = false;
    m_isFinished = true;
    m_spacePrev = false;
    m_enterPrev = false;
    m_escPrev = false;
    m_rightPrev = false;
    m_leftPrev = false;
    m_debounceTimer = 0.0f;
    ReleaseResources();
}

// ============================================================================
// UPDATE LOOP - CINEMATIC PHASE STATE MACHINE & DIRECT INPUT POLLING
// ============================================================================
void StoryManager::Update(float dt, const bool keys[], const bool specialKeys[]) {
    if (!m_isActive) return;

    m_animTime += dt;

    if (m_debounceTimer > 0.0f) {
        m_debounceTimer -= dt;
        if (m_debounceTimer < 0.0f) m_debounceTimer = 0.0f;
    }

    // Direct key state polling (falling back to Win32 GetAsyncKeyState if GLUT callbacks missed window focus)
    bool spaceCur = (keys && keys[32]) || (keys && keys[' ']) || ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0);
    bool enterCur = (keys && keys[13]) || (keys && keys['\r']) || (keys && keys['\n']) || ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0);
    bool escCur   = (keys && keys[27]) || ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0);
    bool rightCur = (specialKeys && specialKeys[GLUT_KEY_RIGHT]) || (keys && (keys['d'] || keys['D'])) || ((GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0) || ((GetAsyncKeyState('D') & 0x8000) != 0);
    bool leftCur  = (specialKeys && specialKeys[GLUT_KEY_LEFT])  || (keys && (keys['a'] || keys['A'])) || ((GetAsyncKeyState(VK_LEFT) & 0x8000) != 0) || ((GetAsyncKeyState('A') & 0x8000) != 0);

    if (m_debounceTimer <= 0.0f) {
        if (escCur && !m_escPrev) {
            m_skipGlowTimer = 0.25f;
            m_debounceTimer = 0.25f;
            SkipStory();
            return;
        } else if ((spaceCur && !m_spacePrev) || (enterCur && !m_enterPrev) || (rightCur && !m_rightPrev)) {
            m_continueGlowTimer = 0.25f;
            m_debounceTimer = 0.25f;
            NextPanel();
        } else if (leftCur && !m_leftPrev) {
            m_debounceTimer = 0.25f;
            PreviousPanel();
        }
    }

    m_spacePrev = spaceCur;
    m_enterPrev = enterCur;
    m_escPrev = escCur;
    m_rightPrev = rightCur;
    m_leftPrev = leftCur;

    switch (m_phase) {
    case PHASE_IMAGE_FADE_IN:
        m_phaseTimer += dt;
        m_imageAlpha = m_phaseTimer / 1.0f; // 1.0s image fade from black / dissolve
        if (m_imageAlpha > 1.0f) m_imageAlpha = 1.0f;
        m_textAlpha = 0.0f;

        if (m_phaseTimer >= 1.0f) {
            m_imageAlpha = 1.0f;
            m_phase = PHASE_TEXT_DELAY;
            m_phaseTimer = 0.0f;
        }
        break;

    case PHASE_TEXT_DELAY:
        m_phaseTimer += dt;
        m_imageAlpha = 1.0f;
        m_textAlpha = 0.0f;

        if (m_phaseTimer >= 1.0f) { // 1.0s delay after image appears
            m_phase = PHASE_TEXT_FADE_IN;
            m_phaseTimer = 0.0f;
        }
        break;

    case PHASE_TEXT_FADE_IN:
        m_phaseTimer += dt;
        m_imageAlpha = 1.0f;
        m_textAlpha = m_phaseTimer / 0.8f; // Smooth text fade in
        if (m_textAlpha > 1.0f) m_textAlpha = 1.0f;

        if (m_phaseTimer >= 0.8f) {
            m_textAlpha = 1.0f;
            m_phase = PHASE_IDLE;
            m_phaseTimer = 0.0f;
        }
        break;

    case PHASE_IDLE:
        m_imageAlpha = 1.0f;
        m_textAlpha = 1.0f;
        break;

    case PHASE_FADE_OUT:
        m_phaseTimer += dt;

        // Text fades out first before next panel
        m_textAlpha = 1.0f - (m_phaseTimer / 0.3f);
        if (m_textAlpha < 0.0f) m_textAlpha = 0.0f;

        // Image dissolves / fades out
        m_imageAlpha = 1.0f - (m_phaseTimer / 0.6f);
        if (m_imageAlpha < 0.0f) m_imageAlpha = 0.0f;

        if (m_phaseTimer >= 0.6f) {
            if (m_targetIndex < static_cast<int>(m_panels.size())) {
                m_previousTexture = m_panelTextures[m_currentIndex];
                m_currentIndex = m_targetIndex;
                m_phase = PHASE_IMAGE_FADE_IN;
                m_phaseTimer = 0.0f;
                m_imageAlpha = 0.0f;
                m_textAlpha = 0.0f;
            } else {
                FinishStory();
            }
        }
        break;
    }
}

// ============================================================================
// TEXT WRAPPING & STYLING HELPERS
// ============================================================================
std::vector<std::string> StoryManager::WrapText(const std::string& text, size_t maxCharsPerLine) {
    std::vector<std::string> lines;
    std::stringstream lineStream(text);
    std::string lineSegment;

    while (std::getline(lineStream, lineSegment, '\n')) {
        if (lineSegment.empty()) continue;
        std::istringstream wordStream(lineSegment);
        std::string word;
        std::string currentLine = "";

        while (wordStream >> word) {
            if (currentLine.empty()) {
                currentLine = word;
            } else if (currentLine.length() + 1 + word.length() <= maxCharsPerLine) {
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
    return lines;
}

std::string StoryManager::FormatSpacedTitle(const std::string& title) {
    std::string result = "";
    for (size_t i = 0; i < title.length(); ++i) {
        result += title[i];
        if (i < title.length() - 1 && title[i] != ' ') {
            result += " ";
        }
    }
    return result;
}

void StoryManager::RenderTextWithShadow(int x, int y, const char* text, void* font, int r, int g, int b, int shadowOffset) {
    if (m_textAlpha <= 0.001f) return;

    int sr = 0, sg = 0, sb = 0;
    int fr = static_cast<int>(r * m_textAlpha);
    int fg = static_cast<int>(g * m_textAlpha);
    int fb = static_cast<int>(b * m_textAlpha);

    iSetColor(sr, sg, sb);
    iText(x + shadowOffset, y - shadowOffset, (char*)text, font);
    iSetColor(fr, fg, fb);
    iText(x, y, (char*)text, font);
}

void StoryManager::RenderCenteredTextWithShadow(int y, const char* text, void* font, int r, int g, int b, int shadowOffset) {
    if (m_textAlpha <= 0.001f) return;

    int approxCharWidth = (font == GLUT_BITMAP_TIMES_ROMAN_24) ? 12 : 9;
    int textWidth = static_cast<int>(strlen(text)) * approxCharWidth;
    int startX = (m_screenWidth - textWidth) / 2;
    if (startX < 50) startX = 50;

    RenderTextWithShadow(startX, y, text, font, r, g, b, shadowOffset);
}

// ============================================================================
// RENDER HELPERS & CINEMATIC COMPOSITION
// ============================================================================
void StoryManager::RenderCinematicFrame() {
    // Deep dark atmospheric background
    iSetColor(8, 10, 15);
    iFilledRectangle(0, 0, m_screenWidth, m_screenHeight);

    // Subtle background grid accent lines
    iSetColor(16, 20, 30);
    for (int y = 0; y < m_screenHeight; y += 40) {
        iLine(0, y, m_screenWidth, y);
    }

    // Top Cinematic Letterbox Header Bar
    iSetColor(4, 5, 8);
    iFilledRectangle(0, m_screenHeight - 75, m_screenWidth, 75);
    iSetColor(190, 160, 90);
    iLine(0, m_screenHeight - 75, m_screenWidth, m_screenHeight - 75);

    // Bottom Cinematic Letterbox Footer Bar
    iSetColor(4, 5, 8);
    iFilledRectangle(0, 0, m_screenWidth, 95);
    iSetColor(190, 160, 90);
    iLine(0, 95, m_screenWidth, 95);

    // Header Title
    iSetColor(0, 0, 0);
    iText(52, m_screenHeight - 47, (char*)"GENESIS: WHISPERS OF THE INFECTED", GLUT_BITMAP_HELVETICA_18);
    iSetColor(190, 160, 90);
    iText(50, m_screenHeight - 45, (char*)"GENESIS: WHISPERS OF THE INFECTED", GLUT_BITMAP_HELVETICA_18);
    
    char chapterBuf[64];
    sprintf(chapterBuf, "PANEL %02d / %02d", m_currentIndex + 1, static_cast<int>(m_panels.size()));
    iSetColor(0, 0, 0);
    iText(m_screenWidth - 188, m_screenHeight - 47, chapterBuf, GLUT_BITMAP_HELVETICA_12);
    iSetColor(240, 230, 210);
    iText(m_screenWidth - 190, m_screenHeight - 45, chapterBuf, GLUT_BITMAP_HELVETICA_12);
}

void StoryManager::RenderStoryPanel() {
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_panels.size())) return;

    const StoryPanel& panel = m_panels[m_currentIndex];
    unsigned int tex = (m_currentIndex < static_cast<int>(m_panelTextures.size())) ? m_panelTextures[m_currentIndex] : 0;

    // Image frame position and dimensioning (Clean Story Image area)
    int imgW = 760;
    int imgH = 370;
    int imgX = (m_screenWidth - imgW) / 2;
    int imgY = 250;

    // Outer frame gold highlight glow animation
    double pulse = 0.85 + 0.15 * sin(m_animTime * 3.5);
    int goldR = static_cast<int>(190 * pulse);
    int goldG = static_cast<int>(160 * pulse);
    int goldB = static_cast<int>(90 * pulse);

    iSetColor(goldR, goldG, goldB);
    iRectangle(imgX - 4, imgY - 4, imgW + 8, imgH + 8);
    iSetColor(30, 35, 45);
    iRectangle(imgX - 2, imgY - 2, imgW + 4, imgH + 4);

    // 1. RENDER PRELOADED STORY IMAGE WITH DISSOLVE / CROSS-FADE EFFECT (INSTANT SWITCHING)
    if (m_phase == PHASE_IMAGE_FADE_IN && m_previousTexture != 0 && m_currentIndex > 0) {
        // Dissolve transition: Previous image fades out while target image fades in
        float prevAlpha = 1.0f - m_imageAlpha;
        iShowImageAlpha(imgX, imgY, imgW, imgH, m_previousTexture, prevAlpha);
        iShowImageAlpha(imgX, imgY, imgW, imgH, tex, m_imageAlpha);
    } else if (tex != 0) {
        // Smooth image fade from black
        iShowImageAlpha(imgX, imgY, imgW, imgH, tex, m_imageAlpha);
    } else {
        // Dark box fallback if texture fails to load
        iSetColor(20, 25, 35);
        iFilledRectangle(imgX, imgY, imgW, imgH);
    }

    // 2. STORY NARRATION TEXT SYSTEM (Fades in 1s after image appears, fades out before next panel)
    if (m_textAlpha > 0.001f) {
        int boxW = 880;
        int boxH = 135;
        int boxX = (m_screenWidth - boxW) / 2;
        int boxY = 100;

        // Dark semi-transparent container fill and border
        iSetColor(12, 14, 20);
        iFilledRectangle(boxX, boxY, boxW, boxH);
        iSetColor(190, 160, 90); // Muted gold container border
        iRectangle(boxX, boxY, boxW, boxH);
        iRectangle(boxX + 2, boxY + 2, boxW - 4, boxH - 4);

        // Title: Large size, Bold Serif style (Times New Roman / Georgia), Spaced letters, Muted Gold RGB(190, 160, 90)
        std::string spacedTitle = FormatSpacedTitle(panel.title);
        RenderCenteredTextWithShadow(boxY + boxH - 28, spacedTitle.c_str(), GLUT_BITMAP_TIMES_ROMAN_24, 190, 160, 90);

        // Narration Text: Medium size, Warm Off-White / Ivory RGB(240, 230, 210), Auto-wrapped, Smooth Fade-in
        std::vector<std::string> lines = WrapText(panel.text, 68);
        int lineY = (lines.size() >= 3) ? (boxY + boxH - 54) : (boxY + boxH - 58);
        int lineSpacing = (lines.size() >= 3) ? 24 : 26;

        for (size_t i = 0; i < lines.size(); ++i) {
            RenderCenteredTextWithShadow(lineY, lines[i].c_str(), GLUT_BITMAP_HELVETICA_18, 240, 230, 210);
            lineY -= lineSpacing;
        }
    }
}

void StoryManager::RenderCinematicButton(int x, int y, int w, int h, const char* keyLabel, const char* actionLabel, bool isHovered, float glowTimer) {
    if (m_textAlpha <= 0.001f) return;

    float baseAlpha = m_textAlpha * 0.90f;
    float glowFactor = (glowTimer > 0.0f) ? (glowTimer / 0.25f) : 0.0f;

    // Small scale pulsing animation when pressed (+2px outward extension)
    int scaleOffset = static_cast<int>(glowFactor * 2.0f);
    int bx = x - scaleOffset;
    int by = y - scaleOffset;
    int bw = w + (scaleOffset * 2);
    int bh = h + (scaleOffset * 2);

    // 1. Dark Charcoal Semi-Transparent Rectangular Background
    int bgR = static_cast<int>((isHovered ? 26 : 14) * baseAlpha);
    int bgG = static_cast<int>((isHovered ? 30 : 16) * baseAlpha);
    int bgB = static_cast<int>((isHovered ? 42 : 22) * baseAlpha);
    iSetColor(bgR, bgG, bgB);
    iFilledRectangle(bx, by, bw, bh);

    // Subtle chamfered/rounded corners
    iSetColor(8, 10, 14);
    iFilledRectangle(bx, by, 2, 2);
    iFilledRectangle(bx + bw - 2, by, 2, 2);
    iFilledRectangle(bx, by + bh - 2, 2, 2);
    iFilledRectangle(bx + bw - 2, by + bh - 2, 2, 2);

    // 2. Thin Golden Border & Glow Effect
    int borderR = 190;
    int borderG = 160;
    int borderB = 90;

    if (glowTimer > 0.0f) {
        borderR = 255;
        borderG = 235;
        borderB = 160;
    } else if (isHovered) {
        borderR = 255;
        borderG = 215;
        borderB = 110;
    }

    int scaledR = static_cast<int>(borderR * baseAlpha);
    int scaledG = static_cast<int>(borderG * baseAlpha);
    int scaledB = static_cast<int>(borderB * baseAlpha);

    iSetColor(scaledR, scaledG, scaledB);
    iRectangle(bx, by, bw, bh);

    if (isHovered || glowTimer > 0.0f) {
        iRectangle(bx - 1, by - 1, bw + 2, bh + 2);
    }

    // 3. Button Inside Text Labels
    // Line 1: [ KEY ] in Muted Gold
    int approxCharWidthKey = 8;
    int keyWidth = static_cast<int>(strlen(keyLabel)) * approxCharWidthKey;
    int keyX = bx + (bw - keyWidth) / 2;
    int keyY = by + bh - 19;

    iSetColor(0, 0, 0);
    iText(keyX + 1, keyY - 1, (char*)keyLabel, GLUT_BITMAP_HELVETICA_12);
    iSetColor(scaledR, scaledG, scaledB);
    iText(keyX, keyY, (char*)keyLabel, GLUT_BITMAP_HELVETICA_12);

    // Line 2: Action Name in Soft White (Serif Bold / Cinematic style font)
    int approxCharWidthAct = 10;
    int actWidth = static_cast<int>(strlen(actionLabel)) * approxCharWidthAct;
    int actX = bx + (bw - actWidth) / 2;
    int actY = by + 10;

    int textR = static_cast<int>((isHovered ? 255 : 240) * baseAlpha);
    int textG = static_cast<int>((isHovered ? 245 : 230) * baseAlpha);
    int textB = static_cast<int>((isHovered ? 230 : 210) * baseAlpha);

    iSetColor(0, 0, 0);
    iText(actX + 1, actY - 1, (char*)actionLabel, GLUT_BITMAP_TIMES_ROMAN_24);
    iSetColor(textR, textG, textB);
    iText(actX, actY, (char*)actionLabel, GLUT_BITMAP_TIMES_ROMAN_24);
}

void StoryManager::RenderControlsAndHUD() {
    if (m_textAlpha <= 0.001f) return;

    // 1. Skip Button Element (Bottom-Left)
    RenderCinematicButton(40, 18, 110, 56, "[ ESC ]", "Skip", m_isSkipHovered, m_skipGlowTimer);

    // 2. Continue Button Element (Bottom-Right)
    const char* actionText = (m_currentIndex == static_cast<int>(m_panels.size()) - 1) ? "Begin Level 1" : "Continue";
    RenderCinematicButton(m_screenWidth - 230, 18, 190, 56, "[ SPACE / ENTER ]", actionText, m_isContinueHovered, m_continueGlowTimer);

    // 3. Minimal Step Indicator Dots (Centered at bottom)
    float baseOpacity = 0.70f * m_textAlpha;
    int total = static_cast<int>(m_panels.size());
    int dotStartX = (m_screenWidth - (total * 20)) / 2;
    int dotY = 38;

    for (int i = 0; i < total; ++i) {
        int dotX = dotStartX + i * 20;
        if (i == m_currentIndex) {
            iSetColor(static_cast<int>(190 * baseOpacity), static_cast<int>(160 * baseOpacity), static_cast<int>(90 * baseOpacity));
            iFilledCircle(dotX, dotY, 4);
        } else {
            iSetColor(static_cast<int>(60 * baseOpacity), static_cast<int>(65 * baseOpacity), static_cast<int>(75 * baseOpacity));
            iFilledCircle(dotX, dotY, 2);
        }
    }
}

void StoryManager::RenderCursor() {
    int cx = m_mouseX;
    int cy = m_mouseY;

    // Custom high-contrast pointer cursor for Story Mode
    iSetColor(190, 160, 90);
    iCircle(cx, cy, 6);
    iSetColor(0, 240, 255);
    iCircle(cx, cy, 4);
    iSetColor(255, 255, 255);
    iFilledCircle(cx, cy, 2);

    // Crosshair pointer accents
    iSetColor(190, 160, 90);
    iLine(cx - 10, cy, cx - 4, cy);
    iLine(cx + 4, cy, cx + 10, cy);
    iLine(cx, cy - 10, cx, cy - 4);
    iLine(cx, cy + 4, cx, cy + 10);
}

// ============================================================================
// MAIN RENDER METHOD
// ============================================================================
void StoryManager::Render() {
    if (!m_isActive) return;

    RenderCinematicFrame();
    RenderStoryPanel();
    RenderControlsAndHUD();
    RenderCursor();
}

// ============================================================================
// INPUT HANDLING DELEGATES
// ============================================================================
void StoryManager::HandleKeyPress(unsigned char key) {
    if (!m_isActive) return;

    if (key == 27 || key == 0x1B) {
        if (m_debounceTimer <= 0.0f) {
            m_skipGlowTimer = 0.25f;
            m_debounceTimer = 0.25f;
            SkipStory();
        }
    } else if (key == 32 || key == ' ' || key == 13 || key == '\r' || key == '\n' || key == 10 || key == 'd' || key == 'D') {
        if (m_debounceTimer <= 0.0f) {
            m_continueGlowTimer = 0.25f;
            m_debounceTimer = 0.25f;
            NextPanel();
        }
    }
}

void StoryManager::HandleSpecialKeyPress(unsigned char key) {
    if (!m_isActive) return;

    if (key == GLUT_KEY_RIGHT) {
        if (m_debounceTimer <= 0.0f) {
            m_continueGlowTimer = 0.25f;
            m_debounceTimer = 0.25f;
            NextPanel();
        }
    } else if (key == GLUT_KEY_LEFT) {
        if (m_debounceTimer <= 0.0f) {
            m_debounceTimer = 0.25f;
            PreviousPanel();
        }
    }
}

void StoryManager::HandleMouseClick(int button, int state, int mx, int my) {
    if (!m_isActive) return;

    m_mouseX = mx;
    m_mouseY = my;

    if (button == 0 && state == 1) { // Left Mouse Click
        if (m_isSkipHovered) {
            m_skipGlowTimer = 0.25f;
            SkipStory();
        } else if (m_isContinueHovered) {
            m_continueGlowTimer = 0.25f;
            NextPanel();
        } else {
            NextPanel();
        }
    }
}

void StoryManager::HandleMouseMove(int mx, int my) {
    m_mouseX = mx;
    m_mouseY = my;

    // Skip Button hover bounds (Bottom-Left)
    m_isSkipHovered = (mx >= 40 && mx <= 150 && my >= 18 && my <= 74);

    // Continue Button hover bounds (Bottom-Right)
    int btnX = m_screenWidth - 230;
    m_isContinueHovered = (mx >= btnX && mx <= btnX + 190 && my >= 18 && my <= 74);
}
