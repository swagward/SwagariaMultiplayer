package com.swagaria.game;

public class Player
{
    private final int id;
    private float x;
    private float y;
    private float lastX;
    private float lastY;
    private float vx = 0f;
    private float vy = 0f;
    private boolean up = false, left = false, right = false;

    private static final float MOVE_SPEED = 6.0f;      //tiles/sec
    private static final float JUMP_SPEED = 12.0f;     //tiles/sec
    private static final float GRAVITY = 40.0f;        //tiles/sec^2 (down)
    private static final float MAX_FALL_SPEED = 50.0f; //terminal velocity
    private boolean onGround = false;

    //player size (in tiles)
    public static final float WIDTH = 1.0f;
    public static final float HEIGHT = 2.0f;

    //player reach (for tile modification)
    public static final float MAX_REACH_DISTANCE = 8000.0f;
    public static final float MAX_REACH_DISTANCE_SQ = MAX_REACH_DISTANCE * MAX_REACH_DISTANCE;

    public Player(int id, float spawnX, float spawnY)
    {
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

    //input handling
    public void setInput(String action, boolean pressed)
    {
        switch (action)
        {
            case "UP_DOWN" -> up = pressed;
            case "UP_UP" -> up = false;
            case "LEFT_DOWN" -> left = pressed;
            case "LEFT_UP" -> left = false;
            case "RIGHT_DOWN" -> right = pressed;
            case "RIGHT_UP" -> right = false;
        }
    }

    public boolean hasMoved()
    {
        return Math.abs(x - lastX) > 0.001f || Math.abs(y - lastY) > 0.001f;
    }

    public void syncPosition()
    {
        lastX = x;
        lastY = y;
    }
}
