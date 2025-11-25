package com.swagaria.game;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.Tile;
import com.swagaria.data.TileType;

public class Chunk
{
    public static final int SIZE = TerrainConfig.CHUNK_SIZE;
    private final int chunkX;
    private final int chunkY;
    private final Tile[][] tiles; //[x][y] format
    //TODO: make tile array 3d for background walls and layered tiles

    public Chunk(int chunkX, int chunkY)
    {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        this.tiles = new Tile[SIZE][SIZE];
        generate();
    }

    private void generate()
    {
        for (int x = 0; x < SIZE; x++)
        {
            int worldX = chunkX * SIZE + x;
            double heightNoise = TerrainConfig.NOISE.eval(worldX * TerrainConfig.NOISE_FREQ, 0.0);
            int surfaceY = (int) Math.floor(TerrainConfig.BASE_HEIGHT + heightNoise * TerrainConfig.NOISE_AMP);

            for (int y = 0; y < SIZE; y++)
            {
                int worldY = chunkY * SIZE + y;
                Tile tile;

                double caveNoise = TerrainConfig.NOISE.eval(worldX * 0.1, worldY * 0.1);
                boolean isCave = (caveNoise > 0.3) && (worldY < surfaceY - 3); //only check underground (below dirt layer)

                if (worldY > surfaceY)
                    tile = new Tile(TileType.AIR);
                else if (isCave)
                    tile = new Tile(TileType.AIR);
                else if (worldY == surfaceY)
                    tile = new Tile(TileType.GRASS);
                else if (worldY > surfaceY - 3)
                    tile = new Tile(TileType.DIRT);
                else
                    tile = new Tile(TileType.STONE);

                tiles[x][y] = tile;
            }
        }
    }

    public int getChunkX() { return chunkX; }
    public int getChunkY() { return chunkY; }
    public Tile getTile(int localX, int localY)
    {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE)
            return null;
        return tiles[localX][localY];
    }

    public void setTile(int localX, int localY, int tileTypeOrdinal)
    {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE)
            return;
        TileType t = TileType.values()[tileTypeOrdinal];
        tiles[localX][localY] = new Tile(t);
    }

    public String serialize()
    {
        StringBuilder sb = new StringBuilder();
        sb.append("CHUNK_DATA,").append(chunkX).append(",").append(chunkY);

        for (int y = 0; y < SIZE; y++) //bottom -> top
        {
            for (int x = 0; x < SIZE; x++) //left -> right
            {
                Tile t = tiles[x][y];
                int ordinal = (t == null) ? TileType.AIR.ordinal() : t.getType().ordinal();
                sb.append(",").append(ordinal);
            }
        }
        return sb.toString();
    }
}
