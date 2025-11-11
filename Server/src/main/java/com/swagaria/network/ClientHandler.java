package com.swagaria.network;

import com.swagaria.game.Player;
import com.swagaria.game.Chunk;

import java.io.*;
import java.net.Socket;

public class ClientHandler implements Runnable {
    private final Socket socket;
    private final int clientId;
    private final Server server;
    private PrintWriter out;
    private BufferedReader in;
    private volatile boolean running = true;

    public ClientHandler(Socket socket, int clientId, Server server) {
        this.socket = socket;
        this.clientId = clientId;
        this.server = server;
    }

    public int getClientId() { return clientId; }

    @Override
    public void run() {
        try {
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            // --- assign id ---
            out.println("ASSIGN_ID," + clientId);
            out.flush();

            // --- send existing players ---
            for (Player p : server.getAllPlayers()) {
                if (p.getId() != clientId) {
                    out.println("PLAYER_JOIN," + p.getId() + "," + p.getX() + "," + p.getY());
                    out.flush();
                }
            }

            // --- spawn this client's player ---
            Player me = server.getPlayer(clientId);
            if (me != null) {
                out.println("SPAWN," + clientId + "," + me.getX() + "," + me.getY());
                out.flush();
            }

            // --- send all chunks (serialize) ---
            try {
                // inform log and attempt to send; any IO error here means client disconnected
                System.out.println("[Server] Sending chunks to player #" + clientId + " (count=" + server.getWorld().getAllChunks().size() + ")");
                for (Chunk chunk : server.getWorld().getAllChunks()) {
                    // defensive: chunk.serialize() already safe-guards null tiles, but double-check
                    String msg = chunk.serialize();
                    out.println(msg);
                    out.flush();
                }
                System.out.println("[Server] Sent " + server.getWorld().getAllChunks().size() + " chunks to player #" + clientId);
            } catch (Exception e) {
                // client probably disconnected while we were sending; log concise message and stop
                System.err.println("[Server] Exception while sending chunks to player #" + clientId + ": " + e.getMessage());
                // fall through to cleanup
                running = false;
            }

            // --- notify others about this join (without chunk list) ---
            if (me != null) {
                server.broadcastExcept("PLAYER_JOIN," + clientId + "," + me.getX() + "," + me.getY(), clientId);
            }

            // --- main receive loop ---
            String line;
            while (running && (line = in.readLine()) != null) {
                handleLine(line);
            }

        } catch (IOException e) {
            // socket errors: log concise summary (avoid printing full stack trace here)
            System.err.println("[Server] Connection error with player #" + clientId + ": " + e.getMessage());
        } finally {
            cleanup();
            server.removeClient(clientId, this);
        }
    }

    private void handleLine(String line) {
        if (line == null || line.isEmpty()) return;

        String[] parts = line.split(",", 3);
        if (parts.length == 0) return;

        String cmd = parts[0];

        switch (cmd) {
            case "INPUT" -> handleInput(parts);
            // server does not accept client-set names in this version
            default -> System.out.println("[Server] Unknown command: " + line);
        }
    }

    private void handleInput(String[] parts) {
        if (parts.length < 3) return;
        int id;
        try {
            id = Integer.parseInt(parts[1]);
        } catch (NumberFormatException ex) {
            return;
        }
        String action = parts[2];

        Player p = server.getPlayer(id);
        if (p == null) return;

        boolean pressed = action.endsWith("_DOWN");
        p.setInput(action, pressed);
    }

    public void sendMessage(String msg) {
        if (out != null) {
            out.println(msg);
            out.flush();
        }
    }

    private void cleanup() {
        running = false;
        try { if (in != null) in.close(); } catch (IOException ignored) {}
        if (out != null) out.close();
        try { if (socket != null) socket.close(); } catch (IOException ignored) {}
    }
}
