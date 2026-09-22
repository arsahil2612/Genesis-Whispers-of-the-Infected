#include "Button.h"
#include "../iGraphics.h"
#include <windows.h>
#include <GL/gl.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// Constructors & Initialization
// ============================================================================
Button::Button()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_width(200.0f)
    , m_height(50.0f)
    , m_textureID(0)
    , m_isHovered(false)
    , m_isPressed(false)
    , m_currentScale(1.0f)
    , m_targetScale(1.0f)
    , m_alpha(1.0f)
    , m_onClick(nullptr)
{
}

Button::Button(float x, float y, float width, float height, unsigned int textureID, std::function<void()> onClick)
    : m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_textureID(textureID)
    , m_isHovered(false)
    , m_isPressed(false)
    , m_currentScale(1.0f)
    , m_targetScale(1.0f)
    , m_alpha(1.0f)
    , m_onClick(onClick)
{
}

void Button::Initialize(float x, float y, float width, float height, unsigned int textureID, std::function<void()> onClick) {
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    m_textureID = textureID;
    m_onClick = onClick;
    m_isHovered = false;
    m_isPressed = false;
    m_currentScale = 1.0f;
    m_targetScale = 1.0f;
    m_alpha = 1.0f;
}

void Button::SetPosition(float x, float y) {
    m_x = x;
    m_y = y;
}

void Button::SetSize(float width, float height) {
    m_width = width;
    m_height = height;
}

void Button::SetTexture(unsigned int textureID) {
    m_textureID = textureID;
}

void Button::SetOnClick(std::function<void()> onClick) {
    m_onClick = onClick;
}

// ============================================================================
// Update Frame Logic & Animation Easing
// ============================================================================
void Button::Update(float dt, int mouseX, int mouseY, bool isMouseDown) {
    // 1. Mouse Bounding Box Collision Check
    bool mouseInside = (mouseX >= m_x && mouseX <= m_x + m_width &&
                        mouseY >= m_y && mouseY <= m_y + m_height);

    m_isHovered = mouseInside;

    // 2. Interaction State & Target Scale Determination
    if (m_isHovered) {
        if (isMouseDown) {
            m_isPressed = true;
            m_targetScale = 0.95f; // Scale down to 95% on click
        } else {
            if (m_isPressed) {
                // Click released while over button -> Execute callback action!
                m_isPressed = false;
                if (m_onClick) {
                    m_onClick();
                }
            }
            m_targetScale = 1.10f; // Scale up to 110% on hover
        }
    } else {
        m_isPressed = false;
        m_targetScale = 1.00f;     // Return to normal 100% scale when idle
    }

    // 3. Smooth Easing Animation (Lerp interpolation)
    float speed = 14.0f;
    m_currentScale += (m_targetScale - m_currentScale) * speed * dt;
}

// ============================================================================
// OpenGL Rendering Pass with Scale Centering, Cyan Glow, and Brightness Boost
// ============================================================================
void Button::Draw() const {
    if (m_alpha <= 0.001f || m_textureID == 0) return;

    // Calculate scaled dimensions centered around the base position
    float scaledW = m_width * m_currentScale;
    float scaledH = m_height * m_currentScale;
    float drawX = m_x - (scaledW - m_width) * 0.5f;
    float drawY = m_y - (scaledH - m_height) * 0.5f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Hover Effect: Draw high-tech cyan glow outline behind button
    if (m_isHovered) {
        float glowPad = 8.0f * (m_currentScale - 0.95f);
        float glowX1 = drawX - glowPad;
        float glowY1 = drawY - glowPad;
        float glowX2 = drawX + scaledW + glowPad;
        float glowY2 = drawY + scaledH + glowPad;

        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        // Cyan soft aura quad
        glColor4f(0.0f, 0.85f, 1.0f, 0.35f * m_alpha);
        glVertex2f(glowX1, glowY1);
        glVertex2f(glowX2, glowY1);
        glVertex2f(glowX2, glowY2);
        glVertex2f(glowX1, glowY2);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }

    // 2. Main Button Image Rendering
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Apply brightness boost when hovered
    if (m_isHovered) {
        glColor4f(1.15f, 1.15f, 1.15f, m_alpha);
    } else {
        glColor4f(1.0f, 1.0f, 1.0f, m_alpha);
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.001f, 0.999f); glVertex2f(drawX, drawY);
    glTexCoord2f(0.999f, 0.999f); glVertex2f(drawX + scaledW, drawY);
    glTexCoord2f(0.999f, 0.001f); glVertex2f(drawX + scaledW, drawY + scaledH);
    glTexCoord2f(0.001f, 0.001f); glVertex2f(drawX, drawY + scaledH);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}
