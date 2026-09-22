#include "Game.h"
#include "ResourceManager.h"
#include "../iGraphics.h"
#include <stdio.h>

// ============================================================================
// CONSTRUCTOR & DESTRUCTOR
// ============================================================================
Game::Game() 
    : m_screenWidth(1280)
    , m_screenHeight(720)
    , m_isRunning(true)
    , m_engineState(GAME_STATE_STORY)
    , m_deltaTime(0.016f)
{
    m_frequency.QuadPart = 0;
    m_lastTime.QuadPart = 0;
}

Game::~Game() {
}

// ============================================================================
// INITIALIZATION
// ============================================================================
void Game::Initialize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;

    // Initialize high precision timer
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);

    // Initialize GameManager (Loads Level 1 map, Arin player character, backgrounds, props, enemies, sound)
    m_gameManager.Initialize();
    m_gameManager.SetCurrentState(STATE_STORY);

    // Initialize Level 1 Cinematic Story Introduction System
    m_storyManager.Initialize(width, height);
    m_storyManager.StartStory();
    m_engineState = GAME_STATE_STORY;

    // Force window input focus immediately on game initialization
    HWND hwnd = GetActiveWindow();
    if (!hwnd) {
        hwnd = FindWindowA(NULL, "GENESIS: Whispers of the Infected");
    }
    if (hwnd) {
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        SetActiveWindow(hwnd);
    }

    printf("[GENESIS Engine] Game Engine Initialized. Initial State: GAME_STATE_STORY. Screen: %dx%d\n", width, height);
}

// ============================================================================
// DELTA TIME CALCULATION
// ============================================================================
float Game::CalculateDeltaTime() {
    if (m_frequency.QuadPart == 0) return 0.016f;

    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    LONGLONG elapsedTicks = currentTime.QuadPart - m_lastTime.QuadPart;
    m_deltaTime = static_cast<float>(elapsedTicks) / static_cast<float>(m_frequency.QuadPart);
    m_lastTime = currentTime;

    // Clamp delta time to avoid large spikes during window moving/freezing
    if (m_deltaTime < 0.0001f) m_deltaTime = 0.0001f;
    if (m_deltaTime > 0.1f)    m_deltaTime = 0.1f;

    return m_deltaTime;
}

// ============================================================================
// UPDATE LOOP
// ============================================================================
void Game::Update() {
    float dt = CalculateDeltaTime();

    // Query active keypress state arrays from iGraphics
    bool keys[512] = { false };
    bool specialKeys[512] = { false };

    for (int i = 0; i < 512; ++i) {
        // Sync spacebar with physical OS key state to prevent dropped GLUT keyUp events
        if (i == 32 || i == ' ') {
            if ((GetAsyncKeyState(VK_SPACE) & 0x8000) == 0) {
                keyPressed[i] = 0;
            }
        }
        // Sync uppercase ASCII letters A-Z with physical OS key state
        if (i >= 'A' && i <= 'Z') {
            if ((GetAsyncKeyState(i) & 0x8000) == 0) {
                keyPressed[i] = 0;
            }
        }
        // Sync lowercase ASCII letters a-z with physical OS key state
        if (i >= 'a' && i <= 'z') {
            char upperKey = (char)(i - 'a' + 'A');
            if ((GetAsyncKeyState(upperKey) & 0x8000) == 0) {
                keyPressed[i] = 0;
            }
        }
        keys[i] = (keyPressed[i] != 0);
        specialKeys[i] = (specialKeyPressed[i] != 0);
    }

    // GAME_STATE_STORY: StoryManager receives priority keyboard input every frame. Gameplay inputs disabled.
    if (m_engineState == GAME_STATE_STORY) {
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW); // Visible cursor during Story Mode

        m_storyManager.Update(dt, keys, specialKeys);

        if (m_storyManager.IsFinished()) {
            // Story Completion Transition: Restore original gameplay cursor behavior & enable gameplay keyboard input
            m_engineState = GAME_STATE_PLAYING;
            m_gameManager.SetCurrentState(STATE_PLAYING);
            glutSetCursor(GLUT_CURSOR_NONE); // Restore original gameplay cursor settings
            printf("[GENESIS Engine] Story Intro finished. Restored normal gameplay keyboard input & transitioned to GAME_STATE_PLAYING.\n");
        }
        return; // Gameplay input and update systems 100% DISABLED during Story Mode!
    }

    // GAME_STATE_PLAYING: Gameplay active (Arin movement, combat, enemy AI, normal cursor)
    m_gameManager.Update(dt, keys, specialKeys);
}

// ============================================================================
// RENDER LOOP
// ============================================================================
void Game::Render() {
    // Render Story Introduction panel if in GAME_STATE_STORY
    if (m_engineState == GAME_STATE_STORY) {
        m_storyManager.Render();
        return;
    }

    // Delegate rendering to GameManager (Backgrounds, Arin, Enemies, Props, Tiles, UI, Menu)
    m_gameManager.Render();
}

// ============================================================================
// INPUT HANDLING
// ============================================================================
void Game::HandleKeyPress(unsigned char key) {
    if (m_engineState == GAME_STATE_STORY) {
        m_storyManager.HandleKeyPress(key);

        // Reset keyPressed state in iGraphics to prevent double triggering
        keyPressed[key] = 0;

        if (m_storyManager.IsFinished()) {
            m_engineState = GAME_STATE_PLAYING;
            m_gameManager.SetCurrentState(STATE_PLAYING);
            glutSetCursor(GLUT_CURSOR_NONE);
        }
        return; // Gameplay input handlers 100% DISABLED during Story Mode!
    }
    m_gameManager.HandleKeyPress(key);
}

void Game::HandleKeyRelease(unsigned char key) {
    keyPressed[key] = 0;
    if (key >= 'A' && key <= 'Z') {
        keyPressed[key + ('a' - 'A')] = 0;
    } else if (key >= 'a' && key <= 'z') {
        keyPressed[key - ('a' - 'A')] = 0;
    }
}

void Game::HandleSpecialKeyPress(unsigned char key) {
    if (m_engineState == GAME_STATE_STORY) {
        if (key == GLUT_KEY_RIGHT) {
            m_storyManager.NextPanel();
        } else if (key == GLUT_KEY_LEFT) {
            m_storyManager.PreviousPanel();
        }
        if (m_storyManager.IsFinished()) {
            m_engineState = GAME_STATE_PLAYING;
            m_gameManager.SetCurrentState(STATE_PLAYING);
            glutSetCursor(GLUT_CURSOR_NONE);
        }
        return; // Gameplay input handlers 100% DISABLED during Story Mode!
    }
    m_gameManager.HandleSpecialKeyPress(key);
}

void Game::HandleSpecialKeyRelease(unsigned char key) {}

void Game::HandleMouseClick(int button, int state, int mx, int my) {
    if (m_engineState == GAME_STATE_STORY) {
        m_storyManager.HandleMouseClick(button, state, mx, my);
        if (m_storyManager.IsFinished()) {
            m_engineState = GAME_STATE_PLAYING;
            m_gameManager.SetCurrentState(STATE_PLAYING);
            glutSetCursor(GLUT_CURSOR_NONE); // Restore original gameplay cursor settings
        }
        return;
    }
    m_gameManager.HandleMouseClick(button, state, mx, my);
}

void Game::HandleMouseMove(int mx, int my) {
    if (m_engineState == GAME_STATE_STORY) {
        m_storyManager.HandleMouseMove(mx, my);
        return;
    }
    m_gameManager.HandleMouseMove(mx, my);
}
