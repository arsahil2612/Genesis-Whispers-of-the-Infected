#ifndef ASSET_LOADER_H
#define ASSET_LOADER_H

#include <string>

// ============================================================================
// Level 1 Background Remapping and Asset Management
// ============================================================================

// Maps gameplay slice index (0-9) to the correct background asset index.
// Restores proper GDD narrative flow across the 10 level slices:
// 0: Spawn Area / Destroyed Home, 1: House Interior Ruins, 2: Village Street,
// 3: Village Square, 4: Abandoned Market, 5: Raider Camp, 6: Abandoned Church,
// 7: Quarantine Zone, 8: Broken Bridge, 9: Boss Arena / Exit Gate.
int GetLevel1BackgroundFileIndex(int sliceIndex, bool bossDefeated);

// Loads (or returns cached) texture handle for a Level 1 background slice.
unsigned int LoadLevel1BackgroundTexture(int sliceIndex, bool bossDefeated);

// Loads (or returns cached) texture handle for a Level 2 background slice.
unsigned int LoadLevel2BackgroundTexture(int sliceIndex);

// Clears static background texture caches
void ClearBackgroundCache();

// Helper for resolving asset paths across build directories
std::string GetAssetPath(const std::string &relativePath);

#endif // ASSET_LOADER_H
