#include "SDL_image.h"
#include "../include/Game.h"
#include "../include/Network.h"
#include "../include/TextureManager.h"

enum class AppState { MENU, CONNECTING, IN_GAME };

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
    texManager.loadTexture("dirt_bg", "assets/textures/tiles/dirt_bg.png", renderer);
    texManager.loadTexture("copper_pickaxe", "assets/textures/tools/copper_pickaxe.png", renderer);
    texManager.loadTexture("copper_axe", "assets/textures/tools/copper_axe.png", renderer);
    texManager.loadTexture("copper_hammer", "assets/textures/tools/copper_hammer.png", renderer);

    Network network;
    Game game;
    network.setGame(&game);
    game.setNetwork(&network);

    auto currentState = AppState::MENU;
    std::string ipInput = "192.168.56.1";
    bool isRunning = true;
    SDL_Event event;

    Uint32 fpsLastTime = SDL_GetTicks();
    Uint32 fpsFrames = 0;
    float fps = 0.0f;

    SDL_StartTextInput();

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;

            if (currentState == AppState::MENU) {
                if (event.type == SDL_TEXTINPUT) {
                    ipInput += event.text.text;
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && !ipInput.empty()) {
                        ipInput.pop_back();
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        currentState = AppState::CONNECTING;
                    }
                }
            } else if (currentState == AppState::IN_GAME) {
                game.handleInput(event);
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
        SDL_RenderClear(renderer);

        if (currentState == AppState::MENU) {
            game.drawText(renderer, "SWAGARIA MULTIPLAYER", 250, 150, {255, 255, 0, 255});
            game.drawText(renderer, "Enter Server IP:", 250, 250, {255, 255, 255, 255});
            game.drawText(renderer, ipInput + "_", 250, 300, {0, 255, 255, 255});
            game.drawText(renderer, "Press ENTER to Connect", 250, 400, {150, 150, 150, 255});
        }
        else if (currentState == AppState::CONNECTING) {
            game.drawText(renderer, "Connecting to " + ipInput + "...", 250, 300, {255, 255, 255, 255});
            SDL_RenderPresent(renderer); // Force render to show message

            if (network.connectToServer(ipInput, 25565)) {
                currentState = AppState::IN_GAME;
                SDL_StopTextInput();
            } else {
                currentState = AppState::MENU; // Fallback on fail
            }
        }
        else if (currentState == AppState::IN_GAME) {
            game.processNetworkMessages();
            game.update();
            game.render(renderer);
        }

        //fps counting in title bar
        fpsFrames++;
        if (const Uint32 currentTime = SDL_GetTicks(); currentTime - fpsLastTime >= 500)
        {
            //update every half second
            fps = (fpsFrames * 1000.0f) / (currentTime - fpsLastTime);
            fpsLastTime = currentTime;
            fpsFrames = 0;

            std::string title = "Swagaria - FPS: " + std::to_string(static_cast<int>(fps));
            SDL_SetWindowTitle(window, title.c_str());
        }

        if (currentState != AppState::IN_GAME) SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    network.disconnect();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
