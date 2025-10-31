#include "../include/Game.h"
#include "../include/Network.h"
#include <SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Swagaria",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Network network;
    Game game;
    network.setGame(&game);
    game.setNetwork(&network);

    if (!network.connectToServer("127.0.0.1", 25565)) {
        std::cerr << "Failed to connect to server" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                isRunning = false;
            if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0)
                game.handleInput(event);
        }

        game.processNetworkMessages();
        game.update();
        game.render(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    network.disconnect();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
