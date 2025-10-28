#include <SDL.h>
#include <iostream>
#include "../include/Network.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[SDL] Initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    //initialise the network
    Network net;
    if (!net.init()) return -1;

    //connect to the server
    const std::string host = "127.0.0.1";
    const Uint16 port = 25565;

    if (!net.connectToServer(host, port)) {
        std::cerr << "[Client] Could not connect to server at "
                  << host << ":" << port << std::endl;
    } else {
        net.sendMessage("Client connected!");
    }

    //SDL window creation/rendering
    SDL_Window* window = SDL_CreateWindow(
        "Terraria Clone Client",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "[SDL] Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "[SDL] Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    //main loop
    bool running = true;
    SDL_Event event;

    Uint32 lastPing = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        //send pings to the server
        static Uint32 lastPing = 0;
        static int pingCount = 0;

        Uint32 now = SDL_GetTicks();
        if (now - lastPing > 2000) { //send a ping every 2 seconds to make sure server connection doesnt crash
            pingCount++;
            std::string message = "Ping " + std::to_string(pingCount) + " from client: hello!";
            net.sendMessage(message);
            lastPing = now;
        }

        //check for messages from server
        net.receiveMessage();

        //window render
        SDL_SetRenderDrawColor(renderer, 70, 165, 250, 255); //sky colour
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    //SDL/SDL_net cleanup once closed
    net.cleanup();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[Client] Exiting cleanly.\n";
    return 0;
}
