#include "../include/Game.h"
#include "../include/Network.h"
#include <algorithm>
#include <cmath>
#include <sstream>

#include "../include/TextureManager.h"


class TextureManager;

Game::Game()
{
    std::cout << "Client started!" << std::endl;

    if (TTF_Init() == -1)
        std::cerr << "[sdl_ttf] failed to initialize: " << TTF_GetError() << std::endl;
    else
    {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
        if (!font)
            std::cerr << "[sdl_ttf] failed to load font: " << TTF_GetError() << std::endl;
        else
            std::cout << "[sdl_ttf] font loaded successfully.\n";
    }

    world = std::make_unique<World>();
}

Game::~Game()
{
    if (font)
    {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();
}

void Game::pushNetworkMessage(const std::string& msg)
{
    std::lock_guard lock(incomingMutex);
    incomingMessages.push(msg);
}

void Game::processNetworkMessages()
{
    std::lock_guard lock(incomingMutex);
    while (!incomingMessages.empty())
    {
        std::string msg = incomingMessages.front();
        incomingMessages.pop();
        handleOneNetworkMessage(msg);
    }
}

void Game::handleOneNetworkMessage(const std::string& msg)
{
    std::istringstream ss(msg);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(ss, token, ','))
        parts.push_back(token);
    if (parts.empty())
        return;

    if (const std::string& cmd = parts[0]; cmd == "ASSIGN_ID")
    {
        localPlayerId = std::stoi(parts[1]);
        std::cout << "[client] assigned id " << localPlayerId << std::endl;
    }
    else if (cmd == "SPAWN")
    {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, id == localPlayerId, "Player" + std::to_string(id) };
        std::cout << "[client] spawned player " << id << " at " << x << "," << y << std::endl;
    }
    else if (cmd == "PLAYER_MOVE")
    {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        if (players.count(id))
        {
            players[id].x = x;
            players[id].y = y;
        }
    }
    else if (cmd == "PLAYER_JOIN")
    {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, false, "Player" + std::to_string(id) };
        std::cout << "[client] player " << id << " joined\n";
    }
    else if (cmd == "PLAYER_LEAVE")
    {
        const int id = std::stoi(parts[1]);
        players.erase(id);
        std::cout << "[client] player " << id << " left\n";
    }
    else if (cmd == "CHUNK_DATA")
    {
        if (!world)
            return;

        int chunkX = std::stoi(parts[1]);
        int chunkY = std::stoi(parts[2]);
        auto chunk = std::make_unique<Chunk>(chunkX, chunkY);

        constexpr int expectedTileCount = Chunk::SIZE * Chunk::SIZE;
        for (int i = 0; i < expectedTileCount; i++)
        {
            const int type = std::stoi(parts[3 + i]);
            const int x = i % Chunk::SIZE;
            const int y = i / Chunk::SIZE;
            chunk->setTile(x, y, type);
        }

        world->addChunk(std::move(chunk));
    }
    else if (cmd == "UPDATE_TILE")
    {
        if (!world) return;

        //TODO: make proper worldX to tileX conversion function
        //coordinate conversions to get the tile the mouse is hovering over
        const int worldX = std::stoi(parts[1]);
        const int topDownWorldY = std::stoi(parts[2]);
        const int newTileType = std::stoi(parts[3]);
        const int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;
        const int bottomUpWorldY = worldHeightInTiles - 1 - topDownWorldY;
        const int chunkX = static_cast<int>(floor(static_cast<float>(worldX) / Chunk::SIZE));
        const int chunkY_BottomUp = static_cast<int>(floor(static_cast<float>(bottomUpWorldY) / Chunk::SIZE));
        const int fixedChunkY = chunkY_BottomUp;
        const int tileX = (worldX % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;
        const int tileY_BottomUp = (bottomUpWorldY % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;

        if (Chunk* chunk = world->getChunk(chunkX, fixedChunkY))
            chunk->setTile(tileX, tileY_BottomUp, newTileType);
    }
}

void Game::handleInput(const SDL_Event& e)
{
    if (localPlayerId == -1)
        return;

    //handle player movement
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
    {
        std::string action;
        switch (e.key.keysym.sym)
        {
            case SDLK_w: action = (e.type == SDL_KEYDOWN ? "UP_DOWN" : "UP_UP"); break;       //jump
            case SDLK_s: action = (e.type == SDL_KEYDOWN ? "DOWN_DOWN" : "DOWN_UP"); break;   //unused for now
            case SDLK_a: action = (e.type == SDL_KEYDOWN ? "LEFT_DOWN" : "LEFT_UP"); break;   //move left
            case SDLK_d: action = (e.type == SDL_KEYDOWN ? "RIGHT_DOWN" : "RIGHT_UP"); break; //move right
            default: return;
        }

        std::ostringstream oss;
        oss << "INPUT," << localPlayerId << "," << action;

        if (network)
            network->queueMessage(oss.str());
    }
    //handle player tile interaction
    else if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        int tileToSend = -1;
        if (e.button.button == SDL_BUTTON_LEFT)
            tileToSend = 0; //left click to break (placing air)
        else if (e.button.button == SDL_BUTTON_RIGHT)
            tileToSend = currentHeldItem; //right click to place (any tile)

        if (tileToSend == -1)
            return; //discard because value unchanged

        //get coordinates
        const int mouseScreenX = e.button.x;
        const int mouseScreenY = e.button.y;
        const int mouseWorldX = mouseScreenX - cameraX;
        const int mouseWorldY = mouseScreenY - cameraY;
        const int tileX = static_cast<int>(std::floor(static_cast<float>(mouseWorldX) / World::TILE_PX_SIZE));
        const int tileY = static_cast<int>(std::floor(static_cast<float>(mouseWorldY) / World::TILE_PX_SIZE));

        //conversion for sdl2 & java Y origin
        const int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;
        const int fixedTileY = worldHeightInTiles - 1 - tileY;

        std::ostringstream oss;
        oss << "SET_TILE," << tileX << "," << fixedTileY << "," << tileToSend;

        if (network)
            network->queueMessage(oss.str());
    }
    //handle block selection
    else if (e.type == SDL_MOUSEWHEEL)
    {
        auto index = std::find(tiles.begin(), tiles.end(),currentHeldItem);
        if (index == tiles.end())
            index = tiles.begin(); //reset if invalid

        int currentIndex = std::distance(tiles.begin(), index);
        if (e.wheel.y > 0)
            currentIndex = (currentIndex + 1) % tiles.size();
        else if (e.wheel.y < 0)
            currentIndex = (currentIndex - 1 + tiles.size()) % tiles.size();

        currentHeldItem = tiles[currentIndex];
    }
}

void Game::render(SDL_Renderer* renderer)
{
    const TextureManager& texManager = TextureManager::getInstance();
    std::string textureId;
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255); //basic sky backdrop (replace with png later)
    SDL_RenderClear(renderer);

    int winW, winH;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);

    cameraX = 0;
    cameraY = 0;

    //center the view on the local player
    if (players.count(localPlayerId))
    {
        auto& p = players[localPlayerId];
        int playerCenterX = static_cast<int>(p.x * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2);
        int playerCenterY = static_cast<int>(p.y * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2);

        cameraX = (winW / 2) - playerCenterX;
        cameraY = (winH / 2) - playerCenterY;
    }

    //render all chunks (add culling later)
    for (auto& [key, chunkPtr] : world->chunks)
    {
        Chunk* chunk = chunkPtr.get();
        const int chunkWorldX = chunk->chunkX * Chunk::SIZE * World::TILE_PX_SIZE;
        const int chunkWorldY = (World::WORLD_HEIGHT_IN_CHUNKS - 1 - chunk->chunkY) * Chunk::SIZE * World::TILE_PX_SIZE;

        for (int y = 0; y < Chunk::SIZE; ++y)
        {
            const int flippedY = Chunk::SIZE - 1 - y;
            for (int x = 0; x < Chunk::SIZE; ++x)
            {
                auto& [type] = chunk->tiles[flippedY][x];
                const int worldX = chunkWorldX + x * World::TILE_PX_SIZE + cameraX;
                const int worldY = chunkWorldY + y * World::TILE_PX_SIZE + cameraY;

                //TODO: change from SDL_Rects to textures, reduces draw calls and more fps hopefully
                if (type == 0) continue; //air

                switch (type)
                {
                    case 1: textureId = "grass"; break;
                    case 2: textureId = "dirt"; break;
                    case 3: textureId = "stone"; break;
                    case 4: textureId = "wood_log"; break;
                    case 5: textureId = "wood_plank"; break;
                    default: //if tile doesnt have a texture or is unknown just draw pink square
                        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                        SDL_Rect missingTile { worldX, worldY, World::TILE_PX_SIZE, World::TILE_PX_SIZE };
                        SDL_RenderFillRect(renderer, &missingTile);
                        continue;
                }

                texManager.draw(renderer, textureId, worldX, worldY, World::TILE_PX_SIZE, World::TILE_PX_SIZE);
            }
        }
    }

    //render all players
    for (auto& [id, p] : players)
    {
        SDL_Rect rect
        {
            static_cast<int>(p.x * World::TILE_PX_SIZE + cameraX),
            static_cast<int>(p.y * World::TILE_PX_SIZE + cameraY),
            World::TILE_PX_SIZE,      //player width in pixels (1 tile wide)
            World::TILE_PX_SIZE * 2   //player height in pixels (2 tiles tall)
        };

        if (p.isLocal) //draw local client as blue
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        else //everyone else draw yellow
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

        SDL_RenderFillRect(renderer, &rect);
    }

    //render UI
    std::string blockName;
    switch (currentHeldItem)
    {
        case 1: textureId = "grass"; blockName = "Grass"; break;
        case 2: textureId = "dirt"; blockName = "Dirt"; break;
        case 3: textureId = "stone"; blockName = "Stone"; break;
        case 4: textureId = "wood_log"; blockName = "Wood Log"; break;
        case 5: textureId = "wood_plank"; blockName = "Wood Plank"; break;
    }

    const int iconSize = 32;
    const int margin = 10;
    if (currentHeldItem != 0)
        texManager.draw(renderer, textureId, margin, margin, iconSize, iconSize);

    SDL_Color textColor = { 255, 255, 255, 255 };
    std::string hotbarText = "Selected: " + blockName;
    drawText(renderer, hotbarText, margin + iconSize + 5, margin + 8, textColor);

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
