package com.swagaria.game;

import com.swagaria.data.terrain.TerrainConfig;

public class Physics
{
    public static void stepPlayer(Player p, World world, float dt)
    {
        float x = p.getX();
        float y = p.getY();
        float vx;
        float vy = p.getVy();
        boolean onGround = p.isOnGround();

        //horizontal movement
        if (p.isMovingLeft()) vx = -p.getMoveSpeed();
        else if (p.isMovingRight()) vx = p.getMoveSpeed();
        else vx = 0f;

        //platform boost leniency
        if (p.isJumpPressed() && vy <= 0f)
        {
            int centerX = (int) Math.floor(x + Player.WIDTH / 2f);
            int footY = (int) Math.floor(y + Player.HEIGHT - 0.2f);
            int midY = (int) Math.floor(y + Player.HEIGHT / 2f);

            //check if player and platform are in bounds of each other
            if (centerX >= 0 && centerX < TerrainConfig.WORLD_WIDTH) {
                if (footY >= 0 && footY < TerrainConfig.WORLD_HEIGHT && world.isPlatformTile(centerX, footY) ||
                        midY >= 0 && midY < TerrainConfig.WORLD_HEIGHT && world.isPlatformTile(centerX, midY))
                    vy = -p.getJumpSpeed() * 0.6f;
            }
        }

        //jump
        if (p.isJumpPressed() && onGround)
        {
            vy = -Math.abs(p.getJumpSpeed());
            onGround = false;
        }

        //gravity
        vy += Math.abs(p.getGravity()) * dt;
        if (vy > Math.abs(p.getMaxFallSpeed())) vy = Math.abs(p.getMaxFallSpeed());

        //horizontal collision
        float newX = x + vx * dt;

        //check left and right bounds
        if (newX < 0)
        {
            newX = 0;
            vx = 0;
        }
        else if (newX + Player.WIDTH > TerrainConfig.WORLD_WIDTH)
        {
            newX = TerrainConfig.WORLD_WIDTH - Player.WIDTH;
            vx = 0;
        }

        float checkBottom = y + 0.1f;
        float checkTop = y + Player.HEIGHT - 0.1f;
        int bottomTile = (int) Math.floor(checkBottom);
        int topTile = (int) Math.floor(checkTop);

        if (vx > 0f)
        {
            int rightTile = (int) Math.floor(newX + Player.WIDTH);
            for (int ty = bottomTile; ty <= topTile; ty++)
            {
                if (ty >= 0 && ty < TerrainConfig.WORLD_HEIGHT && world.isSolidTile(rightTile, ty))
                {
                    newX = rightTile - Player.WIDTH;
                    break;
                }
            }
        }
        else if (vx < 0f)
        {
            int leftTile = (int) Math.floor(newX);
            for (int ty = bottomTile; ty <= topTile; ty++)
            {
                if (ty >= 0 && ty < TerrainConfig.WORLD_HEIGHT && world.isSolidTile(leftTile, ty))
                {
                    newX = leftTile + 1f;
                    break;
                }
            }
        }
        x = newX;

        //vertical collision
        float newY = y + vy * dt;

        //check top and bottom bounds
        if (newY < 0)
        {
            newY = 0;
            vy = 0;
        }
        else if (newY + Player.HEIGHT > TerrainConfig.WORLD_HEIGHT)
        {
            newY = TerrainConfig.WORLD_HEIGHT - Player.HEIGHT;
            vy = 0;
            onGround = true;
        }

        int minTx = (int) Math.floor(x + 0.05f);
        int maxTx = (int) Math.floor(x + Player.WIDTH - 0.05f);

        onGround = false;

        if (vy >= 0f) //falling or stationary
        {
            int nextFootY = (int) Math.floor(newY + Player.HEIGHT);
            int prevFootY = (int) Math.floor(y + Player.HEIGHT);

            for (int tx = minTx; tx <= maxTx; tx++)
            {
                if (tx < 0 || tx >= TerrainConfig.WORLD_WIDTH || nextFootY < 0 || nextFootY >= TerrainConfig.WORLD_HEIGHT)
                    continue;

                boolean isSolid = world.isSolidTile(tx, nextFootY);
                boolean isPlatform = world.isPlatformTile(tx, nextFootY);

                if (isSolid || isPlatform)
                {
                    boolean ignorePlatform = isPlatform && (p.isDownPressed() || (prevFootY > nextFootY));
                    if (isSolid || (isPlatform && !ignorePlatform && prevFootY <= nextFootY))
                    {
                        newY = nextFootY - Player.HEIGHT;
                        vy = 0f;
                        onGround = true;
                        break;
                    }
                }
            }
        }
        else if (vy < 0f) //jumping up
        {
            int headY = (int) Math.floor(newY);
            for (int tx = minTx; tx <= maxTx; tx++)
            {
                if (tx < 0 || tx >= TerrainConfig.WORLD_WIDTH || headY < 0 || headY >= TerrainConfig.WORLD_HEIGHT)
                    continue;

                if (world.isSolidTile(tx, headY))
                {
                    newY = headY + 1f;
                    vy = 0f;
                    break;
                }
            }
        }

        //set results
        p.setPosition(x, newY);
        p.setVelocity(vx, vy);
        p.setOnGround(onGround);
    }
}
