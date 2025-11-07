#include "../include/Game.h"

#include <algorithm>

#include "../include/Network.h"
#include <sstream>

Game::Game() {
    if (TTF_Init() == -1) {
        std::cerr << "[sdl_ttf] failed to initialize: " << TTF_GetError() << std::endl;
    } else {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16); //add font file inside of game so not pulling from windows dir (would break on linux/mac)
        if (!font)
            std::cerr << "[sdl_ttf] failed to load font: " << TTF_GetError() << std::endl;
        else
            std::cout << "[sdl_ttf] font loaded successfully.\n";
    }

    //create world and camera with screen size
    world = std::make_unique<World>();
    camera = std::make_unique<Camera>(800, 600);
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

    if (const std::string& cmd = parts[0]; cmd == "ASSIGN_ID") {
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
        //printChunkLayout();
    }
}

void Game::handleInput(const SDL_Event& e) const
{
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

    //IDE says always false but its wrong so don't change.
    if (network)
        network->queueMessage(oss.str());
}

void Game::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255);
    SDL_RenderClear(renderer);

    // center camera on local player
    if (players.count(localPlayerId)) {
        const auto& p = players[localPlayerId];
        camera->centerOn(p.x + 16, p.y + 16);
    }

    constexpr int tilePxSize = 2;

    // calculate visible world area (frustum)
    const float camLeft = camera->x;
    const float camTop = camera->y;
    const float camRight = camera->x + camera->screenWidth;
    const float camBottom = camera->y + camera->screenHeight;

    // precompute chunk size in pixels
    constexpr int chunkPxSize = Chunk::SIZE * tilePxSize;

    // draw only chunks inside the camera frustum
    for (auto& [key, chunkPtr] : world->chunks) {
        Chunk* chunk = chunkPtr.get();

        // compute chunk bounds in world space
        const int chunkWorldX = chunk->chunkX * chunkPxSize;
        const int chunkWorldY = (World::WORLD_HEIGHT_IN_CHUNKS - 1 - chunk->chunkY) * chunkPxSize;
        const int chunkRight = chunkWorldX + chunkPxSize;
        const int chunkBottom = chunkWorldY + chunkPxSize;

        // frustum culling: skip chunks fully outside the camera view
        if (chunkRight < camLeft || chunkWorldX > camRight ||
            chunkBottom < camTop || chunkWorldY > camBottom) {
            continue;
        }

        // draw visible tiles in chunk
        for (int y = 0; y < Chunk::SIZE; ++y) {
            for (int x = 0; x < Chunk::SIZE; ++x) {
                Tile& t = chunk->tiles[y][x];
                if (t.type == 0) continue;

                const int worldX = chunkWorldX + x * tilePxSize;
                const int worldY = chunkWorldY + y * tilePxSize;

                // skip tiles outside view
                if (worldX + tilePxSize < camLeft || worldX > camRight ||
                    worldY + tilePxSize < camTop || worldY > camBottom) {
                    continue;
                }

                SDL_Rect tileRect{
                    camera->worldToScreenX(worldX),
                    camera->worldToScreenY(worldY),
                    tilePxSize, tilePxSize
                };

                if (t.type == 1)
                    SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // dirt
                else
                    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // stone

                SDL_RenderFillRect(renderer, &tileRect);
            }
        }
    }

    // draw players (with camera)
    for (auto& [id, p] : players) {
        SDL_Rect rect{
            camera->worldToScreenX(p.x),
            camera->worldToScreenY(p.y),
            32, 32
        };

        if (p.isLocal)
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        else
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

        SDL_RenderFillRect(renderer, &rect);

        if (!p.name.empty()) {
            drawText(renderer, p.name,
                camera->worldToScreenX(p.x) - 10,
                camera->worldToScreenY(p.y) - 25,
                p.isLocal ? SDL_Color{0,255,0,255} : SDL_Color{255,255,0,255});
        }
    }

    SDL_RenderPresent(renderer);
}

void Game::drawText(SDL_Renderer* renderer, const std::string& text, const int x, const int y, const SDL_Color color) const
{
    if (!font) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    const SDL_Rect dstRect = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}