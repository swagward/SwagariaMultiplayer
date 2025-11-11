package com.swagaria.data;

import com.swagaria.util.OpenSimplexNoise;

public class TerrainConfig {
    public static final int CHUNK_SIZE = 16;
    public static final int WORLD_CHUNKS_X = 16;
    public static final int WORLD_CHUNKS_Y = 16;
    public static final int WORLD_WIDTH = CHUNK_SIZE * WORLD_CHUNKS_X;
    public static final int WORLD_HEIGHT = CHUNK_SIZE * WORLD_CHUNKS_Y;

    public static final double NOISE_FREQ = 0.02;
    public static final double NOISE_AMP = 10.0;
    public static final double BASE_HEIGHT = 40.0;

    public static final long SEED = System.currentTimeMillis();
    public static final OpenSimplexNoise NOISE = new OpenSimplexNoise(SEED);
}
