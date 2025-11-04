package com.swagaria.game;

public class Player {
    private final int id;
    private String name;
    private float x, y;
    private float lastX, lastY;
    private float vx = 0, vy = 0;
    private final float moveSpeed = 100.0f;     // horizontal speed (pixels/sec)
    private final float jumpVelocity = -250.0f; // upward impulse (pixels/sec)
    private final float gravity = 500.0f;       // gravity strength
    private final float floorY = 300.0f;        // floor height

    private boolean up = false, down = false, left = false, right = false;
    private boolean onGround = true;

    public Player(int id, float x, float y) {
        this.id = id;
        this.name = "Player" + this.id;
        this.x = x;
        this.y = y;
        this.lastX = x;
        this.lastY = y;
    }

    public int getId() { return id; }
    public float getX() { return x; }
    public float getY() { return y; }
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public void setInput(String action, boolean pressed) {
        switch (action) {
            case "UP_DOWN"    -> {
                up = pressed;
                // Jump only when grounded
                if (pressed && onGround) {
                    vy = jumpVelocity;
                    onGround = false;
                }
            }
            case "UP_UP"      -> up = false;
            case "DOWN_DOWN"  -> down = pressed;
            case "DOWN_UP"    -> down = false;
            case "LEFT_DOWN"  -> left = pressed;
            case "LEFT_UP"    -> left = false;
            case "RIGHT_DOWN" -> right = pressed;
            case "RIGHT_UP"   -> right = false;
        }
    }

    public void update(float deltaTime) {
        // Horizontal velocity
        vx = 0;
        if (left)  vx -= moveSpeed;
        if (right) vx += moveSpeed;

        // Apply gravity
        vy += gravity * deltaTime;

        // Apply velocity
        x += vx * deltaTime;
        y += vy * deltaTime;

        // Simple floor collision
        if (y > floorY) {
            y = floorY;
            vy = 0;
            onGround = true;
        }
    }

    public boolean hasMoved() {
        return Math.abs(x - lastX) > 0.01f || Math.abs(y - lastY) > 0.01f;
    }

    public void syncPosition() {
        lastX = x;
        lastY = y;
    }
}
