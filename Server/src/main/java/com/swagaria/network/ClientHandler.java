package com.swagaria.network;

import java.io.*;
import java.net.Socket;

/**
 * handles all communication with a single connected client,
 * each instance runs in its own thread (pretty cool)
 */
public class ClientHandler implements Runnable {
    private final Socket socket;
    private PrintWriter out;
    private BufferedReader in;

    public ClientHandler(Socket socket) {
        this.socket = socket;
    }

    @Override
    public void run() {
        try {
            //setup input/output streams
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            sendMessage("Welcome to the Swagaria server!");

            //listen for incoming messages from the client
            String inputLine;
            while ((inputLine = in.readLine()) != null) {
                System.out.println("[Server] From " + socket.getInetAddress().getHostAddress() + ": " + inputLine);

                //echo message back to client (testing for now)
                sendMessage("Echo: " + inputLine);
            }

        } catch (IOException e) {
            System.err.println("[Server] Lost connection with "
                    + socket.getInetAddress().getHostAddress() + ": " + e.getMessage());
        } finally {
            cleanup();
        }
    }

    //sends messages through to the client
    private void sendMessage(String message) {
        if (out != null) {
            out.println(message);
        }
    }

    //closes connection with the client
    private void cleanup() {
        try {
            if (in != null) in.close();
            if (out != null) out.close();
            socket.close();
            System.out.println("[Server] Disconnected " + socket.getInetAddress().getHostAddress());
        } catch (IOException ignored) {}
    }
}
