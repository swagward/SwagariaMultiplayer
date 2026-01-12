package com.swagaria.data.terrain;

import com.swagaria.util.OpenSimplexNoise;

public class TerrainConfig
{
    public static final int CHUNK_SIZE = 16;
    public static final int WORLD_CHUNKS_X = 64;
    public static final int WORLD_CHUNKS_Y = 64;
    public static final int WORLD_WIDTH = CHUNK_SIZE * WORLD_CHUNKS_X;
    public static final int WORLD_HEIGHT = CHUNK_SIZE * WORLD_CHUNKS_Y;

    public static final double NOISE_FREQ = 0.02;
    public static final double NOISE_AMP = 10.0;
    public static final double BASE_HEIGHT = 40.0;

    public static final double TREE_CHANCE = 0.15;
    public static final double FLORA_CHANCE = 0.4;
    public static final double FLOWER_CHANCE = 0.2;

    public static long SEED = System.currentTimeMillis();
    public static OpenSimplexNoise NOISE = new OpenSimplexNoise(SEED);

    public static void setSeed(long newSeed) {
        SEED = newSeed;
        NOISE = new OpenSimplexNoise(SEED);
        System.out.println("[World] Seed set to: " + SEED);
    }
}