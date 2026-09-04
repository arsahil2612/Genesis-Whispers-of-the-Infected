#ifndef MAP_H
#define MAP_H

#include <vector>
// ============================================================================
// Platform Bounding Box Struct
// ============================================================================
struct Platform {
    double x, y, width, height;
    Platform() : x(0), y(0), width(0), height(0) {}
    Platform(double _x, double _y, double _w, double _h) : x(_x), y(_y), width(_w), height(_h) {}
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
    int currentLevelNumber;

    // Parallax Factor Constants
    double parallaxFarFactor;      // 0.25 (Sky, distant horizon)
    double parallaxMidFactor;      // 0.45 (Midground trees & scenery)
    double parallaxGameplayFactor; // 1.00 (Ground, platforms, props, characters)
    double parallaxFgFactor;       // 1.15 (Foreground atmospheric rain & leaves)

public:
    Map();
    void LoadLevel(int levelNumber);

    // Multi-Layer Parallax Scrolling Render Pipeline
    void RenderFarBackground(double cameraX, bool bossDefeated = false);
    void RenderMidground(double cameraX, bool bossDefeated = false);
    void RenderGroundSurface(double cameraX);
    void RenderForeground(double cameraX, double animTime = 0.0);

    // Legacy / Convenience background renderers
    void RenderBackground(double cameraX, bool bossDefeated = false);
    void RenderWater(double cameraX, double cameraY);
    void RenderBridgeAndEnvironmentSprites(double cameraX, double cameraY);
    void ApplyCameraTracking(double playerX, double playerY, int screenWidth, int screenHeight);
    
    // Inline Getters & Setters
    const std::vector<Platform>& GetPlatforms() const { return platforms; }
    double GetCameraX() const { return cameraX; }
    double GetCameraY() const { return cameraY; }
    void SetCameraX(double cx) { cameraX = cx; }
    int GetLevelWidth() const { return levelWidth; }
    int GetCurrentLevelNumber() const { return currentLevelNumber; }

    // Parallax Factor Getters
    double GetParallaxFarFactor() const { return parallaxFarFactor; }
    double GetParallaxMidFactor() const { return parallaxMidFactor; }
    double GetParallaxGameplayFactor() const { return parallaxGameplayFactor; }
    double GetParallaxFgFactor() const { return parallaxFgFactor; }
};

#endif // MAP_H

