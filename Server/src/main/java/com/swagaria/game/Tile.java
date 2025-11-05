package com.swagaria.game;

public class Tile {
    public int type; // 0 = air, 1 = dirt, 2 = stone, etc.

    public Tile(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }
}
