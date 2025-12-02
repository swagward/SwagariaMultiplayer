#pragma once

class Camera
{
public:
    //TODO: dynamically update screenWidth/Height if the window size changes.
    Camera(int screenWidth, int screenHeight);

    void update();
    void setTarget(float targetWorldX, float targetWorldY);
    void setScreenOffsetTarget(float targetScreenX, float targetScreenY);
    void handleZoom(int wheelDelta);

    int getX() const { return static_cast<int>(x); }
    int getY() const { return static_cast<int>(y); }
    float getPreciseX() const { return x; }
    float getPreciseY() const { return y; }

    float getZoom() const { return zoom; }
    float getTargetX() const { return targetX; }
    float getTargetY() const { return targetY; }

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