package com.swagaria.network;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.TileDefinition;
import com.swagaria.data.TileLayer;
import com.swagaria.game.Player;
import com.swagaria.game.Chunk;

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

        String[] parts = line.split(",", 5);
        if (parts.length == 0)
            return;

        String cmd = parts[0].trim(); //trim just to be safe

        switch (cmd)
        {
            case "INPUT" -> handleInput(parts);
            case "SET_TILE" -> handleSetTile(parts);
            default -> System.out.println("[Server] Unknown command: " + line);
        }
    }

    private void handleSetTile(String[] parts)
    {
        if (parts.length < 5) return;

        try
        {
            int worldX = Integer.parseInt(parts[1].trim());
            int clientBottomUpY = Integer.parseInt(parts[2].trim());
            int tileTypeId = Integer.parseInt(parts[3].trim());
            int layerIndex = Integer.parseInt(parts[4].trim());

            if (layerIndex < 0 || layerIndex >= TileLayer.NUM_LAYERS)
            {
                System.err.println("[Server] Invalid layer index: " + layerIndex + " from player #" + clientId);
                return;
            }

            Player p = server.getPlayer(clientId);
            if (p == null) return;

            float playerX = p.getX() + (Player.WIDTH / 2f);
            float playerY = p.getY() + (Player.HEIGHT / 2f);

            int worldHeightInTiles = TerrainConfig.WORLD_HEIGHT;
            int serverTopDownY = worldHeightInTiles - 1 - clientBottomUpY;

            float tileX = worldX + 0.5f;
            float tileY = serverTopDownY + 0.5f;

            float dx = playerX - tileX;
            float dy = playerY - tileY;
            float distanceSq = (dx * dx) + (dy * dy);

            if (distanceSq > Player.MAX_REACH_DISTANCE_SQ)
                return;

            int currentTileId = server.getWorld().getTileAt(worldX, serverTopDownY, layerIndex).getTypeId();

            //placing a tile
            if (tileTypeId != TileDefinition.ID_AIR)
            {
                //stop tiles from overwriting existing tiles (in the same layer)
                if (currentTileId != TileDefinition.ID_AIR)
                    return;

                //stop placing inside player (only applies to foreground)
                if (layerIndex == TileLayer.FOREGROUND)
                {
                    boolean overlaps = p.getX() < worldX + 1 &&
                            p.getX() + Player.WIDTH > worldX &&
                            p.getY() < serverTopDownY + 1 &&
                            p.getY() + Player.HEIGHT > serverTopDownY;

                    if (overlaps)
                        return;
                }

                //check placement validation (only applies to foreground for now, like torches)
                if (layerIndex == TileLayer.FOREGROUND)
                {
                    if (!isPlacementValid(worldX, serverTopDownY, tileTypeId))
                        return;
                }

                //TODO: check if player has this item in inventory
            }

            //breaking (placing air)
            else if (tileTypeId == TileDefinition.ID_AIR)
            {
                // skip if already air (placing air on air)
                if (currentTileId == TileDefinition.ID_AIR)
                    return;

                //TODO: add damage accumulation / check tool
            }

            //try placing, if true then all good
            boolean success = server.getWorld().setTileAt(worldX, serverTopDownY, layerIndex, tileTypeId);

            if (success)
            {
                String updateMsg = "UPDATE_TILE," + worldX + "," + serverTopDownY + "," + tileTypeId + "," + layerIndex;
                server.broadcast(updateMsg);
            }

        }
        catch (NumberFormatException e)
        {
            System.err.println("[Server] Bad SET_TILE from player #" + clientId + ": " + e.getMessage());
        }
    }

    private boolean isPlacementValid(int x, int y, int tileToPlaceId)
    {
        //torch rule
        final int ID_TORCH = 5;
        if (tileToPlaceId == ID_TORCH)
        {
            //can place if a foreground block is below it
            int blockBelowId = server.getWorld().getTileAt(x, y + 1, TileLayer.FOREGROUND).getDefinition().typeID;
            if (blockBelowId != TileDefinition.ID_AIR)
                return true;

            //can place if theres a background block behind it
            int backgroundWallId = server.getWorld().getTileAt(x, y, TileLayer.BACKGROUND).getDefinition().typeID;
            if (backgroundWallId != TileDefinition.ID_AIR)
                return true;

            return false;
        }

        //placing next to other tiles
        int[][] neighbors = {
                {x - 1, y}, //left
                {x + 1, y}, //right
                {x, y - 1}, //up
                {x, y + 1} //down
        };

        for (int[] pos : neighbors)
        {
            //check if foreground is next to x/y
            int fgNeighborId = server.getWorld().getTileAt(pos[0], pos[1], TileLayer.FOREGROUND).getDefinition().typeID;
            if (fgNeighborId != TileDefinition.ID_AIR)
                return true;

            //check if background block is behind x/y
            int bgNeighborId = server.getWorld().getTileAt(pos[0], pos[1], TileLayer.BACKGROUND).getDefinition().typeID;
            if (bgNeighborId != TileDefinition.ID_AIR)
                return true;
        }

        //if all nearby tiles are air
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