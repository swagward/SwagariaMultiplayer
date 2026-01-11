package com.swagaria.data;

import com.swagaria.data.items.Item;
import com.swagaria.data.items.TileItem;
import com.swagaria.data.items.ToolItem;
import com.swagaria.data.items.ToolType;
import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.data.terrain.TileLayer;

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
        ITEM_REGISTRY.put(TileDefinition.ID_GRASS, new TileItem(TileDefinition.ID_GRASS, "Grass", 999, TileDefinition.ID_GRASS));
        ITEM_REGISTRY.put(TileDefinition.ID_DIRT, new TileItem(TileDefinition.ID_DIRT, "Dirt", 999, TileDefinition.ID_DIRT));
        ITEM_REGISTRY.put(TileDefinition.ID_STONE, new TileItem(TileDefinition.ID_STONE, "Stone", 999, TileDefinition.ID_STONE));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_LOG, new TileItem(TileDefinition.ID_WOOD_LOG, "Wood Log", 999, TileDefinition.ID_WOOD_LOG));
        ITEM_REGISTRY.put(TileDefinition.ID_TORCH, new TileItem(TileDefinition.ID_TORCH, "Torch", 999, TileDefinition.ID_TORCH));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_PLANK, new TileItem(TileDefinition.ID_WOOD_PLANK, "Wood Plank", 999, TileDefinition.ID_WOOD_PLANK));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_PLANK_BG, new TileItem(TileDefinition.ID_WOOD_PLANK_BG, "Wood Plank BG", 999, TileDefinition.ID_WOOD_PLANK_BG));
        ITEM_REGISTRY.put(TileDefinition.ID_STONE_BG, new TileItem(TileDefinition.ID_STONE_BG, "Stone BG", 999, TileDefinition.ID_STONE_BG));
        ITEM_REGISTRY.put(TileDefinition.ID_DIRT_BG, new TileItem(TileDefinition.ID_DIRT_BG, "Dirt BG", 999, TileDefinition.ID_DIRT_BG));
        ITEM_REGISTRY.put(TileDefinition.ID_LEAVES, new TileItem(TileDefinition.ID_LEAVES, "Leaves", 999, TileDefinition.ID_LEAVES));
        ITEM_REGISTRY.put(TileDefinition.ID_TALL_GRASS, new TileItem(TileDefinition.ID_TALL_GRASS, "Tall Grass", 999, TileDefinition.ID_TALL_GRASS));
        ITEM_REGISTRY.put(TileDefinition.ID_FLOWER, new TileItem(TileDefinition.ID_FLOWER, "Flowers", 999, TileDefinition.ID_FLOWER));
        ITEM_REGISTRY.put(TileDefinition.ID_SLATE, new TileItem(TileDefinition.ID_SLATE, "Slate", 999, TileDefinition.ID_SLATE));
        ITEM_REGISTRY.put(TileDefinition.ID_SLATE_BG, new TileItem(TileDefinition.ID_SLATE_BG, "Slate BG", 999, TileDefinition.ID_SLATE_BG));
        ITEM_REGISTRY.put(TileDefinition.ID_WOOD_PLATFORM, new TileItem(TileDefinition.ID_WOOD_PLATFORM, "Wood Platform", 999, TileDefinition.ID_WOOD_PLATFORM));
        ITEM_REGISTRY.put(TileDefinition.ID_GLASS, new TileItem(TileDefinition.ID_GLASS, "Glass", 999, TileDefinition.ID_GLASS));

        //tool items (100-199)
        ITEM_REGISTRY.put(100, new ToolItem(100, "Copper Pickaxe", 1, 3, ToolType.PICKAXE, TileLayer.FOREGROUND));
        ITEM_REGISTRY.put(101, new ToolItem(101, "Copper Axe", 1, 2, ToolType.AXE, TileLayer.BACKGROUND));
        ITEM_REGISTRY.put(102, new ToolItem(102, "Copper Hammer", 1, 1, ToolType.HAMMER, TileLayer.BACKGROUND));
    }

    public static Item getByID(int itemID) { return ITEM_REGISTRY.get(itemID); }

    public static String getDefinitionSync()
    {
        StringBuilder sb = new StringBuilder();
        sb.append("ITEM_DEF_SYNC,");

        for (Item item : ITEM_REGISTRY.values())
        {
            sb.append("|").append(item.getItemID()).append(":").append(item.getName()).append(":").append(item.getMaxStackSize());

            //add item-specific properties
            if(item instanceof TileItem tileItem)
                sb.append(":T:").append(tileItem.getTileID());
            else if(item instanceof ToolItem toolItem)
                sb.append(":R:").append(toolItem.toolType.name()).append(":").append(toolItem.damage);
            else
                sb.append(":O:"); // O for Other
        }

        return sb.toString();
    }
}
