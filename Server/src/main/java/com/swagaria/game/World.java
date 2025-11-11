package com.swagaria.game;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.Tile;
import com.swagaria.data.TileType;

import java.util.ArrayList;
import java.util.List;

public class World {
    private final Chunk[][] chunks;

    public World() {
        chunks = new Chunk[TerrainConfig.WORLD_CHUNKS_X][TerrainConfig.WORLD_CHUNKS_Y];
        generate();
        System.out.println("[World] Generated " + (TerrainConfig.WORLD_CHUNKS_X * TerrainConfig.WORLD_CHUNKS_Y) + " chunks.");
    }

    private void generate() {
        System.out.println("[World] Generating " + TerrainConfig.WORLD_CHUNKS_X + "x" + TerrainConfig.WORLD_CHUNKS_Y + " chunks...");
        for (int cy = 0; cy < TerrainConfig.WORLD_CHUNKS_Y; cy++) {
            for (int cx = 0; cx < TerrainConfig.WORLD_CHUNKS_X; cx++) {
                Chunk chunk = new Chunk(cx, cy); // chunk generates itself in ctor
                chunks[cx][cy] = chunk;
            }
        }
        System.out.println("[World] Generation complete.");
    }

    // return a list for code that expects getAllChunks()
    public List<Chunk> getAllChunks() {
        List<Chunk> list = new ArrayList<>();
        for (int y = 0; y < TerrainConfig.WORLD_CHUNKS_Y; y++) {
            for (int x = 0; x < TerrainConfig.WORLD_CHUNKS_X; x++) {
                list.add(chunks[x][y]);
            }
        }
        return list;
    }

    // convenience: return a chunk by chunk coords (or null)
    public Chunk getChunk(int cx, int cy) {
        if (cx < 0 || cx >= TerrainConfig.WORLD_CHUNKS_X || cy < 0 || cy >= TerrainConfig.WORLD_CHUNKS_Y) return null;
        return chunks[cx][cy];
    }

    public Tile getTileAt(int worldX, int worldY) {
        if (worldX < 0 || worldY < 0 ||
                worldX >= TerrainConfig.WORLD_WIDTH ||
                worldY >= TerrainConfig.WORLD_HEIGHT)
            return null;

        int chunkX = worldX / TerrainConfig.CHUNK_SIZE;
        int chunkY = worldY / TerrainConfig.CHUNK_SIZE;

        Chunk chunk = getChunk(chunkX, chunkY);
        if (chunk == null) return null;

        int localX = worldX % TerrainConfig.CHUNK_SIZE;
        int localY = worldY % TerrainConfig.CHUNK_SIZE;

        return chunk.getTile(localX, localY);
    }

    public boolean isSolidTile(int worldX, int worldY) {
        Tile tile = getTileAt(worldX, worldY);
        if (tile.getType() == TileType.AIR || tile == null) return false;

        TileType type = tile.getType();
        return (type == TileType.STONE || type == TileType.DIRT || type == TileType.GRASS);
    }

    // helper used earlier for spawn: find a spawn near middle
    public int[] findSpawnTile() {
        // middle X coordinate of the world in tile units
        int worldMidX = TerrainConfig.WORLD_WIDTH / 2;

        // approximate surface height by sampling noise at midX
        int surfaceY = getSurfaceHeightAt(worldMidX);

        // spawn 1-2 tiles above surface
        return new int[] { worldMidX, surfaceY + 2 };
    }


    private int getSurfaceHeightAt(int worldX) {
        double noise = TerrainConfig.NOISE.eval(worldX * TerrainConfig.NOISE_FREQ, 0);
        return (int)(TerrainConfig.BASE_HEIGHT + noise * TerrainConfig.NOISE_AMP);
    }
}
