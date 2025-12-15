#pragma once

#include <string>
#include "ItemRegistry.h"

struct ItemSlot
{
    int itemID = 0;
    int quantity = 0;

    [[nodiscard]] bool isEmpty() const { return itemID == 0 || quantity == 0; }

    [[nodiscard]] std::string getTextureID() const
    {
        if (isEmpty()) return "";
        return ItemRegistry::getInstance().getDefinition(itemID).textureID;
    }
};
