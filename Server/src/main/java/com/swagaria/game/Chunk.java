package com.swagaria.game;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.Tile;
import com.swagaria.data.TileType;

/**
 * Chunk coordinate system & conventions:
 * - tiles[x][y]
 *   - x: 0..CHUNK_SIZE-1 left -> right
 *   - y: 0..CHUNK_SIZE-1 bottom -> top (y=0 is the bottom row)
 *
 * - Generation loops must follow:
 *     for x=0..CHUNK_SIZE-1
 *       for y=0..CHUNK_SIZE-1
 *         worldX = chunkX*CHUNK_SIZE + x
 *         worldY = chunkY*CHUNK_SIZE + y
 *         compute tile based on worldY (bottom-up)
 *         tiles[x][y] = new Tile(...)
 *
 * - Serialization order (network):
 *     write tiles in the same order the client expects to read:
 *       for y = 0..CHUNK_SIZE-1  (bottom -> top)
 *         for x = 0..CHUNK_SIZE-1 (left -> right)
 *           append tiles[x][y]
 *
 * This ensures no flips when client reconstructs chunk by:
 *   x = i % CHUNK_SIZE
 *   y = i / CHUNK_SIZE
 */
public class Chunk {
    public static final int SIZE = TerrainConfig.CHUNK_SIZE;

    private final int chunkX;
    private final int chunkY;
    private final Tile[][] tiles; // tiles[x][y]

    public Chunk(int chunkX, int chunkY) {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        this.tiles = new Tile[SIZE][SIZE];
        generate(); // fill tiles according to the convention
    }

    private void generate() {
        // simple noise-based surface like earlier; y is bottom->top
        for (int x = 0; x < SIZE; x++) {
            int worldX = chunkX * SIZE + x;

            double noise = TerrainConfig.NOISE.eval(worldX * TerrainConfig.NOISE_FREQ, 0.0);
            int surfaceY = (int) Math.floor(TerrainConfig.BASE_HEIGHT + noise * TerrainConfig.NOISE_AMP);

            for (int y = 0; y < SIZE; y++) {
                int worldY = chunkY * SIZE + y; // bottom -> top

                Tile tile;
                if (worldY > surfaceY) {
                    tile = new Tile(TileType.AIR);
                } else if (worldY == surfaceY) {
                    tile = new Tile(TileType.GRASS);
                } else if (worldY > surfaceY - 3) {
                    tile = new Tile(TileType.DIRT);
                } else {
                    tile = new Tile(TileType.STONE);
                }

                tiles[x][y] = tile; // note tiles[x][y] (consistent)
            }
        }
    }

    public int getChunkX() { return chunkX; }
    public int getChunkY() { return chunkY; }

    // local coords expected in convention: x=0..SIZE-1, y=0..SIZE-1 bottom->top
    public Tile getTile(int localX, int localY) {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE) return null;
        return tiles[localX][localY];
    }

    public void setTile(int localX, int localY, int tileTypeOrdinal) {
        if (localX < 0 || localX >= SIZE || localY < 0 || localY >= SIZE) return;
        TileType t = TileType.values()[tileTypeOrdinal];
        tiles[localX][localY] = new Tile(t);
    }

    // Serialize bottom->top, left->right so client parsing (x = i%SIZE, y = i/SIZE)
    public String serialize() {
        StringBuilder sb = new StringBuilder();
        sb.append("CHUNK_DATA,").append(chunkX).append(",").append(chunkY);

        // write in this exact order: y=0..SIZE-1, x=0..SIZE-1
        for (int y = 0; y < SIZE; y++) {              // bottom -> top
            for (int x = 0; x < SIZE; x++) {          // left -> right
                Tile t = tiles[x][y];
                int ordinal = (t == null) ? TileType.AIR.ordinal() : t.getType().ordinal();
                sb.append(",").append(ordinal);
            }
        }
        return sb.toString();
    }
}
