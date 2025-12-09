#include "SDL_image.h"
#include "../include/Game.h"
#include "../include/Network.h"
#include "../include/TextureManager.h"

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "failed to initialize sdl: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)))
    {
        std::cerr << "failed to initialize SDL_image: " << SDL_GetError() << std::endl;
    }

    SDL_Window* window = SDL_CreateWindow("Swagaria",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Network network;
    Game game;
    network.setGame(&game);
    game.setNetwork(&network);

    if (!network.connectToServer("127.0.0.1", 25565))
    {
        std::cerr << "failed to connect to server" << std::endl;
        return -1;
    }

    //load game textures
    TextureManager& texManager = TextureManager::getInstance();
    texManager.loadTexture("grass", "assets/textures/tiles/grass.png", renderer);
    texManager.loadTexture("dirt", "assets/textures/tiles/dirt.png", renderer);
    texManager.loadTexture("missing_texture", "assets/textures/tiles/missing_texture.png", renderer);
    texManager.loadTexture("stone", "assets/textures/tiles/stone.png", renderer);
    texManager.loadTexture("torch", "assets/textures/tiles/torch.png", renderer);
    texManager.loadTexture("wood_log", "assets/textures/tiles/wood_log.png", renderer);
    texManager.loadTexture("wood_plank", "assets/textures/tiles/wood_plank.png", renderer);
    texManager.loadTexture("wood_plank_bg", "assets/textures/tiles/wood_plank_bg.png", renderer);
    texManager.loadTexture("stone_bg", "assets/textures/tiles/stone_bg.png", renderer);

    bool isRunning = true;
    SDL_Event event;

    Uint32 fpsLastTime = SDL_GetTicks();
    Uint32 fpsFrames = 0;
    float fps = 0.0f;

    while (isRunning)
    {
        //handle input
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) isRunning = false;
            else game.handleInput(event);
        }

        //game logic
        game.processNetworkMessages();
        game.update();
        game.render(renderer);

        //fps counting in title bar
        fpsFrames++;
        if (const Uint32 currentTime = SDL_GetTicks(); currentTime - fpsLastTime >= 500)
        {   //update every half second
            fps = (fpsFrames * 1000.0f) / (currentTime - fpsLastTime);
            fpsLastTime = currentTime;
            fpsFrames = 0;

            std::string title = "Swagaria - FPS: " + std::to_string(static_cast<int>(fps));
            SDL_SetWindowTitle(window, title.c_str());
        }

        SDL_Delay(1);
    }

    network.disconnect();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
