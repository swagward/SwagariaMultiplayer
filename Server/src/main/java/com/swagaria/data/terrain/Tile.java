package com.swagaria.data.terrain;

public class Tile
{
    private int typeId;
    private float damage;

    public Tile(int typeId)
    {
        this.typeId = typeId;
        this.damage = 0.0f;
    }

    public int getTypeId()
    {
        return typeId;
    }

    public void setTypeId(int typeId)
    {
        this.typeId = typeId;
        this.damage = 0.0f;
    }

    public float getDamage() { return damage; }
    public void setDamage(float damage) { this.damage = damage; }

    public TileDefinition getDefinition()
    {
        return TileDefinition.getDefinition(typeId);
    }
}
