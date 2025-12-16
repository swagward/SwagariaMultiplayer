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
        Item copperPickaxe = ItemRegistry.getByID(100);
        Item copperAxe = ItemRegistry.getByID(101);
        Item copperHammer = ItemRegistry.getByID(102);

        Item grass = ItemRegistry.getByID(TileDefinition.ID_GRASS);
        Item woodPlank = ItemRegistry.getByID(TileDefinition.ID_WOOD_PLANK);
        Item woodPlankBG = ItemRegistry.getByID(TileDefinition.ID_WOOD_PLANK_BG);
        Item torch = ItemRegistry.getByID(TileDefinition.ID_TORCH);


        slots[0] = new ItemSlot(copperPickaxe, 1);
        slots[1] = new ItemSlot(copperAxe, 1);
        slots[2] = new ItemSlot(copperHammer, 1);

        slots[3] = new ItemSlot(grass, 99);
        slots[4] = new ItemSlot(woodPlank, 130);
        slots[5] = new ItemSlot(woodPlankBG, 99);
        slots[6] = new ItemSlot(torch, 54);
    }

    public int getNumSlots() { return NUM_SLOTS; }

    public ItemSlot getSlot(int index)
    {
        if (index < 0 || index >= NUM_SLOTS)
            return new ItemSlot(null, 0); //safely return empty slot

        return slots[index];
    }

    public void addItem(int itemID, int quantity) {
        if (itemID == 0 || quantity <= 0) return;
        Item itemDef = ItemRegistry.getByID(itemID);
        if (itemDef == null)
        {
            System.err.println("[SERVER] Error: failed to add item: " + itemID);
            return;
        }

        int remaining = quantity;
        int maxStack = itemDef.getMaxStackSize();

        //try to add to existing inventory item
        for (int i = 0; i < NUM_SLOTS && remaining > 0; i++)
        {
            ItemSlot slot = slots[i];
            if(!slot.isEmpty() && slot.getItem().getItemID() == itemID && slot.getQuantity() < maxStack)
            {
                int canAdd = maxStack - slot.getQuantity();
                int transfer = Math.min(remaining, canAdd);

                slot.setQuantity(slot.getQuantity() + transfer);
                remaining -= transfer;

                owner.getServer().sendMessageTo("INV_UPDATE," + owner.getId() + "," + i + "," + slot.serialize(), owner.getId());
            }
        }

        //add remaining item to new empty slot
        for (int i = 0; i < NUM_SLOTS && remaining > 0; i++)
        {
            ItemSlot slot = slots[i];
            if(slot.isEmpty())
            {
                int transfer = Math.min(remaining, maxStack);

                slots[i] = new ItemSlot(itemDef, transfer);
                remaining -= transfer;

                owner.getServer().sendMessageTo("INV_UPDATE," + owner.getId() + "," + i + "," + slots[i].serialize(), owner.getId());
            }
        }

        if(remaining > 0)
            System.out.println("[SERVER] Warning: Item " + itemDef.getName() + " (x" + remaining + ") did not fit in inventory for player " + owner.getId());
    }

    public void decreaseQuantity(int index, int amount)
    {
        ItemSlot slot = getSlot(index);
        if (slot.isEmpty()) return; //nothing to decrease

        int newQuantity = slot.getQuantity() - amount;
        if (newQuantity <= 0)
            slots[index] = new ItemSlot(null, 0);
        else slot.setQuantity(newQuantity);

        owner.getServer().sendMessageTo("INV_UPDATE," + owner.getId() + "," + index + "," + slots[index].serialize(), owner.getId());
    }

    public String serialize()
    {
        StringBuilder sb = new StringBuilder();
        sb.append("INV_SYNC,");
        for (int i = 0; i < NUM_SLOTS; i++)
            sb.append(",").append(slots[i].serialize());

        return sb.toString();
    }
}
