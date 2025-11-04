// ClientHandler.java
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

            // --- Assign ID to client ---
            out.println("ASSIGN_ID," + clientId);
            out.flush();

            // --- Send all existing players to the new client ---
            for (Player p : server.getAllPlayers()) {
                if (p.getId() != clientId) {
                    out.println("PLAYER_JOIN," + p.getId() + "," + p.getX() + "," + p.getY() + "," + p.getName());
                    out.flush();
                }
            }

            // --- Spawn this client’s own player ---
            Player me = server.getPlayer(clientId);
            if (me != null) {
                out.println("SPAWN," + clientId + "," + me.getX() + "," + me.getY() + "," + me.getName());
                out.flush();
            }

            // --- Let everyone else know this player joined ---
            if (me != null) {
                server.broadcastExcept("PLAYER_JOIN," + clientId + "," + me.getX() + "," + me.getY() + "," + me.getName(), clientId);
            }

            // --- Main receive loop ---
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

        String[] parts = line.split(",", 3); // Allow names with commas
        if (parts.length == 0) return;

        String cmd = parts[0];

        switch (cmd) {
            case "INPUT" -> handleInput(parts);
            case "SETNAME" -> handleSetName(parts);
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

    private void handleSetName(String[] parts) {
        if (parts.length < 3) return;
        int id = Integer.parseInt(parts[1]);
        String newName = parts[2];

        Player p = server.getPlayer(id);
        if (p == null) return;

        // Update server-side name
        p.setName(newName);
        System.out.println("[Server] Player #" + id + " set name to " + newName);

        // Broadcast to all clients
        String msg = "PLAYER_NAME," + id + "," + newName;
        server.broadcast(msg);
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
