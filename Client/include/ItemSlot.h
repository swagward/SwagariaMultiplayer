#pragma once

#include <string>

struct ItemSlot
{
    int itemID = 0;
    int quantity = 0;

    [[nodiscard]] bool isEmpty() const{ return itemID == 0 || quantity == 0; }
    [[nodiscard]] std::string getTextureID() const
    {
        switch (itemID)
        {
            case 1: return "grass";
            case 2: return "dirt";
            case 3: return "stone";
            case 4: return "wood_log";
            case 5: return "torch";
            case 6: return "wood_plank";
            case 7: return "wood_plank_bg";
            case 8: return "stone_bg";
            default: return "missing_texture";
        }
    }
};