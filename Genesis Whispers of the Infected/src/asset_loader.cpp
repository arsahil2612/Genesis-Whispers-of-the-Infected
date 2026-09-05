#define _CRT_SECURE_NO_WARNINGS
#include "asset_loader.h"
#include "igraphics_declarations.h"
#include <cstdio>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

std::string GetAssetPath(const std::string &relativePath) {
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    std::string path1 = relativePath;
    FILE* f = fopen(path1.c_str(), "rb");
    if (f) {
        fclose(f);
        return path1;
    }
    std::string path2 = "../" + relativePath;
    f = fopen(path2.c_str(), "rb");
    if (f) {
        fclose(f);
        return path2;
    }
    return relativePath;
}

void PlayAudioFile(const std::string &primaryRelativePath, const std::string &fallbackRelativePath) {
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

    // Allocate round-robin MCI channels so multiple sounds can play simultaneously without interrupting each other
    static int channelIndex = 0;
    int ch = channelIndex;
    channelIndex = (channelIndex + 1) % 16;

    char alias[32];
    sprintf_s(alias, sizeof(alias), "snd_ch_%d", ch);

    char cmdClose[64];
    sprintf_s(cmdClose, sizeof(cmdClose), "close %s", alias);
    mciSendStringA(cmdClose, NULL, 0, NULL);

    char cmdOpen[MAX_PATH + 128];
    sprintf_s(cmdOpen, sizeof(cmdOpen), "open \"%s\" type mpegvideo alias %s", fullPath, alias);
    MCIERROR err = mciSendStringA(cmdOpen, NULL, 0, NULL);
    if (err == 0) {
        char cmdPlay[64];
        sprintf_s(cmdPlay, sizeof(cmdPlay), "play %s from 0", alias);
        mciSendStringA(cmdPlay, NULL, 0, NULL);
    } else {
        PlaySoundA(fullPath, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
}

// Static caching tables for background textures
static unsigned int g_level1BgTextures[10] = { 0 };
static int g_level1BgSourceIndex[10] = { 0 };
static unsigned int g_level2BgTextures[10] = { 0 };

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

void ClearBackgroundCache() {
    for (int i = 0; i < 10; ++i) {
        g_level1BgTextures[i] = 0;
        g_level1BgSourceIndex[i] = 0;
        g_level2BgTextures[i] = 0;
    }
}
