package com.swagaria.data.inventory;

import com.swagaria.data.ItemRegistry;
import com.swagaria.data.items.Item;
import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.game.Player;

public class Inventory
{
    private static final int NUM_SLOTS = 40; //10 for hotbar, 30 for storage
    private final ItemSlot[] slots;
    private final Player owner;

    public Inventory(Player _owner)
    {
        this.owner = _owner;
        this.slots = new ItemSlot[NUM_SLOTS];

        //initialise all slots as empty
        for (int i = 0; i < NUM_SLOTS; i++)
            slots[i] = new ItemSlot(null, 0);

        //give player temp items
        Item grass = ItemRegistry.getByID(TileDefinition.ID_GRASS);
        Item woodPlank = ItemRegistry.getByID(TileDefinition.ID_WOOD_PLANK);
        Item woodPlankBG = ItemRegistry.getByID(TileDefinition.ID_WOOD_PLANK_BG);
        Item torch = ItemRegistry.getByID(TileDefinition.ID_TORCH);

        slots[0] = new ItemSlot(grass, 99);
        slots[1] = new ItemSlot(woodPlank, 130);
        slots[2] = new ItemSlot(woodPlankBG, 99);
        slots[3] = new ItemSlot(woodPlank, 54);
    }

    public int getNumSlots() { return NUM_SLOTS; }

    public ItemSlot getSlot(int index)
    {
        if (index < 0 || index >= NUM_SLOTS)
            return new ItemSlot(null, 0); //safely return empty slot

        return slots[index];
    }

    public void decreaseQuantity(int index, int amount)
    {
        ItemSlot slot = getSlot(index);
        if (slot.isEmpty()) return; //nothing to decrease

        int newQuantity = slot.getQuantity() - amount;
        if (newQuantity <= 0)
            slots[index] = new ItemSlot(null, 0);
        else slot.setQuantity(newQuantity);

        owner.getServer().broadcast("INV_UPDATE," + owner.getId() + "," + index + "," + slot.serialize());
    }

    public String serialize()
    {
        StringBuilder sb = new StringBuilder();
        sb.append("INV_SYNC");
        for (int i = 0; i < NUM_SLOTS; i++)
            sb.append(",").append(slots[i].serialize());

        return sb.toString();
    }
}
