#pragma once
#include <array>
#include "Tile.h"

class Chunk {
public:
    static const int SIZE = 16;
    int chunkX, chunkY;
    std::array<std::array<Tile, SIZE>, SIZE> tiles;

    Chunk(int cx, int cy) : chunkX(cx), chunkY(cy) {}

    void setTile(int x, int y, int type) {
        tiles[y][x].type = type;
    }
};
