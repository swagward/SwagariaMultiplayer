package com.swagaria.data.items;

import com.swagaria.data.ItemRegistry;

public abstract class Item
{
    protected final int itemID;
    protected final String name;
    protected final int maxStackSize;

    private static ItemRegistry registry;
    public static void setRegistry(ItemRegistry _registry) { registry = _registry; }

    public Item(int _itemID, String _name, int _maxStackSize)
    {
        itemID = _itemID;
        name = _name;
        maxStackSize = _maxStackSize;
    }


    public int getItemID() { return itemID; }
    public String getName() { return name; }
    public int getMaxStackSize() { return maxStackSize; }

    public abstract boolean use(com.swagaria.game.Player caller, int targetX, int targetY, int slotIndex);
}
