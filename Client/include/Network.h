#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <SDL_net.h>

class Game;
class World;

class Network
{
public:
    Network();
    ~Network();

    bool connectToServer(const std::string& host, int port);
    void disconnect();
    void queueMessage(const std::string& msg);
    [[nodiscard]] bool isConnected() const { return connected; }

    void setGame(Game* g) { game = g; }
    void setWorld(World* w) { world = w; }

private:
    TCPsocket socket = nullptr;
    std::thread recvThread;
    std::thread sendThread;
    std::atomic<bool> connected{false};
    std::queue<std::string> sendQueue;
    std::mutex sendMutex;

    Game* game = nullptr;
    World* world = nullptr;

    void receiveLoop();
    void sendLoop();
};