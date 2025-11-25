package com.swagaria.network;

import com.swagaria.game.Physics;
import com.swagaria.game.Player;
import com.swagaria.game.World;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.*;

public class Server
{
    private final int port;
    private final ExecutorService pool = Executors.newCachedThreadPool();
    private final AtomicInteger idCounter = new AtomicInteger(1);
    private final Map<Integer, Player> players = new ConcurrentHashMap<>();
    private final List<ClientHandler> handlers = new CopyOnWriteArrayList<>();
    private final World world = new World();
    private final boolean running = true;

    private final ScheduledExecutorService tickExecutor = Executors.newSingleThreadScheduledExecutor();
    private long lastUpdateTime = System.nanoTime();

    public Server(int port) { this.port = port; }

    public void start() throws IOException
    {
        try (ServerSocket ss = new ServerSocket(port))
        {
            System.out.println("[Server] Listening on port " + port);

            // start tick loop
            tickExecutor.scheduleAtFixedRate(this::gameTick, 0, 16, TimeUnit.MILLISECONDS);

            while (running)
            {
                Socket sock = ss.accept();
                int id = idCounter.getAndIncrement();

                //spawn inside the world near center (tile units)
                int[] spawnTile = world.findSpawnTile();
                float spawnX = spawnTile[0];  //horizontal center
                float spawnY = spawnTile[1];  //just above surface
                Player p = new Player(id, spawnX, spawnY);
                players.put(id, p);

                ClientHandler h = new ClientHandler(sock, id, this);
                handlers.add(h);
                pool.submit(h);

                System.out.println("[Server] Player #" + id + " connected.");
            }
        }
    }

    public World getWorld() { return world; }
    public Collection<Player> getAllPlayers() { return players.values(); }
    public Player getPlayer(int id) { return players.get(id); }

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
