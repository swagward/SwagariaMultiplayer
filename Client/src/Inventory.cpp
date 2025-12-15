#include "../include/Inventory.h"
#include "../include/World.h"
#include <algorithm>
#include <iostream>

Inventory::Inventory()
{
    slots.resize(INVENTORY_SIZE);
}

void Inventory::updateSlot(const int  index, const int  ID, const int quantity)
{
    if (index >= 0 && index < INVENTORY_SIZE)
    {
        slots[index].itemID = ID;
        slots[index].quantity = quantity;

        const auto& def = ItemRegistry::getInstance().getDefinition(ID);
        std::cout << "[INVENTORY] Slot " << index << " updated to: " << def.name << " (Qty: " << quantity << ")\n";
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
        const int row = inventoryIndex / INVENTORY_COLUMNS;
        const int col = inventoryIndex % INVENTORY_COLUMNS;

        const int totalWidth = INVENTORY_COLUMNS * SLOT_SIZE + (INVENTORY_COLUMNS - 1) * SPACING;
        const int startX = (winW - totalWidth) / 2;

        // temp layout
        const int startY = winH - SLOT_SIZE - SPACING * 2 - (INVENTORY_ROWS * SLOT_SIZE + (INVENTORY_ROWS - 1) * SPACING);

        x = startX + col * (SLOT_SIZE + SPACING);
        y = startY + row * (SLOT_SIZE + SPACING);
    }
}


