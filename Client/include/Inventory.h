#pragma once

#include "ItemSlot.h"
#include <vector>

constexpr int HOTBAR_SIZE = 10;
constexpr int INVENTORY_ROWS = 4;
constexpr int INVENTORY_COLS = 10;
constexpr int INVENTORY_SIZE = INVENTORY_COLS * INVENTORY_ROWS;
constexpr int SLOT_SIZE = 64;
constexpr int SPACING = 8; //spacing between slots

class Inventory
{
public:
    Inventory();

    std::vector<ItemSlot> slots;
    int selectedHotbarIndex = 0;
    ItemSlot mouseHeldItem;

    [[nodiscard]] int getCurrentHeldItem() const
    {
        if (selectedHotbarIndex >= 0 && selectedHotbarIndex < HOTBAR_SIZE) //hotbar cycles through 0-9
            return slots[selectedHotbarIndex].itemID;

        return 0;
    }

    void updateSlot(int index, int ID, int quantity);
    static void getSlotScreenPosition(int slotIndex, int& x, int& y, int winW, int winH);
};