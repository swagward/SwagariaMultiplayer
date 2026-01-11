package com.swagaria.game;

import com.swagaria.data.terrain.TerrainConfig;
import com.swagaria.data.terrain.Tile;
import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.data.terrain.TileLayer;
import com.swagaria.data.components.CollisionComponent;

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
        for (int cy = 0; cy < TerrainConfig.WORLD_CHUNKS_Y; cy++)
        {
            for (int cx = 0; cx < TerrainConfig.WORLD_CHUNKS_X; cx++)
            {
                Chunk chunk = new Chunk(cx, cy, this);
                chunks[cx][cy] = chunk;
            }
        }

        for (int cy = 0; cy < TerrainConfig.WORLD_CHUNKS_Y; cy++)
            for (int cx = 0; cx < TerrainConfig.WORLD_CHUNKS_X; cx++)
                chunks[cx][cy].generateTrees();
    }

    public List<Chunk> getAllChunks()
    {
        List<Chunk> list = new ArrayList<>();
        for (int y = 0; y < TerrainConfig.WORLD_CHUNKS_Y; y++)
            for (int x = 0; x < TerrainConfig.WORLD_CHUNKS_X; x++)
                list.add(chunks[x][y]);

        return list;
    }

    public Chunk getChunk(int cx, int cy)
    {
        if (cx < 0 || cx >= TerrainConfig.WORLD_CHUNKS_X || cy < 0 || cy >= TerrainConfig.WORLD_CHUNKS_Y) return null;
        return chunks[cx][cy];
    }

    public Tile getTileAt(int worldX, int worldY, int layer)
    {
        if (worldX < 0 || worldY < 0 ||
                worldX >= TerrainConfig.WORLD_WIDTH ||
                worldY >= TerrainConfig.WORLD_HEIGHT)
            return null;

        int realY = TerrainConfig.WORLD_HEIGHT - 1 - worldY;

        int chunkX = worldX / TerrainConfig.CHUNK_SIZE;
        int chunkY = realY / TerrainConfig.CHUNK_SIZE;

        Chunk chunk = getChunk(chunkX, chunkY);
        if (chunk == null)
            return null;

        int localX = worldX % TerrainConfig.CHUNK_SIZE;
        int localY = realY % TerrainConfig.CHUNK_SIZE;

        return chunk.getTile(localX, localY, layer);
    }

    public void setTileAt(int worldX, int worldY, int layer, int tileTypeId)
    {
        //acts as the players way of placing tiles
        if (worldX < 0 || worldY < 0 ||
                worldX >= TerrainConfig.WORLD_WIDTH ||
                worldY >= TerrainConfig.WORLD_HEIGHT ||
                layer < 0 || layer >= TileLayer.NUM_LAYERS) return;

        int realY = TerrainConfig.WORLD_HEIGHT - 1 - worldY;

        int chunkX = worldX / TerrainConfig.CHUNK_SIZE;
        int chunkY = realY / TerrainConfig.CHUNK_SIZE;

        Chunk chunk = getChunk(chunkX, chunkY);
        if (chunk == null)
            return;

        int localX = worldX % TerrainConfig.CHUNK_SIZE;
        int localY = realY % TerrainConfig.CHUNK_SIZE;

        if (TileDefinition.getDefinition(tileTypeId).tileID == TileDefinition.ID_AIR && tileTypeId != TileDefinition.ID_AIR)
            return;

        chunk.setTile(localX, localY, layer, tileTypeId);
    }

    public void setTileGlobal(int worldX, int worldY, int layer, int tileID)
    {
        int chunkX = Math.floorDiv(worldX, Chunk.SIZE);
        int chunkY = Math.floorDiv(worldY, Chunk.SIZE);

        Chunk chunk = getChunk(chunkX, chunkY);
        if(chunk != null)
        {
            int localX = Math.floorMod(worldX, Chunk.SIZE);
            int localY = Math.floorMod(worldY, Chunk.SIZE);
            chunk.setTile(localX, localY, layer, tileID);
        }
    }

    //for collision
    public boolean isSolidTile(int worldX, int worldY)
    {
        Tile tile = getTileAt(worldX, worldY, TileLayer.FOREGROUND);
        if (tile == null || tile.getTileID() == TileDefinition.ID_AIR)
            return false;

        if (tile.getTileID() == TileDefinition.ID_WOOD_PLATFORM) return false;

        TileDefinition definition = tile.getDefinition();
        if (definition.hasComponent(CollisionComponent.class))
            return definition.getComponent(CollisionComponent.class).blocksMovement;

        return false; //default to no collision
    }

    public boolean isPlatformTile(int worldX, int worldY) {
        Tile tile = getTileAt(worldX, worldY, TileLayer.FOREGROUND);
        if (tile == null) return false;

        return tile.getTileID() == TileDefinition.ID_WOOD_PLATFORM;
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

    public float calculateDistanceSq(float playerX, float playerY, int tileX, int tileY)
    {
        float tileCenterX = tileX + 0.5f;
        float tileCenterY = tileY + 0.5f;

        float dx = playerX - tileCenterX;
        float dy = playerY - tileCenterY;

        return (dx * dx) + (dy * dy);
    }
}