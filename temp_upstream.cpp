#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include "game_manager.h"
#include "ResourceManager.h"
#include "igraphics_declarations.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

// Active Dialogue Text buffer
static char g_dialogueSpeaker[64] = "";
static char g_dialogueText[512] = "";

// =====================================================================
