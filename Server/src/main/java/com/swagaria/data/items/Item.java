package com.swagaria.data.items;

public abstract class Item
{
    protected final int itemID;
    protected final String name;
    protected final int maxStackSize;

    public Item(int _itemID, String _name, int _maxStackSize)
    {
        itemID = _itemID;
        name = _name;
        maxStackSize = _maxStackSize;
    }

    public int getItemID() { return itemID; }
    public String getName() { return name; }
    public int getMaxStackSize() { return maxStackSize; }

    public abstract boolean use(com.swagaria.game.Player caller, int posX, int posY, int slotIndex);
}
