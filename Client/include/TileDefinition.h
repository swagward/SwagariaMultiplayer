#pragma once

//Eventually store everything in an ItemID class, TileID class, etc.
//TileID will mimic ItemID but with an offset, like:
//ItemID.Grass = 1;
//TileID.Grass = 1 + 1000;
//This will make it easier to add new tiles and items, keeping them separate fom each other.

namespace TileDefinition
{
    constexpr int ID_AIR = 0;
    constexpr int ID_GRASS = 1;
    constexpr int ID_DIRT = 2;
    constexpr int ID_STONE = 3;
    constexpr int ID_WOOD_LOG = 4;
    constexpr int ID_TORCH = 5;
    constexpr int ID_WOOD_PLANK = 6;

    constexpr int ID_WOOD_PLANK_BG = 7;
    constexpr int ID_STONE_BG = 8;
}