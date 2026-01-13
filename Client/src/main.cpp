#include "SDL_image.h"
#include "SDL_mixer.h"
#include "../include/AudioManager.h"
#include "../include/Button.h"
#include "../include/Game.h"
#include "../include/Network.h"
#include "../include/TextureManager.h"

enum class AppState { MAIN_MENU, SETTINGS, IP_INPUT, CONNECTING, IN_GAME };

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "[SDL2] Failed to initialize: " << SDL_GetError() << std::endl;
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)))
        std::cerr << "[SDL_IMAGE] Failed to initialize: " << SDL_GetError() << std::endl;
    if (int flags = MIX_INIT_MP3; (Mix_Init(flags) & flags) != flags)
        std::cerr << "[SDL_MIXER] Failed to init MP3: " << Mix_GetError() << std::endl;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        std::cerr << "[SDL_MIXER] Failed to open audio: " << Mix_GetError() << std::endl;

    SDL_Window* window = SDL_CreateWindow("Swagaria",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);

    //load tile textures
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
    texManager.loadTexture("leaves", "assets/textures/tiles/leaves.png", renderer);
    texManager.loadTexture("flowers", "assets/textures/tiles/flowers.png", renderer);
    texManager.loadTexture("tall_grass", "assets/textures/tiles/tall_grass.png", renderer);
    texManager.loadTexture("slate", "assets/textures/tiles/slate.png", renderer);
    texManager.loadTexture("slate_bg", "assets/textures/tiles/slate_bg.png", renderer);
    texManager.loadTexture("bedrock", "assets/textures/tiles/bedrock.png", renderer);
    texManager.loadTexture("wood_platform", "assets/textures/tiles/wood_platform.png", renderer);
    texManager.loadTexture("glass", "assets/textures/tiles/glass.png", renderer);
    //load item textures
    texManager.loadTexture("copper_pickaxe", "assets/textures/tools/copper_pickaxe.png", renderer);
    texManager.loadTexture("copper_axe", "assets/textures/tools/copper_axe.png", renderer);
    texManager.loadTexture("copper_hammer", "assets/textures/tools/copper_hammer.png", renderer);
    //load ui textures
    texManager.loadTexture("menu_bg", "assets/textures/ui/menu_bg.png", renderer);
    //load sfx & music
    AudioManager& audioManager = AudioManager::getInstance();
    audioManager.loadSFX("button_press", "assets/audio/ui/button_pressed.wav");
    audioManager.loadSFX("block_break", "assets/audio/game/block_break.wav");

    Network network;
    Game game;
    network.setGame(&game);
    game.setNetwork(&network);

    auto currentState = AppState::MAIN_MENU;
    std::string ipInput = "Enter Server IP";
    bool isRunning = true;
    SDL_Event event;

    //menu buttons
    Button playButton = {{300, 200, 200, 50}, "PLAY", {60, 60, 70}, {100, 100, 120}};
    Button settingsButton = {{300, 270, 200, 50}, "SETTINGS", {60, 60, 70}, {100, 100, 120}};
    Button quitButton = {{300, 340, 200, 50}, "QUIT", {60, 60, 70}, {150, 50, 50}};
    Button joinButton = {{300, 350, 200, 50}, "JOIN SERVER", {50, 150, 50}, {70, 200, 70}};
    Button backButton = {{300, 420, 200, 50}, "BACK", {60, 60, 70}, {100, 100, 120}};

    Uint32 fpsLastTime = SDL_GetTicks();
    Uint32 fpsFrames = 0;
    float fps = 0.0f;

    while (isRunning)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) isRunning = false;

            if (currentState != AppState::IN_GAME)
            {
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                {
                    bool clicked = false;
                    if (currentState == AppState::MAIN_MENU)
                    {
                        if (playButton.isHovering(mouseX, mouseY))
                        {
                            currentState = AppState::IP_INPUT;
                            clicked = true;
                        }
                        else if (settingsButton.isHovering(mouseX, mouseY))
                        {
                            currentState = AppState::SETTINGS;
                            clicked = true;
                        }
                        else if (quitButton.isHovering(mouseX, mouseY))
                            isRunning = false;
                    }
                    else if (currentState == AppState::SETTINGS || currentState == AppState::IP_INPUT)
                    {
                        if (backButton.isHovering(mouseX, mouseY))
                        {
                            currentState = AppState::MAIN_MENU;
                            clicked = true;
                        }
                        if (currentState == AppState::IP_INPUT && joinButton.isHovering(mouseX, mouseY))
                        {
                            currentState = AppState::CONNECTING;
                            clicked = true;
                        }
                    }

                    if (clicked) audioManager.playSFX("button_press");
                }

                //ip input field
                if (currentState == AppState::IP_INPUT)
                {
                    if (event.type == SDL_TEXTINPUT)
                        ipInput += event.text.text;
                    else if (event.type == SDL_KEYDOWN)
                    {
                        if (event.key.keysym.sym == SDLK_BACKSPACE && !ipInput.empty())
                            ipInput.pop_back();
                        if (event.key.keysym.sym == SDLK_RETURN)
                        {
                            currentState = AppState::CONNECTING;
                            audioManager.playSFX("button_press");
                        }
                        if (event.key.keysym.sym == SDLK_ESCAPE)
                            currentState = AppState::MAIN_MENU;
                    }
                }
            }
            else game.handleInput(event);
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
        SDL_RenderClear(renderer);

        if (currentState != AppState::IN_GAME)
        {
            texManager.draw(renderer, "menu_bg", 0, 0, 800, 600);

            if (currentState == AppState::MAIN_MENU)
            {
                game.drawText(renderer, "SWAGARIA", 340, 100, {255, 255, 0, 255});
                playButton.render(renderer, game, mouseX, mouseY);
                settingsButton.render(renderer, game, mouseX, mouseY);
                quitButton.render(renderer, game, mouseX, mouseY);
            }
            else if (currentState == AppState::SETTINGS)
            {
                game.drawText(renderer, "SETTINGS", 340, 100, {255, 255, 255, 255});
                game.drawText(renderer, "Sound Volume: [||||||||--] 80%", 250, 250, {200, 200, 200, 255});
                game.drawText(renderer, "[Work In Progress]", 310, 300, {200, 0, 0, 255});
                backButton.render(renderer, game, mouseX, mouseY);
            }
            else if (currentState == AppState::IP_INPUT)
            {
                game.drawText(renderer, "MULTIPLAYER", 320, 100, {0, 255, 255, 255});
                game.drawText(renderer, "Enter Server IP:", 300, 200, {255, 255, 255, 255});
                game.drawText(renderer, ipInput + "|", 300, 250, {255, 255, 0, 255});
                joinButton.render(renderer, game, mouseX, mouseY);
                backButton.render(renderer, game, mouseX, mouseY);
            }
            else if (currentState == AppState::CONNECTING)
            {
                game.drawText(renderer, "Connecting to " + ipInput + "...", 280, 300, {255, 255, 255, 255});
                SDL_RenderPresent(renderer);
                if (network.connectToServer(ipInput, 25565))
                    currentState = AppState::IN_GAME;
                else
                    currentState = AppState::IP_INPUT;
            }
        }
        else
        {
            //check if still connected or not
            if (!network.isConnected()) {
                std::cout << "[CLIENT] Disconnected from server. Returning to menu." << std::endl;
                currentState = AppState::MAIN_MENU;
                network.disconnect(); //cleanup threads & socket
            } else {
                game.processNetworkMessages();
                game.update();
                game.render(renderer);
            }
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
