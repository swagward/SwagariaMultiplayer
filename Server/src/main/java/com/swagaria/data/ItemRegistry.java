package com.swagaria.data;

import com.swagaria.data.items.Item;
import com.swagaria.data.items.TileItem;
import com.swagaria.data.terrain.TileDefinition;

import java.util.HashMap;
import java.util.Map;

/**
 * central registry for all definable items, mapping item IDs to item objs
 * can be used to instantiate items for inventories, block drops, loot pools, etc.
 */
public class ItemRegistry
{
    private static final Map<Integer, Item> ITEM_REGISTRY = new HashMap<>(); //id, item

    static {
        //tile items (1-99)
        ITEM_REGISTRY.put(TileDefinition.ID_DIRT, new TileItem(TileDefinition.ID_DIRT, "Grass Block", 999, TileDefinition.ID_DIRT));
        ITEM_REGISTRY.put(TileDefinition.ID_GRASS, new TileItem(TileDefinition.ID_GRASS, "Dirt Block", 999, TileDefinition.ID_GRASS));
        ITEM_REGISTRY.put(TileDefinition.ID_STONE, new TileItem(TileDefinition.ID_STONE, "Stone Block", 999, TileDefinition.ID_STONE));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_LOG, new TileItem(TileDefinition.ID_WOOD_LOG, "Wood Log", 999, TileDefinition.ID_WOOD_LOG));
        ITEM_REGISTRY.put(TileDefinition.ID_TORCH, new TileItem(TileDefinition.ID_TORCH, "Torch", 999, TileDefinition.ID_TORCH));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_PLANK, new TileItem(TileDefinition.ID_WOOD_PLANK, "Wood Plank", 999, TileDefinition.ID_WOOD_PLANK));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_PLANK_BG, new TileItem(TileDefinition.ID_WOOD_PLANK_BG, "Wood Wall", 999, TileDefinition.ID_WOOD_PLANK_BG));
        ITEM_REGISTRY.put(TileDefinition.ID_STONE_BG, new TileItem(TileDefinition.ID_STONE_BG, "Stone Wall", 999, TileDefinition.ID_STONE_BG));

        //tool items (100-199)
        //ITEM_REGISTRY.put(100, new ToolItem(100, "Copper Pickaxe", 1, 10, ToolType.PICKAXE));

        Item.setRegistry(new ItemRegistry());
    }

    public static Item getByID(int itemID) { return ITEM_REGISTRY.get(itemID); }

    public static String getDefinitionSync()
    {
        StringBuilder sb = new StringBuilder();
        sb.append("ITEM_DEF_SYNC");

        for (Item item : ITEM_REGISTRY.values())
        {
            sb.append("|").append(item.getItemID()).append(":").append(item.getName()).append(":").append(item.getMaxStackSize());

            //add item-specific properties
            if(item instanceof TileItem tileItem)
                sb.append(":T:").append(tileItem.getTileTypeID());
            else
                sb.append(":O:");
        }

        return sb.toString();
    }
}
