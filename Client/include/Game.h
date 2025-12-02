#pragma once
#include <SDL_ttf.h>
#include <mutex>
#include <queue>
#include <thread>
#include <map> // Include map for Player rendering
#include <vector> // Include vector for tiles

#include "Player.h"
#include "World.h"
#include "Camera.h"

class Network;

class Game
{
public:
    Game();
    ~Game();

    void setNetwork(Network* n) { network = n; }
    void pushNetworkMessage(const std::string& msg); //called from network thread
    void processNetworkMessages();                   //called from main thread
    void handleInput(const SDL_Event& e);            //called from main thread
    void render(SDL_Renderer* renderer);
    void update();

    int getLocalPlayerId() const { return localPlayerId; }
    void setLocalPlayerId(const int id) { localPlayerId = id; }

    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) const;

private:
    Network* network = nullptr;
    std::unique_ptr<World> world;
    Camera camera = Camera(800, 600);
    TTF_Font* font = nullptr;

    int localPlayerId = -1;
    std::unordered_map<int, Player> players;

    int currentHeldItem = 1;
    std::vector<int> tiles = { 1, 2, 3, 4, 5, 6, 7, 8 };
    //TODO: make adding tiles easier than current process:
    //adding to java server, setting isSolidTile or not, adding textures + loading, adding to tile vector, adding to switch statements for hotbar and UI render

    bool isFreecamActive = false;
    float freecamSpeed = 10.0f;
    const float freecamSpeedStep = 5.0f;
    const float minFreecamSpeed = 5.0f;
    const float maxFreecamSpeed = 50.0f;
    std::map<SDL_Keycode, bool> keysHeld;

    std::mutex incomingMutex;
    std::queue<std::string> incomingMessages;
    void handleOneNetworkMessage(const std::string& msg);
};