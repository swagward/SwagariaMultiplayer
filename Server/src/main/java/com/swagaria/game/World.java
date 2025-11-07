package com.swagaria.game;

import java.util.*;

public class World {
    private final int worldWidth = 16;
    private final int worldHeight = 16;
    private final int tileSize = 16;
    private final Map<String, Chunk> chunks = new HashMap<>();

    public World() {
        // generate chunks so (0,0) is bottom-left
        for (int cy = 0; cy < worldHeight; cy++) {
            for (int cx = 0; cx < worldWidth; cx++) {
                addChunk(new Chunk(cx, cy, 1));
            }
        }
        System.out.println("[World] Generated " + chunks.size() + " chunks.");
    }

    private void addChunk(Chunk chunk) {
        chunks.put(key(chunk.getChunkX(), chunk.getChunkY()), chunk);
    }

    public Collection<Chunk> getAllChunks() {
        return chunks.values();
    }

    private String key(int x, int y) {
        return x + "," + y;
    }

    public int getTileSize() {
        return tileSize;
    }

    public boolean isSolidTile(int tileX, int tileY) {
        int chunkSize = Chunk.SIZE;

        int chunkX = (int)Math.floor((float)tileX / chunkSize);
        int chunkY = (int)Math.floor((float)tileY / chunkSize);

        String key = chunkX + "," + chunkY;
        Chunk chunk = chunks.get(key);
        if (chunk == null)
            return false;

        int localX = tileX - chunkX * chunkSize;
        int localY = tileY - chunkY * chunkSize;

        Tile tile = chunk.getTile(localX, localY);
        if (tile == null)
            return false;

        int type = tile.getType();
        return type == 1 || type == 2 || type == 3;
    }

}