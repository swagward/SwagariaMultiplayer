package com.swagaria.data.items;

import com.swagaria.data.components.RequiredToolComponent;
import com.swagaria.data.terrain.Tile;
import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.game.Player;
import com.swagaria.game.World;

public class ToolItem extends Item
{
    public final float damage;
    public final ToolType toolType;
    public final int layerToBreak;

    public ToolItem(int _itemID, String _name, int _maxStackSize, float _damage, ToolType _toolType, int _layerToBreak)
    {
        super(_itemID, _name, _maxStackSize);
        damage = _damage;
        toolType = _toolType;
        layerToBreak = _layerToBreak;
    }

    @Override
    public boolean use(Player caller, int posX, int posY, int slotIndex)
    {
        World world = caller.getServer().getWorld();
        Tile tile = world.getTileAt(posX, posY, layerToBreak);
        if(tile == null || tile.getTileID() == TileDefinition.ID_AIR)
            return false; //nothing to break

        TileDefinition def = TileDefinition.getDefinition(tile.getTileID());
        RequiredToolComponent requiredTool = def.getComponent(RequiredToolComponent.class);
        boolean isCorrectTool = false;

        if(requiredTool != null)
        {
            if(requiredTool.requiredTool == this.toolType || requiredTool.requiredTool == ToolType.ANY)
                isCorrectTool = true;
        }

        if (!isCorrectTool)
            return false; //cannot continue

        //remove tile and add to inventory
        world.setTileAt(posX, posY, layerToBreak, TileDefinition.ID_AIR);
        int itemIDtoAdd = def.tileID;
        if(itemIDtoAdd != TileDefinition.ID_AIR)
            caller.getInventory().addItem(itemIDtoAdd, 1);

        return true;

    }
}
