package com.swagaria.data.components;

import com.swagaria.data.terrain.TileComponent;

public class LightSourceComponent implements TileComponent
{
    public final int lightLevel; //light emitted by tile (0-15)

    public LightSourceComponent(int _lightLevel)
    {
        lightLevel = _lightLevel;
    }
}
