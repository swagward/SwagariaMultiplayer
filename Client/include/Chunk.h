#pragma once
#include <array>
#include "Tile.h"

namespace TileLayer //i hate this
{
    static constexpr int FOREGROUND = 0;
    static constexpr int BACKGROUND = 1;
    static constexpr int NUM_LAYERS = 2;
}

class Chunk
{
public:
    static constexpr int SIZE = 16;
    int chunkX, chunkY;
    //TODO: FIX THIS!!!!!!! MAKE THIS BETTER
    std::array<std::array<std::array<Tile, TileLayer::NUM_LAYERS>, SIZE>, SIZE> tiles; //this too. i fucking hate this

    Chunk(const int cx, const int cy) : chunkX(cx), chunkY(cy) {}

    void setTile(const int x, const int y, const int layer, const int type)
    {
        if (layer >= 0 && layer < TileLayer::NUM_LAYERS)
        {
            tiles[y][x][layer].type = type;
        }
    }
};