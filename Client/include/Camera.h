#pragma once

class Camera {
public:
    float x = 0.0f;
    float y = 0.0f;
    int screenWidth;
    int screenHeight;

    Camera(int screenWidth, int screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight) {}

    // Centers the camera on a target position (like the player)
    void centerOn(float targetX, float targetY) {
        x = targetX - screenWidth / 2.0f;
        y = targetY - screenHeight / 2.0f;
    }

    // Converts world coordinates → screen coordinates
    int worldToScreenX(float worldX) const { return static_cast<int>(worldX - x); }
    int worldToScreenY(float worldY) const { return static_cast<int>(worldY - y); }
};
