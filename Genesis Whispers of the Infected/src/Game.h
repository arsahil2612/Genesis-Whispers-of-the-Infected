#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include "game_manager.h"
#include "ResourceManager.h"

// ============================================================================
// GAME CLASS (CORE FRAMEWORK & ENGINE MANAGER)
// ============================================================================
class Game {
private:
    int m_screenWidth;
    int m_screenHeight;
    bool m_isRunning;

    // Embedded Core Game Manager (Arin, World Map, Enemies, UI, Menus, Audio)
    GameManager m_gameManager;

    // High precision Delta Time tracking using Windows Performance Counter
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_lastTime;
    float m_deltaTime;

public:
    Game();
    ~Game();

    // Core Framework Methods
    void Initialize(int width, int height);
    void Update();
    void Render();

    // High precision Delta Time
    float CalculateDeltaTime();
    float GetDeltaTime() const { return m_deltaTime; }

    // Screen Dimensions
    int GetScreenWidth() const { return m_screenWidth; }
    int GetScreenHeight() const { return m_screenHeight; }

    // Input Event Handlers
    void HandleKeyPress(unsigned char key);
    void HandleKeyRelease(unsigned char key);
    void HandleSpecialKeyPress(unsigned char key);
    void HandleSpecialKeyRelease(unsigned char key);
    void HandleMouseClick(int button, int state, int mx, int my);
    void HandleMouseMove(int mx, int my);
};

#endif // GAME_H
