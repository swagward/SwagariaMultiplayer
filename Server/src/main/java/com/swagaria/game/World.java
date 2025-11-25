package com.swagaria.game;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.Tile;
import com.swagaria.data.TileType;

import java.util.ArrayList;
import java.util.List;

public class World
{
    private final Chunk[][] chunks;

    public World()
    {
        chunks = new Chunk[TerrainConfig.WORLD_CHUNKS_X][TerrainConfig.WORLD_CHUNKS_Y];
        generate();
        System.out.println("[World] Generated " + (TerrainConfig.WORLD_CHUNKS_X * TerrainConfig.WORLD_CHUNKS_Y) + " chunks.");
    }

    private void generate()
    {
        System.out.println("[World] Generating " + TerrainConfig.WORLD_CHUNKS_X + "x" + TerrainConfig.WORLD_CHUNKS_Y + " chunks...");
        for (int cy = 0; cy < TerrainConfig.WORLD_CHUNKS_Y; cy++)
        {
            for (int cx = 0; cx < TerrainConfig.WORLD_CHUNKS_X; cx++)
            {
                Chunk chunk = new Chunk(cx, cy);
                chunks[cx][cy] = chunk;
            }
        }
        System.out.println("[World] Generation complete.");
    }

    public List<Chunk> getAllChunks()
    {
        List<Chunk> list = new ArrayList<>();
        for (int y = 0; y < TerrainConfig.WORLD_CHUNKS_Y; y++)
        {
            for (int x = 0; x < TerrainConfig.WORLD_CHUNKS_X; x++)
            {
                list.add(chunks[x][y]);
            }
        }
        return list;
    }

    public Chunk getChunk(int cx, int cy)
    {
        if (cx < 0 || cx >= TerrainConfig.WORLD_CHUNKS_X || cy < 0 || cy >= TerrainConfig.WORLD_CHUNKS_Y) return null;
        return chunks[cx][cy];
    }

    public Tile getTileAt(int worldX, int worldY)
    {
        if (worldX < 0 || worldY < 0 ||
                worldX >= TerrainConfig.WORLD_WIDTH ||
                worldY >= TerrainConfig.WORLD_HEIGHT)
            return null; //out of bounds, not a valid chunk

        int realY = TerrainConfig.WORLD_HEIGHT - 1 - worldY;

        int chunkX = worldX / TerrainConfig.CHUNK_SIZE;
        int chunkY = realY / TerrainConfig.CHUNK_SIZE;

        Chunk chunk = getChunk(chunkX, chunkY);
        if (chunk == null)
            return null;

        int localX = worldX % TerrainConfig.CHUNK_SIZE;
        int localY = realY % TerrainConfig.CHUNK_SIZE;

        return chunk.getTile(localX, localY);
    }

    public boolean setTileAt(int worldX, int worldY, int tileTypeOrdinal)
    {
        if (worldX < 0 || worldY < 0 ||
                worldX >= TerrainConfig.WORLD_WIDTH ||
                worldY >= TerrainConfig.WORLD_HEIGHT)
            return false; //out of world bounds, dont set tile

        int realY = TerrainConfig.WORLD_HEIGHT - 1 - worldY;

        int chunkX = worldX / TerrainConfig.CHUNK_SIZE;
        int chunkY = realY / TerrainConfig.CHUNK_SIZE;

        Chunk chunk = getChunk(chunkX, chunkY);
        if (chunk == null)
            return false;

        int localX = worldX % TerrainConfig.CHUNK_SIZE;
        int localY = realY % TerrainConfig.CHUNK_SIZE;

        //make sure tile type has actual ID
        if (tileTypeOrdinal < 0 || tileTypeOrdinal >= TileType.values().length)
            return false;

        chunk.setTile(localX, localY, tileTypeOrdinal);
        return true;
    }

    public boolean isSolidTile(int worldX, int worldY)
    {
        Tile tile = getTileAt(worldX, worldY);
        if (tile == null || tile.getType() == TileType.AIR)
            return false;

        TileType type = tile.getType();
        return (type == TileType.STONE ||
                type == TileType.DIRT ||
                type == TileType.GRASS ||
                type == TileType.WOOD_PLANK); //exclude air, logs and BGs
    }

    public int[] findSpawnTile()
    {   //find valid spawn position for players
        int worldMidX = TerrainConfig.WORLD_WIDTH / 2;
        int surfaceY = getSurfaceHeightAt(worldMidX);
        return new int[] { worldMidX, surfaceY + 40 };
    }

    private int getSurfaceHeightAt(int worldX)
    {
        double noise = TerrainConfig.NOISE.eval(worldX * TerrainConfig.NOISE_FREQ, 0);
        return (int)(TerrainConfig.BASE_HEIGHT + noise * TerrainConfig.NOISE_AMP);
    }
}
