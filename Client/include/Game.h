#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <unordered_map>
#include <string>
#include <queue>
#include <mutex>
#include "Player.h"

class Network;

class Game {
public:
    Game();
    ~Game();

    void setNetwork(Network* n) { network = n; }

    void pushNetworkMessage(const std::string& msg);
    void processNetworkMessages();
    void handleInput(const SDL_Event& e);
    void update();
    void render(SDL_Renderer* renderer);

    int getLocalPlayerId() const { return localPlayerId; }
    void setLocalPlayerId(int id) { localPlayerId = id; }

private:
    Network* network = nullptr;
    int localPlayerId = -1;
    std::unordered_map<int, Player> players;
    TTF_Font* font = nullptr;

    std::mutex incomingMutex;
    std::queue<std::string> incomingMessages;

    void handleOneNetworkMessage(const std::string& msg);
    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
};
