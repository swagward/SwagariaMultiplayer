package com.swagaria.network;

import com.swagaria.data.ItemRegistry;
import com.swagaria.data.inventory.ItemSlot;
import com.swagaria.data.items.Item;
import com.swagaria.data.items.TileItem;
import com.swagaria.data.items.ToolItem;
import com.swagaria.data.terrain.TerrainConfig;
import com.swagaria.data.terrain.TileDefinition;
import com.swagaria.data.terrain.TileLayer;
import com.swagaria.game.Player;
import com.swagaria.game.Chunk;
import com.swagaria.game.World;

import java.io.*;
import java.net.Socket;

public class ClientHandler implements Runnable
{
    private final Socket socket;
    private final int clientId;
    private final Server server;
    private PrintWriter out;
    private BufferedReader in;
    private volatile boolean running = true;

    public ClientHandler(Socket socket, int clientId, Server server)
    {
        this.socket = socket;
        this.clientId = clientId;
        this.server = server;
    }

    public int getClientId() { return clientId; }

    @Override
    public void run()
    {
        try
        {
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            out.println("ASSIGN_ID," + clientId);
            out.flush();

            out.println(ItemRegistry.getDefinitionSync());
            out.flush();

            //tell others about new client join
            for (Player p : server.getAllPlayers())
            {
                if (p.getId() != clientId)
                {
                    out.println("PLAYER_JOIN," + p.getId() + "," + p.getX() + "," + p.getY());
                    out.flush();
                }
            }

            //spawn client's player
            Player me = server.getPlayer(clientId);
            if (me != null)
            {
                out.println(me.getInventory().serialize());
                out.flush();

                out.println("SPAWN," + clientId + "," + me.getX() + "," + me.getY());
                out.flush();
            }

            try
            {
                System.out.println("[Server] Sending chunks to player #" + clientId + " (count=" + server.getWorld().getAllChunks().size() + ")");
                for (Chunk chunk : server.getWorld().getAllChunks())
                {
                    String msg = chunk.serialize();
                    out.println(msg);
                    out.flush();
                }
                System.out.println("[Server] Sent " + server.getWorld().getAllChunks().size() + " chunks to player #" + clientId);
            } catch (Exception e)
            {
                //client probably disconnected while sending messages, log message and stop
                System.err.println("[Server] Exception while sending chunks to player #" + clientId + ": " + e.getMessage());
                running = false;
            }

            if (me != null)
                server.broadcastExcept("PLAYER_JOIN," + clientId + "," + me.getX() + "," + me.getY(), clientId);

            //main receive loop here VVV
            String line;
            while (running && (line = in.readLine()) != null)
                handleLine(line);

        } catch (IOException e)
        {
            System.err.println("[Server] Connection error with player #" + clientId + ": " + e.getMessage());
        }
        finally
        {
            cleanup();
            server.removeClient(clientId, this);
        }
    }

    private void handleLine(String line)
    {
        if (line == null || line.isEmpty())
            return;

        //USE_ITEM,<slotIndex>,<posX>,<posY>
        String[] parts = line.split(",", 5);
        if (parts.length == 0)
            return;

        String cmd = parts[0].trim(); //trim just to be safe

        switch (cmd)
        {
            case "INPUT" -> handleInput(parts);
            case "USE_ITEM" -> handleUseItem(parts);
            default -> System.out.println("[Server] Unknown command: " + line);
        }
    }

    private void handleUseItem(String[] parts)
    {
        if (parts.length < 4) return;

        try
        {
            int slotIndex = Integer.parseInt(parts[1].trim());
            int worldX = Integer.parseInt(parts[2].trim());
            int worldY = Integer.parseInt(parts[3].trim());

            Player p = server.getPlayer(clientId);
            if (p == null) return;

            ItemSlot slot = p.getInventory().getSlot(slotIndex);
            Item item = slot.getItem();
            if (item == null || slot.getQuantity() <= 0) //empty slot
                return;

            int worldHeight = TerrainConfig.WORLD_HEIGHT;
            int worldHeightFlipped = worldHeight - 1 - worldY;
            if (p.getServer().getWorld().calculateDistanceSq(p.getX(), p.getY(), worldX,worldHeightFlipped) > Player.MAX_REACH_DISTANCE_SQ)
                return; //reach check

            boolean worldModified = false;
            if(item != null)
                worldModified = item.use(p, worldX, worldHeightFlipped, slotIndex);

            if(worldModified)
            {
                if(item instanceof TileItem tileItem)
                {
                    TileDefinition def = TileDefinition.getDefinition(tileItem.getTileTypeID());
                    String updateMsg = "UPDATE_TILE," + worldX + "," + worldHeightFlipped + "," + tileItem.getTileTypeID() + "," + def.layerToPlace;
                    server.broadcast(updateMsg);
                }
                else if(item instanceof ToolItem toolItem)
                {
                    String updateMsg = "UPDATE_TILE," + worldX + "," + worldHeightFlipped + "," + TileDefinition.ID_AIR + "," + toolItem.layerToBreak;
                    server.broadcast(updateMsg);
                }
            }
        }
        catch (NumberFormatException e)
        {
            System.err.println("[Server] Bad USE_ITEM command from player #" + clientId + ": " + e.getMessage());
        }
    }

    public boolean isPlacementValid(int x, int y, int tileToPlaceId)
    {
        World world = server.getWorld();

        //torch rule
        if (tileToPlaceId == TileDefinition.ID_TORCH)
        {
            //fuck my chud life
            //check if foreground block is below
            int blockBelow = world.getTileAt(x, y + 1, TileLayer.FOREGROUND).getTypeId();
            if (blockBelow != TileDefinition.ID_AIR) return true;

            //check if background block is behind
            int wallBehind = world.getTileAt(x, y, TileLayer.BACKGROUND).getTypeId();
            if (wallBehind != TileDefinition.ID_AIR) return true;

            return false;
        }

        int[][] neighbours = {
                {x - 1, y}, {x + 1, y}, {x, y - 1}, {x, y + 1}
        };

        for (int[] pos : neighbours)
        {
            int fgNeighbourID = world.getTileAt(pos[0], pos[1], TileLayer.FOREGROUND).getTypeId();
            int bgNeighbourID = world.getTileAt(pos[0], pos[1], TileLayer.BACKGROUND).getTypeId();

            if(fgNeighbourID != TileDefinition.ID_AIR || bgNeighbourID != TileDefinition.ID_AIR)
                return true;
        }

        return false;
    }

    private void handleInput(String[] parts)
    {
        if (parts.length < 3) return;
        int id;
        try
        {
            id = Integer.parseInt(parts[1]);
        }
        catch (NumberFormatException ex)
        {
            return;
        }
        String action = parts[2];

        Player p = server.getPlayer(id);
        if (p == null) return;

        boolean pressed = action.endsWith("_DOWN");
        p.setInput(action, pressed);
    }

    public void sendMessage(String msg)
    {
        if (out != null)
        {
            out.println(msg);
            out.flush();
        }
    }

    private void cleanup()
    {
        running = false;
        try
        {
            if (in != null) in.close();
        }
        catch (IOException ignored) {}

        if (out != null)
            out.close();

        try
        {
            if (socket != null)
                socket.close();
        }
        catch (IOException ignored) {}
    }
}