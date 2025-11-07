#pragma once

class Camera {
public:
    float x = 0.0f;
    float y = 0.0f;
    int screenWidth;
    int screenHeight;

    Camera(const int screenWidth, const int screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight) {}

    // Centers the camera on a target position (like the player)
    void centerOn(const float targetX, const float targetY) {
        x = targetX - screenWidth / 2.0f;
        y = targetY - screenHeight / 2.0f;
    }

    // Converts world coordinates → screen coordinates
    [[nodiscard]] int worldToScreenX(const float worldX) const { return static_cast<int>(worldX - x); }
    [[nodiscard]] int worldToScreenY(const float worldY) const { return static_cast<int>(worldY - y); }
};