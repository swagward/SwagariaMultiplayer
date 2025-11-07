package com.swagaria;

import com.swagaria.network.Server;

public class Main {
    public static void main(String[] args) {
        int port = 25565; //default port for the server

        System.out.println("[Server] Starting Swagaria server on port " + port + "...");

        try {
            Server server = new Server(port);
            server.start();
        } catch (Exception e) {
            System.err.println("[Server] Failed to start: " + e.getMessage());
            e.printStackTrace();
        }
    }
}