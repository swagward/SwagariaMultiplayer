#include "../include/Game.h"
#include "../include/Network.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Game::Game() {
    if (TTF_Init() == -1) {
        std::cerr << "[SDL_ttf] Failed to initialize: " << TTF_GetError() << std::endl;
    } else {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
        if (!font) {
            std::cerr << "[SDL_ttf] Failed to load font: " << TTF_GetError() << std::endl;
        } else {
            std::cout << "[SDL_ttf] Font loaded successfully.\n";
        }
    }
}

void Game::pushNetworkMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(incomingMutex);
    incomingMessages.push(msg);
}

void Game::processNetworkMessages() {
    std::lock_guard<std::mutex> lock(incomingMutex);
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
        std::cout << "[Client] Assigned ID " << localPlayerId << std::endl;
    }
    else if (cmd == "SPAWN") {
        int id = std::stoi(parts[1]);
        float x = std::stof(parts[2]);
        float y = std::stof(parts[3]);
        std::string name = (parts.size() >= 5 ? parts[4] : "Player" + std::to_string(id));
        players[id] = {id, x, y, id == localPlayerId, name};
        std::cout << "[Client] Spawned player #" << id << " at " << x << "," << y << std::endl;
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
        float x = std::stof(parts[2]);
        float y = std::stof(parts[3]);
        std::string name = (parts.size() >= 5 ? parts[4] : "Player" + std::to_string(id));
        players[id] = {id, x, y, false, name};
        std::cout << "[Client] Player " << id << " joined (" << name << ")." << std::endl;
    }
    else if (cmd == "PLAYER_LEAVE") {
        int id = std::stoi(parts[1]);
        players.erase(id);
        std::cout << "[Client] Player " << id << " left." << std::endl;
    }
    else if (cmd == "PLAYER_NAME") {
        int id = std::stoi(parts[1]);
        std::string name = parts[2];
        if (players.count(id)) {
            players[id].name = name;
            std::cout << "[Client] Player " << id << " set name to " << name << std::endl;
        }
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
    if (network) network->queueMessage(oss.str());
}

void Game::update() {
    // No client-side prediction for now
}

void Game::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 100, 60, 30, 255);
    SDL_Rect floorRect{0, 332, 800, 68};
    SDL_RenderFillRect(renderer, &floorRect);

    for (auto& [id, p] : players) {
        SDL_Rect rect{ static_cast<int>(p.x), static_cast<int>(p.y), 32, 32 };
        if (p.isLocal)
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        else
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

        if (!p.name.empty()) {
            drawText(renderer, p.name, p.x - 10, p.y - 25,
                p.isLocal ? SDL_Color{0,255,0,255} : SDL_Color{255,255,0,255});
        }
    }

    SDL_RenderPresent(renderer);
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

Game::~Game() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();
}
