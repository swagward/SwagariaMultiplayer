package com.swagaria.network;

import com.swagaria.game.Physics;
import com.swagaria.game.Player;
import com.swagaria.game.World;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.*;

public class Server
{
    private final int port;
    private final int playerLimit;
    private final ExecutorService pool = Executors.newCachedThreadPool();
    private final AtomicInteger idCounter = new AtomicInteger(1);
    private final Map<Integer, Player> players = new ConcurrentHashMap<>();
    private final List<ClientHandler> handlers = new CopyOnWriteArrayList<>();
    private final World world = new World();
    private volatile boolean running = false;
    private ServerSocket serverSocket;
    private ScheduledExecutorService tickExecutor;
    private long lastUpdateTime;

    public Server(int port, int playerLimit)
    {
        this.port = port;
        this.playerLimit = playerLimit;
    }

    public void start() throws IOException
    {
        running = true;

        serverSocket = new ServerSocket(port);
        System.out.println("[Server] Listening on port " + port);

        tickExecutor = Executors.newSingleThreadScheduledExecutor();
        lastUpdateTime = System.nanoTime();
        tickExecutor.scheduleAtFixedRate(this::gameTick, 0, 16, TimeUnit.MILLISECONDS);

        try
        {
            while (running)
            {
                Socket sock = serverSocket.accept();

                if (players.size() >= playerLimit)
                {
                    System.out.println("[Server] Connection rejected: Player limit reached.");
                    sock.close();
                    continue;
                }

                int id = idCounter.getAndIncrement();
                int[] spawnTile = world.findSpawnTile();
                Player p = new Player(id, (float)spawnTile[0], (float)spawnTile[1], this);
                players.put(id, p);

                ClientHandler h = new ClientHandler(sock, id, this);
                handlers.add(h);
                pool.submit(h);

                System.out.println("[Server] Player #" + id + " connected.");
            }
        }
        catch (SocketException e)
        {
            if (running)
            {
                System.err.println("[Server] Socket Error: " + e.getMessage());
            }
        }
        finally
        {
            cleanup();
        }
    }

    public void stop()
    {
        running = false;
        try
        {
            if (serverSocket != null && !serverSocket.isClosed())
            {
                serverSocket.close();
            }
        }
        catch (IOException e)
        {
            System.err.println("[Server] Error closing socket: " + e.getMessage());
        }
    }

    private void cleanup()
    {
        running = false;
        if (tickExecutor != null)
            tickExecutor.shutdown();

        //disconnect all clients
        for (ClientHandler h : handlers)
        {
            try
            {
                h.sendMessage("SERVER_SHUTDOWN");
            }
            catch (Exception ignored) { }
        }

        handlers.clear();
        players.clear();

        System.out.println("[Server] Shut down complete.");
    }

    public World getWorld() { return world; }
    public Collection<Player> getAllPlayers() { return players.values(); }
    public Player getPlayer(int id) { return players.get(id); }

    public ClientHandler getClientHandler(int id)
    {
        for (ClientHandler h : handlers)
        {
            if (h.getClientId() == id)
                return h;
        }
        return null;
    }

    public void broadcast(String msg)
    {
        for (ClientHandler ch : handlers)
            ch.sendMessage(msg);
    }

    public void broadcastExcept(String msg, int excludeId)
    {
        for (ClientHandler ch : handlers)
            if (ch.getClientId() != excludeId)
                ch.sendMessage(msg);
    }

    public void sendMessageTo(String msg, int targetId)
    {
        ClientHandler ch = getClientHandler(targetId);
        if (ch != null) ch.sendMessage(msg);
    }

    public void removeClient(int id, ClientHandler handler)
    {
        players.remove(id);
        handlers.remove(handler);
        broadcast("PLAYER_LEAVE," + id);
        System.out.println("[Server] Player #" + id + " removed.");
    }

    private void gameTick()
    {
        long now = System.nanoTime();
        float deltaTime = (now - lastUpdateTime) / 1_000_000_000f;
        lastUpdateTime = now;

        for (Player p : players.values())
        {
            Physics.stepPlayer(p, getWorld(), deltaTime);
            if (p.hasMoved())
            {
                broadcast("PLAYER_MOVE," + p.getId() + "," + p.getX() + "," + p.getY());
                p.syncPosition();
            }
        }
    }
}