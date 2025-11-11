package com.swagaria.game;

public class Physics {

    /**
     * Step the player physics by dt seconds.
     * Handles horizontal movement, vertical movement, gravity, and jump.
     * Does NOT handle collision yet.
     */
    public static void stepPlayer(Player p, float dt) {
        float x = p.getX();
        float y = p.getY();
        float vx = 0f;
        float vy = p.getVy();
        boolean onGround = p.isOnGround();

        // --- horizontal input ---
        if (p.isMovingLeft())  vx -= p.getMoveSpeed();
        if (p.isMovingRight()) vx += p.getMoveSpeed();

        // --- jump ---
        if (p.isJumpPressed() && onGround) {
            vy = p.getJumpSpeed(); // positive = up
            onGround = false;
        }

        // --- gravity ---
        vy -= p.getGravity() * dt; // gravity pulls down
        if (vy < -p.getMaxFallSpeed()) vy = -p.getMaxFallSpeed();

        // --- update positions ---
        x += vx * dt;
        y += vy * dt;

        // --- simple ground detection for testing ---
        // if y < 0, snap to ground
        if (y < 0) {
            y = 0;
            vy = 0;
            onGround = true;
        }

        // --- apply updated values ---
        p.setPosition(x, y);
        p.setVelocity(vx, vy);
        p.setOnGround(onGround);
    }
}
