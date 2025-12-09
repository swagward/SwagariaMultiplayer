package com.swagaria.game;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.Tile;
import com.swagaria.data.TileDefinition;
import com.swagaria.data.TileLayer;

public class Chunk
{
    public static final int SIZE = TerrainConfig.CHUNK_SIZE;
    private final int chunkX;
    private final int chunkY;
    private final Tile[][][] tiles;

    public Chunk(int chunkX, int chunkY)
    {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        this.tiles = new Tile[SIZE][SIZE][TileLayer.NUM_LAYERS];
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
                int fgTypeId = TileDefinition.ID_AIR;
                int bgTypeId = TileDefinition.ID_AIR;

                //cave gen
                double caveNoise = TerrainConfig.NOISE.eval(worldX * 0.1, worldY * 0.1);
                boolean isCave = (caveNoise > 0.3) && (worldY < surfaceY - 3);

                if (worldY > surfaceY) //above surface
                    fgTypeId = TileDefinition.ID_AIR;
                else if (worldY == surfaceY) //at surface
                {
                    fgTypeId = TileDefinition.ID_GRASS;
                    bgTypeId = TileDefinition.ID_AIR; // TODO: ADD DIRT WALLS HERE TOO!!!!!!
                }
                else if (worldY > surfaceY - 3) //dirt layer
                {
                    fgTypeId = TileDefinition.ID_DIRT;
                    bgTypeId = TileDefinition.ID_AIR; //TODO: ADD DIRT WALLS!!!!!!
                }
                else //stone layer
                {
                    if (isCave)
                    {
                        fgTypeId = TileDefinition.ID_AIR;
                        bgTypeId = TileDefinition.ID_STONE_BG;
                    }
                    else
                    {
                        fgTypeId = TileDefinition.ID_STONE;
                        bgTypeId = TileDefinition.ID_STONE_BG;
                    }
                }

                tiles[x][y][TileLayer.FOREGROUND] = new Tile(fgTypeId);
                tiles[x][y][TileLayer.BACKGROUND] = new Tile(bgTypeId);
            }
        }
    }

    public int getChunkX() { return chunkX; }
    public int getChunkY() { return chunkY; }

    public Tile getTile(int localX, int localY, int layer)
    {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE || layer < 0 || layer >= TileLayer.NUM_LAYERS)
            return null;
        return tiles[localX][localY][layer];
    }

    public void setTile(int localX, int localY, int layer, int tileTypeId)
    {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE || layer < 0 || layer >= TileLayer.NUM_LAYERS)
            return;

        //if tile exists then update it, else just make new instance of it
        Tile tile = tiles[localX][localY][layer];
        if (tile != null)
            tile.setTypeId(tileTypeId);
        else
            tiles[localX][localY][layer] = new Tile(tileTypeId);
    }

    public String serialize()
    {
        StringBuilder sb = new StringBuilder();
        sb.append("CHUNK_DATA,").append(chunkX).append(",").append(chunkY);

        for (int layer = 0; layer < TileLayer.NUM_LAYERS; layer++) {

            for (int y = 0; y < SIZE; y++) //bottom -> top
            {
                for (int x = 0; x < SIZE; x++) //left -> right
                {
                    Tile t = tiles[x][y][layer];
                    //serialize tile ID to be sent to client
                    int typeId = (t == null) ? TileDefinition.ID_AIR : t.getTypeId();
                    sb.append(",").append(typeId);
                }
            }
        }
        return sb.toString();
    }
}