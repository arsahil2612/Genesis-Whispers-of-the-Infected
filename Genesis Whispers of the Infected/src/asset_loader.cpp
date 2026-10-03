#define _CRT_SECURE_NO_WARNINGS
#include "asset_loader.h"
#include "igraphics_declarations.h"
#include <cstdio>
#include <windows.h>
#include <mmsystem.h>
#include <map>
#include <vector>

#pragma comment(lib, "winmm.lib")

std::string GetAssetPath(const std::string &relativePath) {
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);

    std::vector<std::string> candidates;
    candidates.push_back(relativePath);
    candidates.push_back("Genesis Whispers of the Infected/" + relativePath);
    candidates.push_back("../" + relativePath);
    candidates.push_back("../Genesis Whispers of the Infected/" + relativePath);

    if (relativePath.find("MainMenu") != std::string::npos || relativePath.find("Main Menu") != std::string::npos) {
        size_t lastSlash = relativePath.find_last_of("/\\");
        std::string filename = (lastSlash != std::string::npos) ? relativePath.substr(lastSlash + 1) : relativePath;
        candidates.push_back("Genesis Whispers of the Infected/Assets/UI/Main Menu/" + filename);
        candidates.push_back("../Genesis Whispers of the Infected/Assets/UI/Main Menu/" + filename);
        candidates.push_back("Assets/UI/Main Menu/" + filename);
        candidates.push_back("../Assets/UI/Main Menu/" + filename);
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        FILE* f = fopen(candidates[i].c_str(), "rb");
        if (f) {
            fclose(f);
            return candidates[i];
        }
    }

    return relativePath;
}

void PlayAudioFile(const std::string &primaryRelativePath, const std::string &fallbackRelativePath) {
    // Safety Guard: Ignore non-audio files (e.g. .png, .jpg, .bmp) so Windows PlaySound never launches image viewers
    if (primaryRelativePath.find(".png") != std::string::npos || primaryRelativePath.find(".jpg") != std::string::npos ||
        fallbackRelativePath.find(".png") != std::string::npos || fallbackRelativePath.find(".jpg") != std::string::npos) {
        return;
    }

    static std::map<std::string, std::string> loadedAliases;
    static std::map<std::string, std::string> loadedFullPaths;
    static int nextAliasId = 0;

    std::string alias;
    std::string fullPathStr;

    if (loadedAliases.find(primaryRelativePath) != loadedAliases.end()) {
        alias = loadedAliases[primaryRelativePath];
        if (alias == "FAILED") {
            fullPathStr = loadedFullPaths[primaryRelativePath];
            PlaySoundA(fullPathStr.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            return;
        }
    } else {
        std::string path = GetAssetPath(primaryRelativePath);
        FILE* fTest = NULL;
        if (fopen_s(&fTest, path.c_str(), "rb") != 0 || fTest == NULL) {
            if (!fallbackRelativePath.empty()) {
                path = GetAssetPath(fallbackRelativePath);
            }
        } else {
            fclose(fTest);
        }

        char fullPath[MAX_PATH];
        if (_fullpath(fullPath, path.c_str(), MAX_PATH) == NULL) {
            strcpy_s(fullPath, sizeof(fullPath), path.c_str());
        }
        fullPathStr = fullPath;

        char newAlias[32];
        sprintf_s(newAlias, sizeof(newAlias), "snd_alias_%d", nextAliasId++);
        alias = newAlias;

        char cmdOpen[MAX_PATH + 128];
        sprintf_s(cmdOpen, sizeof(cmdOpen), "open \"%s\" type mpegvideo alias %s", fullPath, alias.c_str());
        MCIERROR err = mciSendStringA(cmdOpen, NULL, 0, NULL);
        if (err != 0) {
            loadedAliases[primaryRelativePath] = "FAILED";
            loadedFullPaths[primaryRelativePath] = fullPathStr;
            PlaySoundA(fullPathStr.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            return;
        }
        loadedAliases[primaryRelativePath] = alias;
    }

    char cmdPlay[64];
    sprintf_s(cmdPlay, sizeof(cmdPlay), "play %s from 0", alias.c_str());
    mciSendStringA(cmdPlay, NULL, 0, NULL);
}

// ============================================================================
// MAIN MENU BACKGROUND MUSIC CONTROLLER
// ============================================================================
static bool s_menuMusicPlaying = false;

void PlayMenuMusic() {
    if (s_menuMusicPlaying) return;

    const char* candidates[] = {
        "UI/MainMenu/main_menu_music.wav",
        "Assets/UI/Main Menu/Audio/main menu music.mp3",
        "Assets/UI/Main Menu/main_menu_music.wav",
        "Assets/UI/Main Menu/Audio/main_menu_music.wav",
        "UI/MainMenu/main menu music.mp3",
        "Audios/background.mp3"
    };

    std::string resolvedPath = "";
    for (int i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        std::string p = GetAssetPath(candidates[i]);
        FILE* f = fopen(p.c_str(), "rb");
        if (f) {
            fclose(f);
            resolvedPath = p;
            break;
        }
    }

    if (resolvedPath.empty()) {
        printf("[GENESIS Audio] WARNING: Could not find menu music file.\n");
        return;
    }

    char fullPath[MAX_PATH];
    if (_fullpath(fullPath, resolvedPath.c_str(), MAX_PATH) == NULL) {
        strcpy_s(fullPath, sizeof(fullPath), resolvedPath.c_str());
    }

    mciSendStringA("stop menu_bgm", NULL, 0, NULL);
    mciSendStringA("close menu_bgm", NULL, 0, NULL);

    char cmdOpen[MAX_PATH + 128];
    sprintf_s(cmdOpen, sizeof(cmdOpen), "open \"%s\" type mpegvideo alias menu_bgm", fullPath);
    MCIERROR err = mciSendStringA(cmdOpen, NULL, 0, NULL);
    if (err != 0) {
        sprintf_s(cmdOpen, sizeof(cmdOpen), "open \"%s\" alias menu_bgm", fullPath);
        err = mciSendStringA(cmdOpen, NULL, 0, NULL);
    }

    if (err == 0) {
        mciSendStringA("setaudio menu_bgm volume to 500", NULL, 0, NULL);
        mciSendStringA("play menu_bgm repeat", NULL, 0, NULL);
        s_menuMusicPlaying = true;
        printf("[GENESIS Audio] Started Main Menu music loop from '%s'\n", fullPath);
    } else {
        PlaySoundA(fullPath, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT);
        s_menuMusicPlaying = true;
        printf("[GENESIS Audio] Started Main Menu PlaySound loop from '%s'\n", fullPath);
    }
}

void StopMenuMusic() {
    if (s_menuMusicPlaying) {
        mciSendStringA("stop menu_bgm", NULL, 0, NULL);
        mciSendStringA("close menu_bgm", NULL, 0, NULL);
        PlaySoundA(NULL, NULL, 0);
        s_menuMusicPlaying = false;
        printf("[GENESIS Audio] Stopped Main Menu music.\n");
    }
}

// Static caching tables for background textures
static unsigned int g_level1BgTextures[10] = { 0 };
static int g_level1BgSourceIndex[10] = { 0 };
static unsigned int g_level2BgTextures[10] = { 0 };
static unsigned int g_level3CommonTexture = 0;
static unsigned int g_level3EasyTextures[10] = { 0 };
static unsigned int g_level3HardTextures[10] = { 0 };

// ============================================================================
// Background Slice Index Resolution
// ============================================================================
int GetLevel1BackgroundFileIndex(int sliceIndex, bool bossDefeated) {
    static const int kLevel1BgRemap[10] = {
        1,   // 0: Spawn Area / Arin's destroyed home
        2,   // 1: Destroyed House interior ruins
        3,   // 2: Village Street
        4,   // 3: Village Square
        5,   // 4: Abandoned Market
        6,   // 5: Raider Camp
        7,   // 6: Abandoned Church
        8,   // 7: Quarantine Zone
        7,   // 8: Broken Bridge
        8    // 9: Mini Boss Arena (Exit Gate after victory)
    };

    if (sliceIndex < 0 || sliceIndex >= 10) {
        return 1;
    }

    if (sliceIndex == 9 && bossDefeated) {
        return 1;  // Steel exit gate opens after the Mutated Brute falls
    }

    return kLevel1BgRemap[sliceIndex];
}

// ============================================================================
// Background Texture Loading & Caching
// ============================================================================
unsigned int LoadLevel1BackgroundTexture(int sliceIndex, bool bossDefeated) {
    if (sliceIndex < 0 || sliceIndex >= 10) {
        return 0;
    }

    int sourceIndex = GetLevel1BackgroundFileIndex(sliceIndex, bossDefeated);

    if (g_level1BgTextures[sliceIndex] != 0 && g_level1BgSourceIndex[sliceIndex] == sourceIndex) {
        return g_level1BgTextures[sliceIndex];
    }

    char path[160];
    sprintf(path, "Assets/Backgrounds/Level1/genesis_bg_%d.png", sourceIndex);
    std::string resolved = GetAssetPath(path);

    unsigned int tex = iLoadImage((char*)resolved.c_str());
    if (tex == 0) {
        // Fallback check for Assets/Background/Level1/ directory
        sprintf(path, "Assets/Background/Level1/genesis_bg_%d.png", sourceIndex);
        resolved = GetAssetPath(path);
        tex = iLoadImage((char*)resolved.c_str());
    }

    // Safety fallback: ensure a valid texture is always returned so dark theme gaps never appear
    if (tex == 0) {
        resolved = GetAssetPath("Assets/Backgrounds/Level1/genesis_bg_1.png");
        tex = iLoadImage((char*)resolved.c_str());
    }

    g_level1BgTextures[sliceIndex] = tex;
    g_level1BgSourceIndex[sliceIndex] = sourceIndex;
    return tex;
}

unsigned int LoadLevel2BackgroundTexture(int sliceIndex) {
    if (sliceIndex < 0 || sliceIndex >= 10) {
        return 0;
    }

    if (g_level2BgTextures[sliceIndex] != 0) {
        return g_level2BgTextures[sliceIndex];
    }

    char path[160];
    sprintf(path, "Assets/Backgrounds/Level2/bg_%02d.png", sliceIndex + 1);
    std::string resolved = GetAssetPath(path);

    unsigned int tex = iLoadImage((char*)resolved.c_str());
    if (tex == 0) {
        // Fallback check for genesis_bg format
        sprintf(path, "Assets/Backgrounds/Level2/genesis_bg_%d.png", sliceIndex + 1);
        resolved = GetAssetPath(path);
        tex = iLoadImage((char*)resolved.c_str());
    }

    // Fallback to Level 1 background slice if missing
    if (tex == 0) {
        tex = LoadLevel1BackgroundTexture(sliceIndex, false);
    }

    g_level2BgTextures[sliceIndex] = tex;
    return tex;
}

unsigned int LoadLevel3BackgroundTexture(int route, int sliceIndex) {
    // route: 0 = Common BG, 1 = Easy Route, 2 = Hard Route
    if (route == 0) {
        if (g_level3CommonTexture != 0) return g_level3CommonTexture;
        std::string resolved = GetAssetPath("Assets/Backgrounds/Level3/Common Background/common_bg.png");
        g_level3CommonTexture = iLoadImage((char*)resolved.c_str());
        return g_level3CommonTexture;
    }

    if (sliceIndex < 0 || sliceIndex >= 10) return 0;

    if (route == 1) { // Easy Route
        if (g_level3EasyTextures[sliceIndex] != 0) return g_level3EasyTextures[sliceIndex];

        std::string path;
        if (sliceIndex >= 0 && sliceIndex <= 4) {
            char b[128];
            sprintf(b, "Assets/Backgrounds/Level3/Easy Route/easy_bg%d.png", sliceIndex + 1);
            path = b;
        } else if (sliceIndex == 5) {
            path = "Assets/Backgrounds/Level3/Easy Route/final_arena.png";
        } else if (sliceIndex == 6) {
            path = "Assets/Backgrounds/Level3/Easy Route/escape_door.png";
        } else {
            path = "Assets/Backgrounds/Level3/Easy Route/ending_bg.png";
        }

        std::string resolved = GetAssetPath(path);
        unsigned int tex = iLoadImage((char*)resolved.c_str());
        if (tex == 0) {
            tex = LoadLevel2BackgroundTexture(sliceIndex);
        }
        g_level3EasyTextures[sliceIndex] = tex;
        return tex;
    } else { // Hard Route (route == 2)
        if (g_level3HardTextures[sliceIndex] != 0) return g_level3HardTextures[sliceIndex];

        std::string path;
        if (sliceIndex >= 0 && sliceIndex <= 5) {
            char b[128];
            sprintf(b, "Assets/Backgrounds/Level3/Hard Route/hard_bg%d.png", sliceIndex + 1);
            path = b;
        } else if (sliceIndex == 6) {
            path = "Assets/Backgrounds/Level3/Hard Route/final_arena.png";
        } else if (sliceIndex == 7) {
            path = "Assets/Backgrounds/Level3/Hard Route/escape_door.png";
        } else {
            path = "Assets/Backgrounds/Level3/Hard Route/ending_bg.png";
        }

        std::string resolved = GetAssetPath(path);
        unsigned int tex = iLoadImage((char*)resolved.c_str());
        if (tex == 0) {
            tex = LoadLevel2BackgroundTexture(sliceIndex);
        }
        g_level3HardTextures[sliceIndex] = tex;
        return tex;
    }
}

void ClearBackgroundCache() {
    for (int i = 0; i < 10; ++i) {
        g_level1BgTextures[i] = 0;
        g_level1BgSourceIndex[i] = 0;
        g_level2BgTextures[i] = 0;
        g_level3EasyTextures[i] = 0;
        g_level3HardTextures[i] = 0;
    }
    g_level3CommonTexture = 0;
}
