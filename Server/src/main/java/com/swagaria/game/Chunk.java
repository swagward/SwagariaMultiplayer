package com.swagaria.game;

import com.swagaria.data.terrain.TerrainConfig;
import com.swagaria.data.terrain.Tile;
import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.data.terrain.TileLayer;

import java.util.Random;

public class Chunk
{
    public static final int SIZE = TerrainConfig.CHUNK_SIZE;
    private final int chunkX;
    private final int chunkY;
    private final Tile[][][] tiles;
    private final Random random;

    public Chunk(int chunkX, int chunkY)
    {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        this.tiles = new Tile[SIZE][SIZE][TileLayer.NUM_LAYERS];
        this.random = new Random(TerrainConfig.SEED + (long) chunkX * 31337 + (long) chunkY * 713);
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
                    bgTypeId = TileDefinition.ID_DIRT_BG;
                }
                else if (worldY > surfaceY - 3) //dirt layer
                {
                    fgTypeId = TileDefinition.ID_DIRT;
                    bgTypeId = TileDefinition.ID_DIRT_BG;
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
        generateTrees();
    }

    private void generateTrees()
    {
        int minSpacing = 4; // Slightly more space for the leaf canopy
        int treeBuffer = 0;

        for (int x = 1; x < SIZE - 1; x++) // Bounds check so leaves don't spill out of array
        {
            if (treeBuffer > 0) {
                treeBuffer--;
                continue;
            }

            for (int y = 0; y < SIZE - 8; y++) // Ensure height for the trunk + leaves
            {
                Tile currentTile = tiles[x][y][TileLayer.FOREGROUND];
                Tile aboveTile = tiles[x][y + 1][TileLayer.FOREGROUND];

                if (currentTile.getTypeId() == TileDefinition.ID_GRASS &&
                        aboveTile.getTypeId() == TileDefinition.ID_AIR)
                {
                    if (random.nextDouble() < TerrainConfig.TREE_CHANCE)
                    {
                        growTree(x, y + 1);
                        treeBuffer = minSpacing;
                        break;
                    }
                }
            }
        }
    }

    private void growTree(int startX, int startY) {
        int height = 4 + random.nextInt(3); // Random 4, 5, or 6

        // 1. Grow Trunk
        for (int i = 0; i < height; i++) {
            int currentY = startY + i;
            if (currentY < SIZE) {
                tiles[startX][currentY][TileLayer.FOREGROUND].setTypeId(TileDefinition.ID_WOOD_LOG);
            }
        }

        // 2. Add Leaves at the top
        int topY = startY + height - 1;

        // Simple cross/diamond canopy around the top log
        // (x, y+1), (x, y-1), (x+1, y), (x-1, y)
        int[][] leafOffsets = {
                {0, 1}, {0, -1}, {1, 0}, {-1, 0}, // Cardinal
                {1, 1}, {-1, 1}, {1, -1}, {-1, -1} // Diagonals for a bushier look
        };

        for (int[] offset : leafOffsets) {
            int leafX = startX + offset[0];
            int leafY = topY + offset[1];

            // Boundary checks
            if (leafX >= 0 && leafX < SIZE && leafY >= 0 && leafY < SIZE) {
                Tile leafTile = tiles[leafX][leafY][TileLayer.FOREGROUND];
                // Only place leaves in air so we don't overwrite the trunk
                if (leafTile.getTypeId() == TileDefinition.ID_AIR) {
                    leafTile.setTypeId(TileDefinition.ID_LEAVES);
                }
            }
        }
    }

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