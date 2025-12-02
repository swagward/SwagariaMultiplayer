package com.swagaria.data;

public class Tile
{
    private int typeId;
    private float damage; // New: Current damage accumulated (0.0 to MaxDurability)

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
        this.damage = 0.0f; // Reset damage when type changes
    }

    public float getDamage() { return damage; }
    public void setDamage(float damage) { this.damage = damage; }

    /**
     * Helper to get the static definition for this instance.
     */
    public TileDefinition getDefinition()
    {
        return TileDefinition.getDefinition(typeId);
    }
}
