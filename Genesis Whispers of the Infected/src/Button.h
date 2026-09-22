#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include <functional>

// ============================================================================
// Interactive Reusable UI Button Class
// ============================================================================
class Button {
private:
    float m_x;               // Bottom-left base position X
    float m_y;               // Bottom-left base position Y
    float m_width;           // Base width
    float m_height;          // Base height
    unsigned int m_textureID;// Loaded OpenGL texture handle
    
    bool m_isHovered;        // True when mouse cursor is over button area
    bool m_isPressed;        // True when mouse left button is held down over button
    
    float m_currentScale;    // Smoothly animated scale multiplier (1.0 = normal, 1.1 = hover, 0.95 = click)
    float m_targetScale;     // Desired target scale
    float m_alpha;           // Opacity multiplier (0.0 to 1.0) for fade transitions

    std::function<void()> m_onClick; // Callback executed on click release

public:
    Button();
    Button(float x, float y, float width, float height, unsigned int textureID, std::function<void()> onClick = nullptr);

    void Initialize(float x, float y, float width, float height, unsigned int textureID, std::function<void()> onClick = nullptr);
    
    void SetPosition(float x, float y);
    void SetSize(float width, float height);
    void SetTexture(unsigned int textureID);
    void SetOnClick(std::function<void()> onClick);
    void SetAlpha(float alpha) { m_alpha = alpha; }

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    unsigned int GetTexture() const { return m_textureID; }
    bool IsHovered() const { return m_isHovered; }
    float GetScale() const { return m_currentScale; }

    // Frame update & mouse interaction
    void Update(float dt, int mouseX, int mouseY, bool isMouseDown);

    // OpenGL rendering pass
    void Draw() const;
};

#endif // BUTTON_H
