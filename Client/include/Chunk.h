#pragma once
#include <array>
#include "Tile.h"

class Chunk {
public:
    static constexpr int SIZE = 16;
    int chunkX, chunkY;
    std::array<std::array<Tile, SIZE>, SIZE> tiles;

    Chunk(const int cx, const int cy) : chunkX(cx), chunkY(cy) {}

    void setTile(const int x, const int y, const int type) {
        tiles[y][x].type = type;
    }
};