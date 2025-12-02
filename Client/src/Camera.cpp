#include "../include/Camera.h"
#include <algorithm>

Camera::Camera(const int screenWidth, const int screenHeight)
    : screenW(screenWidth), screenH(screenHeight) { }

void Camera::update()
{
    //smoothly move camera x/y to the target x/y
    x += (targetX - x) * lerpFactor;
    y += (targetY - y) * lerpFactor;
}

void Camera::setTarget(const float targetWorldX, const float targetWorldY)
{
    targetX = (screenW / 2.0f) - (targetWorldX * zoom);
    targetY = (screenH / 2.0f) - (targetWorldY * zoom);
}

void Camera::setScreenOffsetTarget(const float targetScreenX, const float targetScreenY)
{
    targetX = targetScreenX;
    targetY = targetScreenY;
}

void Camera::handleZoom(const int wheelDelta)
{
    float newZoom = zoom;
    if (wheelDelta > 0) //zoom in
        newZoom += zoomStep;
    else if (wheelDelta < 0) //zoom out
        newZoom -= zoomStep;

    zoom = std::clamp(newZoom, minZoom, maxZoom);
}