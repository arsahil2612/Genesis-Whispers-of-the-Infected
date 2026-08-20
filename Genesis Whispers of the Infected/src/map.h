#ifndef MAP_H
#define MAP_H

#include <vector>

// ============================================================================
// Platform Bounding Box Struct
// ============================================================================
struct Platform {
    double x, y, width, height;
};

// ============================================================================
// Map & Level System Class
// ============================================================================
class Map {
private:
    std::vector<Platform> platforms;
    double cameraX;
    double cameraY;
    int levelWidth;
    int levelHeight;

public:
    Map();
    void LoadLevel(int levelNumber);
    void RenderBackground(double cameraX, bool bossDefeated = false);
    void ApplyCameraTracking(double playerX, double playerY, int screenWidth, int screenHeight);
    
    // Inline Getters
    const std::vector<Platform>& GetPlatforms() const { return platforms; }
    double GetCameraX() const { return cameraX; }
    double GetCameraY() const { return cameraY; }
    int GetLevelWidth() const { return levelWidth; }
};

#endif // MAP_H

