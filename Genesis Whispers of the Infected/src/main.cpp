#define _CRT_SECURE_NO_WARNINGS
#include "../iGraphics.h"
#include "Game.h"

// =B===========================================================================
// GLOBAL GAME ENGINE INSTANCE
// ============================================================================
Game g_game;

// ============================================================================
// iGRAPHICS EVENT CALLBACK DELEGATES
// ============================================================================

/*
iDraw() is invoked continuously by iGraphics to render the active scene.
*/
void iDraw() {
  iClear();
  g_game.Render();
}
/*
fixedUpdate() is called periodically by iSetTimer to run physics/game updates.
*/
void fixedUpdate() { g_game.Update(); }

/*
iMouseMove() is called when the mouse is dragged.
*/
void iMouseMove(int mx, int my) { g_game.HandleMouseMove(mx, my); }

/*
iPassiveMouseMove() is called when mouse moves without button pressed.
*/
void iPassiveMouseMove(int mx, int my) { g_game.HandleMouseMove(mx, my); }

/*
iMouse() is called when mouse buttons are clicked.
*/ 
void iMouse(int button, int state, int mx, int my) {
  g_game.HandleMouseClick(button, state, mx, my);
}

/*
iKeyboard() is called on ASCII key down.
*/
void iKeyboard(unsigned char key) { g_game.HandleKeyPress(key); }

/*
iKeyboardUp() is called on ASCII key release.
*/
void iKeyboardUp(unsigned char key) { g_game.HandleKeyRelease(key); }

/*
iSpecialKeyboard() is called on non-ASCII special key down.
*/
void iSpecialKeyboard(unsigned char key) { g_game.HandleSpecialKeyPress(key); }

/*
iSpecialKeyboardUp() is called on non-ASCII special key release.
*/
void iSpecialKeyboardUp(unsigned char key) {
  g_game.HandleSpecialKeyRelease(key);
}

// ============================================================================
// ENTRY POINT
// ============================================================================
int main() {
  // 1. Initialize iGraphics Window & OpenGL Context FIRST
  iInitialize(1280, 720, "GENESIS: Whispers of the Infected", 16);

  // 2. Hide OS mouse cursor to display custom game cursor
  glutSetCursor(GLUT_CURSOR_NONE);

  // 3. Initialize Game Engine Framework
  g_game.Initialize(1280, 720);
  
  // 4. Start iGraphics Event & Render Loop
  iStart();

  return 0;
}