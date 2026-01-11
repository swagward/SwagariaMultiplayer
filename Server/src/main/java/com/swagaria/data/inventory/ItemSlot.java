package com.swagaria.data.inventory;

import com.swagaria.data.items.Item;

public class ItemSlot
{
    private final Item item;
    private int quantity;

    public ItemSlot(Item _item, int _quantity)
    {
        this.item = _item;
        this.quantity = _quantity;
    }

    public Item getItem() { return item; }
    public int getQuantity() { return quantity; }

    public void setQuantity(int newQuantity) { quantity = Math.max(0, newQuantity); } //stop quantity from being negative value

    public boolean isEmpty() { return item == null || quantity <= 0; }

    public String serialize()
    {
        if(isEmpty())
            return "0,0";

        return item.getItemID() + "," + quantity;
    }
}
