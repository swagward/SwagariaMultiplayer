package com.swagaria.network;

import com.swagaria.data.TerrainConfig;
import com.swagaria.data.TileType;
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

        String[] parts = line.split(",", 4);
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
        if (parts.length < 4) return;

        try
        {
            int worldX = Integer.parseInt(parts[1].trim());
            int clientBottomUpY = Integer.parseInt(parts[2].trim());
            int tileTypeOrdinal = Integer.parseInt(parts[3].trim());

            Player p = server.getPlayer(clientId);
            if (p == null) return;

            //tile edit is within reach
            float playerX = p.getX() + (Player.WIDTH / 2f);
            float playerY = p.getY() + (Player.HEIGHT / 2f);

            int worldHeightInTiles = TerrainConfig.WORLD_HEIGHT;
            int serverTopDownY = worldHeightInTiles - 1 - clientBottomUpY;

            //get the centre of the tile
            float tileX = worldX + 0.5f;
            float tileY = serverTopDownY + 0.5f;

            float dx = playerX - tileX;
            float dy = playerY - tileY;
            float distanceSq = (dx * dx) + (dy * dy);

            if (distanceSq > Player.MAX_REACH_DISTANCE_SQ)
                return;

            //placing a block
            if (tileTypeOrdinal != TileType.AIR.ordinal())
            {
                if (server.getWorld().isSolidTile(worldX, serverTopDownY))
                    return;

                //AABB intersection test to see if player tries placing inside themselves
                boolean overlaps = p.getX() < worldX + 1 &&
                        p.getX() + Player.WIDTH > worldX &&
                        p.getY() < serverTopDownY + 1 &&
                        p.getY() + Player.HEIGHT > serverTopDownY;

                if (overlaps)
                    return;

                //TODO: check if player has this item in inventory
            }

            //breaking a block
            if (tileTypeOrdinal != TileType.AIR.ordinal())
            {
                if (server.getWorld().isSolidTile(worldX, serverTopDownY))
                    return;

                //TODO: check if block is unbreakable (i.e: bedrock)
                //TODO: check if player has the correct tool
            }

            boolean success = server.getWorld().setTileAt(worldX, serverTopDownY, tileTypeOrdinal);

            if (success)
            {
                String updateMsg = "UPDATE_TILE," + worldX + "," + serverTopDownY + "," + tileTypeOrdinal;
                server.broadcast(updateMsg);
            }

        }
        catch (NumberFormatException e)
        {
            System.err.println("[Server] Bad SET_TILE from player #" + clientId);
        }
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
