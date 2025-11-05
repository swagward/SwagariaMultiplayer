package com.swagaria.game;

import java.util.*;

public class World {
    private final int worldWidth = 16;
    private final int worldHeight = 16;
    private final Map<String, Chunk> chunks = new HashMap<>();

    public World() {
        // generate chunks so (0,0) is bottom-left
        for (int cy = 0; cy < worldHeight; cy++) {       // bottom to top
            for (int cx = 0; cx < worldWidth; cx++) {    // left to right
                addChunk(new Chunk(cx, cy, 1));
                String key = cx + "," + cy;
                if(chunks.containsKey(key))
                    System.out.print("(" + cx + ", " + cy + ") ");
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
}
