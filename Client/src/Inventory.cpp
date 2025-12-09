#include "../include/Inventory.h"
#include "../include/World.h"
#include <algorithm>

Inventory::Inventory()
{
    slots.resize(INVENTORY_SIZE);

    //test data
    //need to stop using bytes for ID representation because itll get confusing later on when adding different items.
    slots[0] = { 1, 99 };
    slots[1] = { 5, 25 };
    slots[2] = { 7, 25 };
    slots[10] = { 6, 1 };
    slots[11] = { 9, 57 };

    //TODO: add tools for player to start with in slot 0, 1, 2
}

void Inventory::updateSlot(const int  index, const int  ID, const int quantity)
{
    //update slot when player uses item/picks up item/moves item around inventory
    if (index >= 0 && index < INVENTORY_SIZE)
    {
        slots[index].itemID = ID;
        slots[index].quantity = quantity;
    }
}

void Inventory::getSlotScreenPosition(const int slotIndex, int& x, int& y, const int winW, const int winH)
{
    if (slotIndex < HOTBAR_SIZE)
    {
        const int col = slotIndex;

        const int totalWidth = HOTBAR_SIZE * SLOT_SIZE + (HOTBAR_SIZE - 1) * SPACING;
        const int startX = (winW - totalWidth) / 2;

        x = startX + col * (SLOT_SIZE + SPACING);
        y = winH - SLOT_SIZE - SPACING;
    }
    else
    {
        const int inventoryIndex = slotIndex - HOTBAR_SIZE;
        const int row = inventoryIndex / INVENTORY_COLS;
        const int col = inventoryIndex % INVENTORY_COLS;

        const int totalWidth = INVENTORY_COLS * SLOT_SIZE + (INVENTORY_COLS - 1) * SPACING;
        const int startX = (winW - totalWidth) / 2;

        //temp layout
        const int startY = winH - SLOT_SIZE - SPACING * 2 - (INVENTORY_ROWS * SLOT_SIZE + (INVENTORY_ROWS - 1) * SPACING);

        x = startX + col * (SLOT_SIZE + SPACING);
        y = startY + row * (SLOT_SIZE + SPACING);
    }
}


