package com.swagaria.game;

public class World {
    private final int width;
    private final int height;
    private final int[][] tiles;

    public static final int TILE_AIR = 0;
    public static final int TILE_DIRT = 1;

    public World(int width, int height) {
        this.width = width;
        this.height = height;
        this.tiles = new int[width][height];
        generateFlatWorld();
    }

    private void generateFlatWorld() {
        int groundHeight = height / 2;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                tiles[x][y] = (y >= groundHeight) ? TILE_DIRT : TILE_AIR;
            }
        }
        System.out.println("[World] Generated flat world " + width + "x" + height);
    }

    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public int getTile(int x, int y) { return tiles[x][y]; }

    public String serialize() {
        StringBuilder sb = new StringBuilder();
        sb.append("WORLD_DATA,").append(width).append(",").append(height).append(",");
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                sb.append(tiles[x][y]);
                if (x != width - 1 || y != height - 1) sb.append(",");
            }
        }
        return sb.toString();
    }
}
