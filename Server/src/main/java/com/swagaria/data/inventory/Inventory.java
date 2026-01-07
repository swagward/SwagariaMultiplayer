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

        //player's starting items
        slots[0] = new ItemSlot(ItemRegistry.getByID(100), 1); //pickaxe
        slots[1] = new ItemSlot(ItemRegistry.getByID(101), 1); //axe
        slots[2] = new ItemSlot(ItemRegistry.getByID(102), 1); //hammer

        slots[3] = new ItemSlot(ItemRegistry.getByID(TileDefinition.ID_GRASS), 999);
        slots[4] = new ItemSlot(ItemRegistry.getByID(TileDefinition.ID_WOOD_PLANK), 999);
        slots[5] = new ItemSlot(ItemRegistry.getByID(TileDefinition.ID_WOOD_PLANK_BG), 999);
        slots[6] = new ItemSlot(ItemRegistry.getByID(TileDefinition.ID_TORCH), 999);
        slots[7] = new ItemSlot(ItemRegistry.getByID(TileDefinition.ID_LEAVES), 999);
    }

    public ItemSlot getSlot(int index)
    {
        if (index < 0 || index >= NUM_SLOTS)
            return new ItemSlot(null, 0); //safely return empty slot

        return slots[index];
    }

    public void setSlot(int index, int itemID, int quantity)
    {
        if(index < 0 || index >= NUM_SLOTS) return;

        if(itemID <= 0 || quantity <= 0)
            slots[index] = new ItemSlot(null, 0);
        else
        {
            Item itemDef = ItemRegistry.getByID(itemID);
            if(itemDef != null)
                slots[index] = new ItemSlot(itemDef, quantity);
            else
            {
                slots[index] = new ItemSlot(null, 0);
                System.err.println("[SERVER] Player " + owner.getId() + " tried to set slot " + index + " to unknown ID: " + itemID);
            }
        }

        //no need to tell the client to update the inventory since
        //they're the one who initiated the item move
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
