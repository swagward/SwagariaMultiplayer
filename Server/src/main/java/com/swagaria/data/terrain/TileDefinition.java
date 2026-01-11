package com.swagaria.data.terrain;

import com.swagaria.data.components.*;
import com.swagaria.data.items.ToolType;

import java.util.HashMap;
import java.util.Map;

/**
 * acts as a static registry, defining tile properties for all tiles
 * using component pattern, this acts as property storage (idea taken from unity Swagaria)
 */
public class TileDefinition
{
    //replacing the old TileType enum
    public static final int ID_AIR = 0;
    public static final int ID_GRASS = 1;
    public static final int ID_DIRT = 2;
    public static final int ID_STONE = 3;
    public static final int ID_WOOD_LOG = 4;
    public static final int ID_TORCH = 5;
    public static final int ID_WOOD_PLANK = 6;
    public static final int ID_WOOD_PLANK_BG = 7;
    public static final int ID_STONE_BG = 8;
    public static final int ID_DIRT_BG = 9;
    public static final int ID_LEAVES = 10;
    public static final int ID_TALL_GRASS = 11;
    public static final int ID_FLOWER = 12;
    public static final int ID_SLATE = 13;
    public static final int ID_SLATE_BG = 14;
    public static final int ID_BEDROCK = 15;
    public static final int ID_WOOD_PLATFORM = 16;
    public static final int ID_GLASS = 17;

    public final int tileID;
    public final String name;
    public final int layerToPlace;

    private final Map<Class<? extends TileComponent>, TileComponent> components = new HashMap<>();

    private TileDefinition(int _tileID, String _name, int _layerToPlace, TileComponent... components)
    {
        tileID = _tileID;
        name = _name;
        layerToPlace = _layerToPlace;
        for (TileComponent component : components) //lowkey this looks like python now
        {
            this.components.put(component.getClass(), component);
        }
    }

    public <T extends TileComponent> T getComponent(Class<T> componentType) {
        return componentType.cast(components.get(componentType));
    }

    public boolean hasComponent(Class<? extends TileComponent> componentType) {
        return components.containsKey(componentType);
    }

    public boolean hasCollision() {
        CollisionComponent collision = getComponent(CollisionComponent.class);
        return collision != null && collision.blocksMovement;
    }

    private static final Map<Integer, TileDefinition> TILE_REGISTRY = new HashMap<>();

    static {
        TILE_REGISTRY.put(ID_AIR, new TileDefinition(ID_AIR, "Air", TileLayer.FOREGROUND,
                new LightSourceComponent(15)
        ));

        TILE_REGISTRY.put(ID_GRASS, new TileDefinition(ID_GRASS, "Grass Block", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(1, ID_DIRT),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));

        TILE_REGISTRY.put(ID_DIRT, new TileDefinition(ID_DIRT, "Dirt Block", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(2, ID_DIRT),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));

        TILE_REGISTRY.put(ID_STONE, new TileDefinition(ID_STONE, "Stone Block", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(4, ID_STONE),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));

        TILE_REGISTRY.put(ID_WOOD_LOG, new TileDefinition(ID_WOOD_LOG, "Wood Log", TileLayer.BACKGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(2, ID_WOOD_LOG),
                new RequiredToolComponent(ToolType.AXE)
        ));

        TILE_REGISTRY.put(ID_TORCH, new TileDefinition(ID_TORCH, "Torch", TileLayer.FOREGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(1, ID_TORCH),
                new LightSourceComponent(15),
                new RequiredToolComponent(ToolType.ANY)
        ));

        TILE_REGISTRY.put(ID_WOOD_PLANK, new TileDefinition(ID_WOOD_PLANK, "Wood Plank", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(3, ID_WOOD_PLANK),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));

        TILE_REGISTRY.put(ID_WOOD_PLANK_BG, new TileDefinition(ID_WOOD_PLANK_BG, "Wood Wall", TileLayer.BACKGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(2, ID_WOOD_PLANK_BG),
                new RequiredToolComponent(ToolType.HAMMER)
        ));

        TILE_REGISTRY.put(ID_STONE_BG, new TileDefinition(ID_STONE_BG, "Stone Wall", TileLayer.BACKGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(4, ID_STONE_BG),
                new RequiredToolComponent(ToolType.HAMMER)
        ));

        TILE_REGISTRY.put(ID_DIRT_BG, new TileDefinition(ID_DIRT_BG, "Dirt Wall", TileLayer.BACKGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(1, ID_DIRT_BG),
                new RequiredToolComponent(ToolType.HAMMER)
        ));

        TILE_REGISTRY.put(ID_LEAVES, new TileDefinition(ID_LEAVES, "Leaves", TileLayer.FOREGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(1, ID_LEAVES),
                new RequiredToolComponent(ToolType.ANY)
        ));

        TILE_REGISTRY.put(ID_TALL_GRASS, new TileDefinition(ID_TALL_GRASS, "Tall Grass", TileLayer.FOREGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(1, ID_TALL_GRASS),
                new RequiredToolComponent(ToolType.ANY)
        ));

        TILE_REGISTRY.put(ID_FLOWER, new TileDefinition(ID_FLOWER, "Flowers", TileLayer.FOREGROUND,
                new CollisionComponent(false),
                new DurabilityComponent(1, ID_FLOWER),
                new RequiredToolComponent(ToolType.ANY)
        ));

        TILE_REGISTRY.put(ID_SLATE, new TileDefinition(ID_SLATE, "Slate Block", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(6, ID_SLATE),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));

        TILE_REGISTRY.put(ID_SLATE_BG, new TileDefinition(ID_SLATE_BG, "Slate Wall", TileLayer.BACKGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(6, ID_SLATE_BG),
                new RequiredToolComponent(ToolType.HAMMER)
        ));

        TILE_REGISTRY.put(ID_BEDROCK, new TileDefinition(ID_BEDROCK, "Bedrock", TileLayer.BACKGROUND,
                new CollisionComponent(true)
        ));

        TILE_REGISTRY.put(ID_WOOD_PLATFORM, new TileDefinition(ID_WOOD_PLATFORM, "Wood Platform", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(2, ID_WOOD_PLATFORM),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));

        TILE_REGISTRY.put(ID_GLASS, new TileDefinition(ID_GLASS, "Glass", TileLayer.FOREGROUND,
                new CollisionComponent(true),
                new DurabilityComponent(1, ID_GLASS),
                new RequiredToolComponent(ToolType.PICKAXE)
        ));
    }

    //gets static tile definition based on ID
    public static TileDefinition getDefinition(int id)
    {
        return TILE_REGISTRY.getOrDefault(id, TILE_REGISTRY.get(ID_AIR));
    }
}
