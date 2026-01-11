#pragma once
#include <array>
#include "Tile.h"

namespace TileLayer
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
    std::array<std::array<std::array<Tile, TileLayer::NUM_LAYERS>, SIZE>, SIZE> tiles; //i fucking hate this

    Chunk(const int cx, const int cy) : chunkX(cx), chunkY(cy)
    {
        for(int y = 0; y < SIZE; ++y)
            for(int x = 0; x < SIZE; ++x)
                for(int l = 0; l < TileLayer::NUM_LAYERS; ++l)
                    tiles[y][x][l].type = 0;
    }

    void setTile(const int x, const int y, const int layer, const int type)
    {
        if (layer >= 0 && layer < TileLayer::NUM_LAYERS)
        {
            tiles[y][x][layer].type = type;
        }
    }
    [[nodiscard]] Tile getTile(const int x, const int y, const int layer) const
    {
        if (x >= 0 && x < SIZE && y >= 0 && y < SIZE && layer >= 0 && layer < TileLayer::NUM_LAYERS)
            return tiles[y][x][layer];

        return {};
    }

};