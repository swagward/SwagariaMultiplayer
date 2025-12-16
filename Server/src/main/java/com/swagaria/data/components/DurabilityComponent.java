package com.swagaria.data.components;

import com.swagaria.data.terrain.TileComponent;

public class DurabilityComponent implements TileComponent
{
    public final int hitsRequired; //how many hits to break block (not used atm because im lazy)
    public final int itemDropID; //what is dropped on break

    public DurabilityComponent(int _hitsRequired, int _itemDropID)
    {
        hitsRequired = _hitsRequired;
        itemDropID = _itemDropID;
    }
}
