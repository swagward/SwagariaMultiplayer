#pragma once

class Camera
{
public:
    //TODO: Allow the window size to be changed, in game camera should resize so that nothing visually changes, with tile scaling.
    Camera(int screenWidth, int screenHeight);

    void update();
    void setTarget(float targetWorldX, float targetWorldY);
    void setScreenOffsetTarget(float targetScreenX, float targetScreenY);
    void handleZoom(int wheelDelta);

    [[nodiscard]] int getX() const { return static_cast<int>(x); }
    [[nodiscard]] int getY() const { return static_cast<int>(y); }
    [[nodiscard]] float getPreciseX() const { return x; }
    [[nodiscard]] float getPreciseY() const { return y; }

    [[nodiscard]] float getZoom() const { return zoom; }
    [[nodiscard]] float getTargetX() const { return targetX; }
    [[nodiscard]] float getTargetY() const { return targetY; }

private:
    float x = 0.0f;
    float y = 0.0f;

    float targetX = 0.0f;
    float targetY = 0.0f;

    float zoom = 1.0f; //1.0 is default, 0.5 is zoomed out, 3.0 is zoomed in
    const float lerpFactor = 0.03f; //0.1 is 10% movement per frame
    const float minZoom = 0.5f;
    const float maxZoom = 3.0f;
    const float zoomStep = 0.2f;

    int screenW;
    int screenH;
};