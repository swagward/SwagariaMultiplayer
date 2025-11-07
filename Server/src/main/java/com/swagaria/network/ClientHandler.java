package com.swagaria.network;

import com.swagaria.game.Player;
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

    public int getClientId() {
        return clientId;
    }

    @Override
    public void run() {
        try {
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            // assign ID
            out.println("ASSIGN_ID," + clientId);
            out.flush();

            // send all existing players
            for (Player p : server.getAllPlayers()) {
                if (p.getId() != clientId) {
                    out.println("PLAYER_JOIN," + p.getId() + "," + p.getX() + "," + p.getY());
                    out.flush();
                }
            }

            // spawn local player
            Player me = server.getPlayer(clientId);
            if (me != null) {
                out.println("SPAWN," + clientId + "," + me.getX() + "," + me.getY());
                out.flush();
            }

            for (var chunk : server.getWorld().getAllChunks()) {
                out.println(chunk.serialize());
                out.flush();
            }
            System.out.println("[Server] Sent " + server.getWorld().getAllChunks().size() + " chunks to player #" + clientId);

            // notify others
            if (me != null) {
                server.broadcastExcept("PLAYER_JOIN," + clientId + "," + me.getX() + "," + me.getY(), clientId);
            }

            // listen
            String line;
            while (running && (line = in.readLine()) != null) {
                handleLine(line);
            }

        } catch (IOException e) {
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
            default -> System.out.println("[Server] Unknown command: " + cmd);
        }
    }

    private void handleInput(String[] parts) {
        if (parts.length < 3) return;
        int id = Integer.parseInt(parts[1]);
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
        try {
            if (in != null) in.close();
        } catch (IOException ignored) {}
        if (out != null) out.close();
        try {
            if (socket != null) socket.close();
        } catch (IOException ignored) {}
    }
}