#include "../include/Game.h"

#include <algorithm>

#include "../include/Network.h"
#include <sstream>

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

    // create world and camera with screen size
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
    const std::string& cmd = parts[0];

    if (cmd == "ASSIGN_ID") {
        localPlayerId = std::stoi(parts[1]);
        std::cout << "[client] assigned id " << localPlayerId << std::endl;
    }
    else if (cmd == "SPAWN") {
        int id = std::stoi(parts[1]);
        float x = std::stof(parts[2]);
        float y = std::stof(parts[3]);
        players[id] = { id, x, y, id == localPlayerId, "Player" + std::to_string(id) };
        std::cout << "[client] spawned player " << id << " at " << x << "," << y << std::endl;
    }
    else if (cmd == "PLAYER_MOVE") {
        int id = std::stoi(parts[1]);
        float x = std::stof(parts[2]);
        float y = std::stof(parts[3]);
        if (players.count(id)) {
            players[id].x = x;
            players[id].y = y;
        }
    }
    else if (cmd == "PLAYER_JOIN") {
        int id = std::stoi(parts[1]);
        float x = std::stof(parts[2]); //MAKE CONST ^ V
        float y = std::stof(parts[3]);
        players[id] = { id, x, y, false, "Player" + std::to_string(id) };
        std::cout << "[client] player " << id << " joined\n";
    }
    else if (cmd == "PLAYER_LEAVE") {
        int id = std::stoi(parts[1]);
        players.erase(id);
        std::cout << "[client] player " << id << " left\n";
    }
    else if (cmd == "CHUNK_DATA") {
        if (!world)
            return;

        int chunkX = std::stoi(parts[1]);
        int chunkY = std::stoi(parts[2]);
        auto chunk = std::make_unique<Chunk>(chunkX, chunkY);

        int expectedTileCount = Chunk::SIZE * Chunk::SIZE;
        for (int i = 0; i < expectedTileCount; i++) {
            int type = std::stoi(parts[3 + i]);
            int x = i % Chunk::SIZE;
            int y = i / Chunk::SIZE;
            chunk->setTile(x, y, type);
        }

        world->addChunk(std::move(chunk));
        std::cout << "[client] received chunk (" << chunkX << "," << chunkY << ")\n";
        //printChunkLayout();
    }
}

void Game::handleInput(const SDL_Event& e) {
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
    if (network) //IDE says always false but its wrong so dont change.
        network->queueMessage(oss.str());
}

void Game::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255);
    SDL_RenderClear(renderer);

    // center camera on local player
    if (players.count(localPlayerId)) {
        auto& p = players[localPlayerId];
        camera->centerOn(p.x + 16, p.y + 16);
    }

    const int tilePxSize = 2;

    // calculate visible world area (frustum)
    const float camLeft = camera->x;
    const float camTop = camera->y;
    const float camRight = camera->x + camera->screenWidth;
    const float camBottom = camera->y + camera->screenHeight;

    // precompute chunk size in pixels
    const int chunkPxSize = Chunk::SIZE * tilePxSize;

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

void Game::printChunkLayout() {
    std::cout << "[client] chunk layout grid:\n";

    // collect all chunk coordinates
    std::vector<std::pair<int, int>> coords;
    coords.reserve(world->chunks.size());

    for (auto& [key, _] : world->chunks) {
        int x, y;
        char comma;
        std::istringstream ss(key);
        if (ss >> x >> comma >> y)
            coords.emplace_back(x, y);
    }

    // sort them by y, then x (to make the print deterministic)
    std::sort(coords.begin(), coords.end(), [](auto& a, auto& b) {
        if (a.second == b.second)
            return a.first < b.first;
        return a.second < b.second;
    });

    // determine grid size
    int maxX = 0, maxY = 0;
    for (auto& [x, y] : coords) {
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }

    // print grid
    for (int y = maxY; y >= 0; --y) { // top-to-bottom to visualize properly
        for (int x = 0; x <= maxX; ++x) {
            if (std::find(coords.begin(), coords.end(), std::make_pair(x, y)) != coords.end())
                std::cout << "(" << x << "," << y << ") ";
            else
                std::cout << "...... ";
        }
        std::cout << "\n";
    }
}


void Game::drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}
