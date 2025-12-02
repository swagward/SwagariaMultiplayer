package com.swagaria.data;

import com.swagaria.data.components.*;
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

    public final int typeID;
    public final String name;

    private final Map<Class<? extends TileComponent>, TileComponent> components = new HashMap<>();

    private TileDefinition(int _typeID, String _name, TileComponent... components)
    {
        typeID = _typeID;
        name = _name;
        for (TileComponent component : components) //lowkey this looks like python now
        {
            this.components.put(component.getClass(), component);
        }
    }

    //gets component instance
    public <T extends TileComponent> T getComponent(Class<T> componentType) {
        return componentType.cast(components.get(componentType));
    }

    //checks if tile has specific component
    public boolean hasComponent(Class<? extends TileComponent> componentType) {
        return components.containsKey(componentType);
    }

    private static final Map<Integer, TileDefinition> TILE_REGISTRY = new HashMap<>();

    static {
        // --- Tile Registration ---

        // 0. AIR: No components, no collision.
        TILE_REGISTRY.put(ID_AIR, new TileDefinition(ID_AIR, "Air",
                new LightSourceComponent(15)
        ));

        // 1. GRASS: Solid, Breakable, Drops Grass
        TILE_REGISTRY.put(ID_GRASS, new TileDefinition(ID_GRASS, "Grass Block",
                new CollisionComponent(true),
                new DurabilityComponent(5.0f, ID_GRASS)
        ));

        // 2. DIRT: Solid, Breakable, Drops Dirt
        TILE_REGISTRY.put(ID_DIRT, new TileDefinition(ID_DIRT, "Dirt Block",
                new CollisionComponent(true),
                new DurabilityComponent(4.0f, ID_DIRT)
        ));

        // 3. STONE: Solid, Tough Breakable, Drops Stone
        TILE_REGISTRY.put(ID_STONE, new TileDefinition(ID_STONE, "Stone Block",
                new CollisionComponent(true),
                new DurabilityComponent(10.0f, ID_STONE)
        ));

        // 4. WOOD_LOG: Solid, Breakable, Drops Wood Log
        TILE_REGISTRY.put(ID_WOOD_LOG, new TileDefinition(ID_WOOD_LOG, "Wood Log",
                new CollisionComponent(true),
                new DurabilityComponent(6.0f, ID_WOOD_LOG)
        ));

        // 5. TORCH: Not Solid, Breakable, Light Source
        TILE_REGISTRY.put(ID_TORCH, new TileDefinition(ID_TORCH, "Torch",
                new CollisionComponent(false),
                new DurabilityComponent(1.0f, ID_TORCH),
                new LightSourceComponent(15)
        ));

        // 6. WOOD_PLANK: Solid, Breakable, Drops Wood Plank
        TILE_REGISTRY.put(ID_WOOD_PLANK, new TileDefinition(ID_WOOD_PLANK, "Wood Plank",
                new CollisionComponent(true),
                new DurabilityComponent(7.0f, ID_WOOD_PLANK)
        ));

        // 7. WOOD_PLANK_BG: Background (Not solid), Breakable
        TILE_REGISTRY.put(ID_WOOD_PLANK_BG, new TileDefinition(ID_WOOD_PLANK_BG, "Wood Wall",
                new CollisionComponent(false),
                new DurabilityComponent(7.0f, ID_WOOD_PLANK_BG)
        ));

        // 8. STONE_BG: Background (Not solid), Breakable
        TILE_REGISTRY.put(ID_STONE_BG, new TileDefinition(ID_STONE_BG, "Stone Wall",
                new CollisionComponent(false),
                new DurabilityComponent(8.0f, ID_STONE_BG)
        ));
    }

    //gets static tile definition based on ID
    public static TileDefinition getDefinition(int id) {
        return TILE_REGISTRY.getOrDefault(id, TILE_REGISTRY.get(ID_AIR));
    }
}
