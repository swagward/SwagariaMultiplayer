#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "Player.h"

#pragma comment(lib, "ws2_32.lib")

class Game;

class Network {
public:
    Network();
    ~Network();

    bool connectToServer(const std::string& host, int port);
    void disconnect();
    void queueMessage(const std::string& msg);
    bool isConnected() const { return connected; }

    void setGame(Game* g) { game = g; }

private:
    SOCKET socket = INVALID_SOCKET;
    std::thread recvThread;
    std::thread sendThread;
    std::atomic<bool> connected{false};
    std::queue<std::string> sendQueue;
    std::mutex sendMutex;

    Game* game = nullptr;

    void receiveLoop();
    void sendLoop();
};
