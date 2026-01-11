package com.swagaria.data.terrain;

public class Tile
{
    private int tileID;

    public Tile(int typeId)
    {
        this.tileID = typeId;
    }
    public int getTileID()
    {
        return tileID;
    }
    public void setTileID(int tileID)
    {
        this.tileID = tileID;
    }
    public TileDefinition getDefinition()
    {
        return TileDefinition.getDefinition(tileID);
    }
}
