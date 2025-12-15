#pragma once

#include "ItemSlot.h"
#include <vector>

constexpr int SLOT_SIZE = 64;
constexpr int SPACING = 8;
constexpr int HOTBAR_SIZE = 10; //0-9
constexpr int INVENTORY_ROWS = 3;
constexpr int INVENTORY_COLUMNS = 10;
constexpr int INVENTORY_SIZE = HOTBAR_SIZE + (INVENTORY_ROWS * INVENTORY_COLUMNS);

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