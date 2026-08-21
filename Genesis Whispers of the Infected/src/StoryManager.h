#ifndef STORY_MANAGER_H
#define STORY_MANAGER_H

#include <string>
#include <vector>

// ============================================================================
// Story Panel Struct
// Defines individual cinematic panel data: order index, image path, title, text
// ============================================================================
struct StoryPanel {
    int order;
    std::string imagePath;
    std::string title;
    std::string text;
};

enum StoryPhase {
    PHASE_IMAGE_FADE_IN,  // 1.0s image fade from black / dissolve effect
    PHASE_TEXT_DELAY,     // 1.0s delay with image visible, text hidden
    PHASE_TEXT_FADE_IN,   // Smooth text fade-in
    PHASE_IDLE,           // Idle awaiting player SPACE / ENTER / Next click
    PHASE_FADE_OUT        // Fade out text & image before next panel
};

// ============================================================================
// StoryManager Class
// Standalone Level 1 Cinematic Intro System
// ============================================================================
class StoryManager {
private:
    std::vector<StoryPanel> m_panels;
    std::vector<unsigned int> m_panelTextures;
    int m_currentIndex;
    bool m_isActive;
    bool m_isFinished;

    // Transition & Cinematic Animation Phase State Machine
    StoryPhase m_phase;
    float m_phaseTimer;
    float m_imageAlpha;
    float m_textAlpha;
    unsigned int m_previousTexture;
    int m_targetIndex;
    float m_animTime;

    // Screen Resolution & Input tracking
    int m_screenWidth;
    int m_screenHeight;
    int m_mouseX;
    int m_mouseY;
    bool m_isMouseDown;

    // Key press edge detection flags
    bool m_spacePrev;
    bool m_enterPrev;
    bool m_escPrev;

    // Cinematic Button interaction animation & hover state tracking
    float m_continueGlowTimer;
    float m_skipGlowTimer;
    bool m_isContinueHovered;
    bool m_isSkipHovered;

    // Internal helper methods
    void PreloadAllStoryTextures();
    void ReleaseResources();
    void RenderCinematicFrame();
    void RenderStoryPanel();
    void RenderControlsAndHUD();
    void RenderCinematicButton(int x, int y, int w, int h, const char* keyLabel, const char* actionLabel, bool isHovered, float glowTimer);
    void RenderCursor();
    void RenderTextWithShadow(int x, int y, const char* text, void* font, int r, int g, int b, int shadowOffset = 2);
    void RenderCenteredTextWithShadow(int y, const char* text, void* font, int r, int g, int b, int shadowOffset = 2);
    std::vector<std::string> WrapText(const std::string& text, size_t maxCharsPerLine);
    std::string FormatSpacedTitle(const std::string& title);

public:
    StoryManager();
    ~StoryManager();

    // Core System Lifecycle
    void Initialize(int width = 1280, int height = 720);
    void StartStory();
    void NextPanel();
    void PreviousPanel();
    void SkipStory();
    void FinishStory();

    // Update & Render Loops
    void Update(float dt, const bool keys[] = NULL, const bool specialKeys[] = NULL);
    void Render();

    // Event Input Handlers
    void HandleKeyPress(unsigned char key);
    void HandleSpecialKeyPress(unsigned char key);
    void HandleMouseClick(int button, int state, int mx, int my);
    void HandleMouseMove(int mx, int my);

    // Getters
    bool IsActive() const { return m_isActive; }
    bool IsFinished() const { return m_isFinished; }
    int GetCurrentIndex() const { return m_currentIndex; }
    int GetTotalPanels() const { return static_cast<int>(m_panels.size()); }
};

#endif // STORY_MANAGER_H
