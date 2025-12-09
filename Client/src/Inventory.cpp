#include "../include/Inventory.h"
#include "../include/World.h"
#include <algorithm>

#include "../include/TileDefinition.h"

//TODO: Instead of having the inventory client side, it NEEDS to be server side.
//It'll keep track of all the data we store in the inventory (ID's + quantity) with an array
//but that's it, since it doesnt need to know about where each item is in the array.
//The client will receive messages whenever an item changes,
//I.E: you have dirt with a quantity of 0 it removes it, or if a new item is added to the array

Inventory::Inventory()
{
    slots.resize(INVENTORY_SIZE);

    //test data
    //need to stop using bytes for ID representation because itll get confusing later on when adding different items.
    slots[0] = { TileDefinition::ID_GRASS, 1 };
    slots[1] = { TileDefinition::ID_DIRT, 1 };
    slots[2] = { TileDefinition::ID_STONE, 1 };
    slots[3] = { TileDefinition::ID_WOOD_PLANK, 1 };
    slots[4] = { TileDefinition::ID_WOOD_LOG, 1 };
    slots[5] = { TileDefinition::ID_TORCH, 1 };
    slots[7] = { TileDefinition::ID_WOOD_PLANK_BG, 1 };
    slots[8] = { TileDefinition::ID_STONE_BG, 1 };

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


