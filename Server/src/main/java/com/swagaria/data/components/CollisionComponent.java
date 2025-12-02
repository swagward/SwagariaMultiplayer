package com.swagaria.data.components;

import com.swagaria.data.TileComponent;

public class CollisionComponent implements TileComponent
{
    public final boolean blocksMovement; //refactored isSolid boolean from before

    public CollisionComponent(boolean _blocksMovement)
    {
        blocksMovement = _blocksMovement;
    }
}
