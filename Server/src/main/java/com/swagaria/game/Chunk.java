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
    private final World world;
    private final Tile[][][] tiles;
    private final Random random;

    public Chunk(int chunkX, int chunkY, World world)
    {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        this.world = world;
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
                int fgTypeId;
                int bgTypeId = TileDefinition.ID_AIR;

                double caveNoise = TerrainConfig.NOISE.eval(worldX * 0.1, worldY * 0.1);
                boolean isCave = (caveNoise > 0.3) && (worldY < surfaceY - 3);


                if (worldY == 0)                                      //bedrock at y=0
                    fgTypeId = TileDefinition.ID_BEDROCK;
                else if (worldY > surfaceY)                           //above surface level
                    fgTypeId = TileDefinition.ID_AIR;
                else if (worldY == surfaceY)                          //grass layer
                {
                    fgTypeId = TileDefinition.ID_GRASS;
                    bgTypeId = TileDefinition.ID_DIRT_BG;
                }
                else if (worldY > surfaceY - 3)                       //dirt layer
                {
                    fgTypeId = TileDefinition.ID_DIRT;
                    bgTypeId = TileDefinition.ID_DIRT_BG;
                }
                else                                                  //stone & slate level
                {
                    //bottom third of the world being slate
                    int stoneLayerTop = surfaceY - 3;
                    int slateLayer = stoneLayerTop / 3;
                    boolean isSlateRegion = worldY <= slateLayer;

                    if (isCave)
                        fgTypeId = TileDefinition.ID_AIR;
                    else
                        fgTypeId = isSlateRegion ? TileDefinition.ID_SLATE : TileDefinition.ID_STONE;

                    bgTypeId = isSlateRegion ? TileDefinition.ID_SLATE_BG : TileDefinition.ID_STONE_BG;
                }

                tiles[x][y][TileLayer.FOREGROUND] = new Tile(fgTypeId);
                tiles[x][y][TileLayer.BACKGROUND] = new Tile(bgTypeId);
            }
        }
    }

    public void generateTrees() {
        int minSpacing = 3; //controls how close trees can spawn *next* to each other
        int treeBuffer = 0;

        for (int x = 0; x < SIZE; x++) {
            if (treeBuffer > 0) {
                treeBuffer--;
                continue;
            }

            for (int y = SIZE - 2; y >= 0; y--)
            {
                Tile fg = tiles[x][y][TileLayer.FOREGROUND];
                Tile above = tiles[x][y + 1][TileLayer.FOREGROUND];

                if (fg.getTileID() == TileDefinition.ID_GRASS && above.getTileID() == TileDefinition.ID_AIR)
                {
                    if (random.nextDouble() < TerrainConfig.TREE_CHANCE)
                    {
                        //need to convert chunk X/Y back into world coords
                        //since the world.setTileGlobal uses global coords, not the chunks local coords
                        int worldX = (this.chunkX * SIZE) + x;
                        int worldY = (this.chunkY * SIZE) + y + 1;
                        growTree(worldX, worldY);
                        treeBuffer = minSpacing;
                        break;
                    }
                }
            }
        }

        generateFlora();
    }

    private void growTree(int startX, int startY) {
        int trunkHeight = 5 + random.nextInt(3);
        int topY = startY + trunkHeight - 1;

        //generate logs in background
        for (int i = 0; i < trunkHeight; i++)
            world.setTileGlobal(startX, startY + i, TileLayer.BACKGROUND, TileDefinition.ID_WOOD_LOG);

        //generate leaves in foreground with pattern
        world.setTileGlobal(startX, topY + 1, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);

        world.setTileGlobal(startX, topY, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);
        world.setTileGlobal(startX - 1, topY, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);
        world.setTileGlobal(startX + 1, topY, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);

        world.setTileGlobal(startX, topY - 1, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);
        world.setTileGlobal(startX - 1, topY - 1, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);
        world.setTileGlobal(startX + 1, topY - 1, TileLayer.FOREGROUND, TileDefinition.ID_LEAVES);
    }

    private void generateFlora()
    {
        for(int x = 0; x < SIZE; x++)
        {
            for(int y = SIZE - 2; y >= 1; y--)
            {
                Tile foregroundCheck = tiles[x][y][TileLayer.FOREGROUND];
                Tile backgroundCheck = tiles[x][y][TileLayer.BACKGROUND];
                Tile belowCheck = tiles[x][y - 1][TileLayer.FOREGROUND];

                if(belowCheck.getTileID() == TileDefinition.ID_GRASS &&
                   foregroundCheck.getTileID() == TileDefinition.ID_AIR &&
                   backgroundCheck.getTileID() == TileDefinition.ID_AIR)
                {
                    if(random.nextDouble() < TerrainConfig.FLORA_CHANCE)
                    {
                        int floraID;
                        if(random.nextDouble() < TerrainConfig.FLOWER_CHANCE)
                            floraID = TileDefinition.ID_FLOWER;
                        else
                            floraID = TileDefinition.ID_TALL_GRASS;

                        tiles[x][y][TileLayer.FOREGROUND].setTileID(floraID);
                        break;
                    }
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
            tile.setTileID(tileTypeId);
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
                    int typeId = (t == null) ? TileDefinition.ID_AIR : t.getTileID();
                    sb.append(",").append(typeId);

                }
            }
        }
        return sb.toString();
    }
}