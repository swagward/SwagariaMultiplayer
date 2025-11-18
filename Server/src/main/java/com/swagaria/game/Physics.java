package com.swagaria.game;

public class Physics
{

    public static void stepPlayer(Player p, World world, float dt)
    {
        float x = p.getX();
        float y = p.getY();
        float vx = 0f;
        float vy = p.getVy();
        boolean onGround = p.isOnGround();

        //A & D input
        if (p.isMovingLeft())  vx = -p.getMoveSpeed();
        else if (p.isMovingRight()) vx = p.getMoveSpeed();
        else vx = 0f;

        //W Input
        if (p.isJumpPressed() && onGround)
        {
            vy = -Math.abs(p.getJumpSpeed());
            onGround = false;
        }

        //apply gravity to player's Y coord
        vy += Math.abs(p.getGravity()) * dt;
        if (vy > Math.abs(p.getMaxFallSpeed())) vy = Math.abs(p.getMaxFallSpeed());

        //horizontal collision
        float newX = x + vx * dt;

        float checkBottom = y + 0.05f;
        float checkTop    = y + Player.HEIGHT - 0.05f;
        int bottomTile = (int)Math.floor(checkBottom);
        int topTile    = (int)Math.floor(checkTop);

        if (vx > 0f)
        {
            int rightTile = (int)Math.floor(newX + Player.WIDTH);
            for (int ty = bottomTile; ty <= topTile; ty++)
            {
                if (world.isSolidTile(rightTile, ty))
                {
                    newX = rightTile - Player.WIDTH;
                    break;
                }
            }
        }
        else if (vx < 0f)
        {
            int leftTile = (int)Math.floor(newX);
            for (int ty = bottomTile; ty <= topTile; ty++)
            {
                if (world.isSolidTile(leftTile, ty))
                {
                    newX = leftTile + 1f;
                    break;
                }
            }
        }

        //apply x movement regardless
        x = newX;

        //vertical collision
        float newY = y + vy * dt;
        int minTx = (int)Math.floor(x);
        int maxTx = (int)Math.floor(x + Player.WIDTH - 1e-6f);

        onGround = false; //reset per frame before checking
        if (vy > 0f) //falling
        {
            int bottomCheck = (int)Math.floor(newY + Player.HEIGHT);
            for (int tx = minTx; tx <= maxTx; tx++)
            {
                if (world.isSolidTile(tx, bottomCheck))
                {
                    newY = bottomCheck - Player.HEIGHT;
                    vy = 0f;
                    onGround = true;
                    break;
                }
            }
        } else if (vy < 0f) //jumping
        {
            int topCheck = (int)Math.floor(newY);
            for (int tx = minTx; tx <= maxTx; tx++)
            {
                if (world.isSolidTile(tx, topCheck))
                {
                    newY = topCheck + 1f;
                    vy = 0f;
                    break;
                }
            }
        }

        //apply finals
        p.setPosition(x, newY);
        p.setVelocity(vx, vy);
        p.setOnGround(onGround);
    }
}
