package com.swagaria.game;

import com.swagaria.util.OpenSimplexNoise;

//hello this is a comment
//terrain chunk containing tiles
public class Chunk {
    public static final int SIZE = 16;

    private final int chunkX;
    private final int chunkY;
    private final Tile[][] tiles = new Tile[SIZE][SIZE];

    public Chunk(int chunkX, int chunkY, long worldSeed) {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        generate(worldSeed);
    }

    //generate procedural terrain using noise
    private void generate(long worldSeed) {
        OpenSimplexNoise heightNoise = new OpenSimplexNoise(worldSeed);
        OpenSimplexNoise caveNoise = new OpenSimplexNoise(worldSeed + 1337);

        final int worldHeight = SIZE * 16; //vertical scale
        final double heightFrequency = 0.03;
        final double caveFrequency = 0.08;

        for (int x = 0; x < SIZE; x++) {
            int worldX = chunkX * SIZE + x;

            //get the surface Y value
            double heightValue = heightNoise.eval(worldX * heightFrequency, 0.0);
            heightValue = (heightValue + 1.0) * 0.5;
            int surfaceY = (int) (heightValue * (worldHeight * 0.5));

            for (int y = 0; y < SIZE; y++) {
                int worldY = chunkY * SIZE + y;

                //flip the Y so top = bottom and vice versa (stupid java 2D origin
                int arrayY = SIZE - 1 - y;

                //generate cave pockets
                double caveVal = caveNoise.eval(worldX * caveFrequency, worldY * caveFrequency);
                boolean isCave = caveVal > 0.4;

                int type;
                if (worldY > surfaceY) {
                    type = 0; //air
                } else if (isCave) {
                    type = 0; //cave hole
                } else if (worldY == surfaceY) {
                    type = 1; //grass
                } else if (worldY > surfaceY - 3) {
                    type = 2; //dirt
                } else {
                    type = 3; //stone
                }

                tiles[arrayY][x] = new Tile(type);
            }
        }
    }

    public int getChunkX() { return chunkX; }
    public int getChunkY() { return chunkY; }

    public Tile getTile(int x, int y) {
        if (x < 0 || x >= SIZE || y < 0 || y >= SIZE)
            return null;
        return tiles[y][x];
    }

    public String serialize() {
        StringBuilder sb = new StringBuilder();
        sb.append("CHUNK_DATA,").append(chunkX).append(",").append(chunkY);
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                sb.append(",").append(tiles[y][x].type);
            }
        }
        return sb.toString();
    }
}