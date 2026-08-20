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

    printf("[GENESIS Engine] Game Engine Initialized with full render, asset, & gameplay pipeline. Screen: %dx%d\n", width, height);
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
        keys[i] = (keyPressed[i] != 0);
        specialKeys[i] = (specialKeyPressed[i] != 0);
    }

    // Delegate physics and world state updates to GameManager
    m_gameManager.Update(keys, specialKeys);
}

// ============================================================================
// RENDER LOOP
// ============================================================================
void Game::Render() {
    // Delegate rendering to GameManager (Backgrounds, Arin, Enemies, Props, Tiles, UI, Menu)
    m_gameManager.Render();
}

// ============================================================================
// INPUT HANDLING
// ============================================================================
void Game::HandleKeyPress(unsigned char key) {
    m_gameManager.HandleKeyPress(key);
}

void Game::HandleKeyRelease(unsigned char key) {}

void Game::HandleSpecialKeyPress(unsigned char key) {
    m_gameManager.HandleSpecialKeyPress(key);
}

void Game::HandleSpecialKeyRelease(unsigned char key) {}

void Game::HandleMouseClick(int button, int state, int mx, int my) {
    m_gameManager.HandleMouseClick(button, state, mx, my);
}

void Game::HandleMouseMove(int mx, int my) {
    m_gameManager.HandleMouseMove(mx, my);
}
