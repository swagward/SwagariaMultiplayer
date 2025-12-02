package com.swagaria.game;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.Tile;
import com.swagaria.data.TileDefinition; // Import the new definition registry

public class Chunk
{
    public static final int SIZE = TerrainConfig.CHUNK_SIZE;
    private final int chunkX;
    private final int chunkY;
    private final Tile[][] tiles; // Array now holds the dynamic Tile instance (ID + Damage)
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
            int surfaceY = (int) Math.floor(TerrainConfig.BASE_HEIGHT + heightNoise * TerrainConfig.NOISE_AMP + 40);

            for (int y = 0; y < SIZE; y++)
            {
                int worldY = chunkY * SIZE + y;
                int tileTypeId;

                double caveNoise = TerrainConfig.NOISE.eval(worldX * 0.1, worldY * 0.1);
                boolean isCave = (caveNoise > 0.3) && (worldY < surfaceY - 3);

                if (worldY > surfaceY) //above surface
                    tileTypeId = TileDefinition.ID_AIR;
                else if (worldY == surfaceY) //at surface
                    tileTypeId = TileDefinition.ID_GRASS;
                else if (worldY > surfaceY - 3) //dirt layer
                    tileTypeId = TileDefinition.ID_DIRT;
                else //stone layer
                {
                    if (isCave)
                        tileTypeId = TileDefinition.ID_STONE_BG;
                    else
                        tileTypeId = TileDefinition.ID_STONE;
                }

                tiles[x][y] = new Tile(tileTypeId);
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

    public void setTile(int localX, int localY, int tileTypeId)
    {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE)
            return;

        // If tile exists, update its type. Otherwise, create a new Tile instance.
        if (tiles[localX][localY] != null) {
            tiles[localX][localY].setTypeId(tileTypeId);
        } else {
            tiles[localX][localY] = new Tile(tileTypeId);
        }
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
                // Serialize Tile ID and Damage (Damage is 0 for new chunks, but include it)
                int typeId = (t == null) ? TileDefinition.ID_AIR : t.getTypeId();
                float damage = (t == null) ? 0.0f : t.getDamage();

                // We'll only send the Tile ID for now, simplifying the client update for existing chunks
                sb.append(",").append(typeId);
            }
        }
        return sb.toString();
    }
}