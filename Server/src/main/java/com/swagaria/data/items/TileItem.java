package com.swagaria.data.items;

import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.data.terrain.TileLayer;
import com.swagaria.game.Player;
import com.swagaria.game.World;

public class TileItem extends Item
{
    private final int tileTypeID;

    public TileItem(int _itemID, String _name, int _maxStackSize, int _tileTypeID)
    {
        super(_itemID, _name, _maxStackSize);
        this.tileTypeID = _tileTypeID;
    }

    public int getTileTypeID() { return tileTypeID; }

    @Override
    public boolean use(Player caller, int posX, int posY, int slotIndex)
    {
        World world = caller.getServer().getWorld();
        TileDefinition def = TileDefinition.getDefinition(tileTypeID);
        int layer = def.layerToPlace;

        //check if block already exist x/y coordinate
        int currentIDinTargetPos = world.getTileAt(posX, posY, layer).getTypeId();
        if (currentIDinTargetPos != TileDefinition.ID_AIR)
            return false;

        //check for player overlap (stop tile from being placed inside player)
        if (layer == TileLayer.FOREGROUND)
        {
            if (def.hasCollision() &&
                    caller.overlapsTile(posX, posY))
                return false;
        }

        //ensure placement rules
        if(!caller.getServer().getClientHandler(caller.getId()).isPlacementValid(posX, posY, tileTypeID))
            return false;

        //add tile and remove from inventory
        world.setTileAt(posX, posY, layer, tileTypeID);
        caller.getInventory().decreaseQuantity(slotIndex, 1);

        return true;
    }
}
