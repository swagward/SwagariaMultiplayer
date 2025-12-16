package com.swagaria.data.terrain;

public class Tile
{
    private int typeId;

    public Tile(int typeId)
    {
        this.typeId = typeId;
    }
    public int getTypeId()
    {
        return typeId;
    }
    public void setTypeId(int typeId)
    {
        this.typeId = typeId;
    }
    public TileDefinition getDefinition()
    {
        return TileDefinition.getDefinition(typeId);
    }
}
