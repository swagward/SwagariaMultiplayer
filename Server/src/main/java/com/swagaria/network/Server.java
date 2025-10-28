package com.swagaria.network;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * the main server class | listens for incoming client connections
 * and assigns each one its own handler thread
 */
public class Server {
    private final int port;
    private boolean running;
    private final ExecutorService clientPool;

    public Server(int port) {
        this.port = port;
        this.clientPool = Executors.newFixedThreadPool(2); //maximum amount of clients able to join
    }

    //listens for new clients connecting
    public void start() throws IOException {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            running = true;
            System.out.println("[Server] Listening for players on port " + port);

            while (running) {
                Socket clientSocket = serverSocket.accept();
                String clientAddress = clientSocket.getInetAddress().getHostAddress();

                System.out.println("[Server] Player joined from " + clientAddress);

                //assign a handler thread to the client
                clientPool.submit(new ClientHandler(clientSocket));
            }
        }
    }
}
