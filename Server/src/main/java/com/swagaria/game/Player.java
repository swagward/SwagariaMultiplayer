package com.swagaria.game;

public class Player {
    private final int id;

    // position in tiles (bottom-left corner)
    private float x;
    private float y;

    // previous position for hasMoved()
    private float lastX;
    private float lastY;

    // velocity in tiles/sec
    private float vx = 0f;
    private float vy = 0f;

    // input flags
    private boolean up = false, left = false, right = false;

    // physics constants
    private static final float MOVE_SPEED = 6.0f;    // tiles/sec
    private static final float JUMP_SPEED = 12.0f;   // tiles/sec
    private static final float GRAVITY = 40.0f;     // tiles/sec^2 (down)
    private static final float MAX_FALL_SPEED = 50.0f;

    // player size (tiles)
    public static final float WIDTH = 1.0f;
    public static final float HEIGHT = 2.0f;

    // grounded state
    private boolean onGround = false;

    public Player(int id, float spawnX, float spawnY) {
        this.id = id;
        this.x = spawnX;
        this.y = spawnY;
        this.lastX = spawnX;
        this.lastY = spawnY;
    }

    public int getId() { return id; }

    public float getX() { return x; }
    public float getY() { return y; }

    public void setPosition(float nx, float ny) { this.x = nx; this.y = ny; }
    public void setVelocity(float nvx, float nvy) { this.vx = nvx; this.vy = nvy; }
    public float getVx() { return vx; }
    public float getVy() { return vy; }

    public boolean isOnGround() { return onGround; }
    public void setOnGround(boolean g) { this.onGround = g; }

    public boolean isMovingLeft() { return left; }
    public boolean isMovingRight() { return right; }
    public boolean isJumpPressed() { return up; }

    public float getMoveSpeed() { return MOVE_SPEED; }
    public float getJumpSpeed() { return JUMP_SPEED; }
    public float getGravity() { return GRAVITY; }
    public float getMaxFallSpeed() { return MAX_FALL_SPEED; }

    // input handling
    public void setInput(String action, boolean pressed) {
        switch (action) {
            case "UP_DOWN" -> up = pressed;
            case "UP_UP" -> up = false;
            case "LEFT_DOWN" -> left = pressed;
            case "LEFT_UP" -> left = false;
            case "RIGHT_DOWN" -> right = pressed;
            case "RIGHT_UP" -> right = false;
        }
    }

    // network helpers
    public boolean hasMoved() {
        return Math.abs(x - lastX) > 0.001f || Math.abs(y - lastY) > 0.001f;
    }

    public void syncPosition() {
        lastX = x;
        lastY = y;
    }
}
