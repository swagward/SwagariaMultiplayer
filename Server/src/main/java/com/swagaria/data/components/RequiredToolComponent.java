package com.swagaria.data.components;

import com.swagaria.data.items.ToolType;
import com.swagaria.data.terrain.TileComponent;

public class RequiredToolComponent implements TileComponent
{
    public final ToolType requiredTool;

    public RequiredToolComponent(ToolType _requiredTool)
    {
        requiredTool = _requiredTool;
    }
}
