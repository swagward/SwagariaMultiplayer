#include "../include/Game.h"

#include <algorithm>
#include <sstream>
#include "../include/Network.h"

Game::Game() {
    if (TTF_Init() == -1) {
        std::cerr << "[sdl_ttf] failed to initialize: " << TTF_GetError() << std::endl;
    } else {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
        if (!font)
            std::cerr << "[sdl_ttf] failed to load font: " << TTF_GetError() << std::endl;
        else
            std::cout << "[sdl_ttf] font loaded successfully.\n";
    }

    world = std::make_unique<World>();
}

Game::~Game() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();
}

void Game::pushNetworkMessage(const std::string& msg) {
    std::lock_guard lock(incomingMutex);
    incomingMessages.push(msg);
}

void Game::processNetworkMessages() {
    std::lock_guard lock(incomingMutex);
    while (!incomingMessages.empty()) {
        std::string msg = incomingMessages.front();
        incomingMessages.pop();
        handleOneNetworkMessage(msg);
    }
}

void Game::handleOneNetworkMessage(const std::string& msg) {
    std::istringstream ss(msg);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(ss, token, ',')) {
        parts.push_back(token);
    }

    if (parts.empty()) return;

    const std::string& cmd = parts[0];
    if (cmd == "ASSIGN_ID") {
        localPlayerId = std::stoi(parts[1]);
        std::cout << "[client] assigned id " << localPlayerId << std::endl;
    }
    else if (cmd == "SPAWN") {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, id == localPlayerId, "Player" + std::to_string(id) };
        std::cout << "[client] spawned player " << id << " at " << x << "," << y << std::endl;
    }
    else if (cmd == "PLAYER_MOVE") {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        if (players.count(id)) {
            players[id].x = x;
            players[id].y = y;
        }
    }
    else if (cmd == "PLAYER_JOIN") {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, false, "Player" + std::to_string(id) };
        std::cout << "[client] player " << id << " joined\n";
    }
    else if (cmd == "PLAYER_LEAVE") {
        const int id = std::stoi(parts[1]);
        players.erase(id);
        std::cout << "[client] player " << id << " left\n";
    }
    else if (cmd == "CHUNK_DATA") {
        if (!world)
            return;

        int chunkX = std::stoi(parts[1]);
        int chunkY = std::stoi(parts[2]);
        auto chunk = std::make_unique<Chunk>(chunkX, chunkY);

        constexpr int expectedTileCount = Chunk::SIZE * Chunk::SIZE;
        for (int i = 0; i < expectedTileCount; i++) {
            const int type = std::stoi(parts[3 + i]);
            const int x = i % Chunk::SIZE;
            const int y = i / Chunk::SIZE;
            chunk->setTile(x, y, type);
        }

        world->addChunk(std::move(chunk));
        std::cout << "[client] received chunk (" << chunkX << "," << chunkY << ")\n";
    }
}

void Game::handleInput(const SDL_Event& e) const {
    if (localPlayerId == -1) return;

    std::string action;
    switch (e.key.keysym.sym) {
        case SDLK_w: action = (e.type == SDL_KEYDOWN ? "UP_DOWN" : "UP_UP"); break;
        case SDLK_s: action = (e.type == SDL_KEYDOWN ? "DOWN_DOWN" : "DOWN_UP"); break;
        case SDLK_a: action = (e.type == SDL_KEYDOWN ? "LEFT_DOWN" : "LEFT_UP"); break;
        case SDLK_d: action = (e.type == SDL_KEYDOWN ? "RIGHT_DOWN" : "RIGHT_UP"); break;
        default: return;
    }

    std::ostringstream oss;
    oss << "INPUT," << localPlayerId << "," << action;

    if (network) //
        network->queueMessage(oss.str());
}

void Game::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255);
    SDL_RenderClear(renderer);

    constexpr int tilePxSize = 16;

    int winW, winH;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);

    int offsetX = 0;
    int offsetY = 0;

    // center the view on the local player
    if (players.count(localPlayerId)) {
        auto& p = players[localPlayerId];
        int playerCenterX = static_cast<int>(p.x * tilePxSize + tilePxSize / 2);
        int playerCenterY = static_cast<int>(p.y * tilePxSize + tilePxSize / 2);

        offsetX = (winW / 2) - playerCenterX;
        offsetY = (winH / 2) - playerCenterY;
    }

    // render all chunks
    for (auto& [key, chunkPtr] : world->chunks) {
        Chunk* chunk = chunkPtr.get();
        int chunkWorldX = chunk->chunkX * Chunk::SIZE * tilePxSize;
        int chunkWorldY = (World::WORLD_HEIGHT_IN_CHUNKS - 1 - chunk->chunkY) * Chunk::SIZE * tilePxSize;

        for (int y = 0; y < Chunk::SIZE; ++y) {
            int flippedY = Chunk::SIZE - 1 - y;
            for (int x = 0; x < Chunk::SIZE; ++x) {
                auto& [type] = chunk->tiles[flippedY][x];
                int worldX = chunkWorldX + x * tilePxSize + offsetX;
                int worldY = chunkWorldY + y * tilePxSize + offsetY;

                if (type == 0) continue; // air

                SDL_Rect tileRect{ worldX, worldY, tilePxSize, tilePxSize };
                switch (type) {
                    case 1: SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); break;       // grass
                    case 2: SDL_SetRenderDrawColor(renderer, 150, 50, 0, 255); break;      // dirt
                    case 3: SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); break;   // stone
                    default: SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); break;    // unknown
                }
                SDL_RenderFillRect(renderer, &tileRect);
            }
        }
    }

    // render all players
    for (auto& [id, p] : players) {
        SDL_Rect rect{
            static_cast<int>(p.x * tilePxSize + offsetX),
            static_cast<int>(p.y * tilePxSize + offsetY),
            tilePxSize * 2,  // player width in pixels
            tilePxSize * 2   // player height in pixels
        };

        if (p.isLocal)
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        else
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}



void Game::drawText(SDL_Renderer* renderer, const std::string& text, const int x, const int y, const SDL_Color color) const {
    if (!font) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    const SDL_Rect dstRect = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}
