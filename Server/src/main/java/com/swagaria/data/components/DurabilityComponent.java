package com.swagaria.data.components;

import com.swagaria.data.terrain.TileComponent;

public class DurabilityComponent implements TileComponent
{
    public final float maxDurability; //how many hits to break block
    public final int itemDropID; //what is dropped on break

    public DurabilityComponent(float _maxDurability, int _itemDropID)
    {
        maxDurability = _maxDurability;
        itemDropID = _itemDropID;
    }
}
