#pragma once
#include <SDL_ttf.h>
#include <mutex>
#include <queue>
#include <thread>

#include "Player.h"
#include "World.h"

class Network;

class Game
{
public:
    Game();
    ~Game();

    void setNetwork(Network* n) { network = n; }
    void pushNetworkMessage(const std::string& msg); //called from network thread
    void processNetworkMessages();                   //called from main thread
    void handleInput(const SDL_Event& e);      //called from main thread
    void render(SDL_Renderer* renderer);

    int getLocalPlayerId() const { return localPlayerId; }
    void setLocalPlayerId(const int id) { localPlayerId = id; }

    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) const;

private:
    Network* network = nullptr;
    std::unique_ptr<World> world;
    TTF_Font* font = nullptr;

    int localPlayerId = -1;
    std::unordered_map<int, Player> players;

    int cameraX = 0;
    int cameraY = 0;

    int currentHeldItem = 1;
    std::vector<int> tiles = { 1, 2, 3, 4, 5 };

    std::mutex incomingMutex;
    std::queue<std::string> incomingMessages;

    void handleOneNetworkMessage(const std::string& msg);
};
