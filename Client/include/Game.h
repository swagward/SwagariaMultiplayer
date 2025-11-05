#pragma once
#include <SDL_ttf.h>
#include <mutex>
#include <queue>
#include <thread>

#include "Player.h"
#include "World.h"
#include "Camera.h"

class Network;

class Game {
public:
    Game();
    ~Game();

    void setNetwork(Network* n) { network = n; }
    void pushNetworkMessage(const std::string& msg); //called from network thread
    void processNetworkMessages(); //called from main thread
    void handleInput(const SDL_Event& e); //called from main thread
    void render(SDL_Renderer* renderer);

    int getLocalPlayerId() const { return localPlayerId; }
    void setLocalPlayerId(int id) { localPlayerId = id; }
    void printChunkLayout();

    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);

private:
    Network* network = nullptr;
    std::unique_ptr<World> world;
    std::unique_ptr<Camera> camera;

    int localPlayerId = -1;
    std::unordered_map<int, Player> players;
    TTF_Font* font = nullptr;

    std::mutex incomingMutex;
    std::queue<std::string> incomingMessages;

    void handleOneNetworkMessage(const std::string& msg);
};
