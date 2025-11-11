#pragma once
#include <algorithm>

class Camera {
public:
    float x = 0.0f;
    float y = 0.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    int screenWidth;
    int screenHeight;
    float zoom = 1.0f;           // 1.0 = default zoom, <1.0 = zoom out, >1.0 = zoom in
    float followSpeed = 0.1f;    // 0.1 = smooth follow, 1.0 = instant snap

    Camera(const int screenWidth, const int screenHeight)
        : screenWidth(screenWidth), screenHeight(screenHeight) {}

    // set where the camera should move towards (instead of instant snap)
    void setTarget(const float tx, const float ty) {
        targetX = tx - (screenWidth / (2.0f * zoom));
        targetY = ty - (screenHeight / (2.0f * zoom));
    }

    // call this each frame to move camera smoothly toward target
    void update() {
        x += (targetX - x) * followSpeed;
        y += (targetY - y) * followSpeed;
    }

    // handle zoom input
    void adjustZoom(float delta) {
        zoom = std::clamp(zoom + delta, -1.0f, 3.0f); // allow 0.5x–3x zoom
    }

    // world → screen transform with zoom
    [[nodiscard]] int worldToScreenX(const float worldX) const {
        return static_cast<int>((worldX - x) * zoom);
    }

    [[nodiscard]] int worldToScreenY(const float worldY) const {
        return static_cast<int>((worldY - y) * zoom);
    }

    // screen → world (for mouse interactions)
    [[nodiscard]] float screenToWorldX(const int screenX) const {
        return screenX / zoom + x;
    }

    [[nodiscard]] float screenToWorldY(const int screenY) const {
        return screenY / zoom + y;
    }
};
