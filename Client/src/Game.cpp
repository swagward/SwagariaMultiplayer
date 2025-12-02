#include "../include/Game.h"
#include "../include/Network.h"
#include "../include/TextureManager.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>

class TextureManager;

Game::Game() : camera(800, 600)
{
    std::cout << "[CLIENT] Started!" << std::endl;

    if (TTF_Init() == -1)
        std::cerr << "[SDL_TTF] Failed to initialize: " << TTF_GetError() << std::endl;
    else
    {
        font = TTF_OpenFont("assets/fonts/Andy Bold.ttf", 24);
        if (!font)
            std::cerr << "[SDL_TTF] Failed to load font: " << TTF_GetError() << std::endl;
        else
            std::cout << "[SDL_TTF] Font loaded successfully.\n";
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

    std::cout << "[CLIENT] Closed." << std::endl;
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
        std::cout << "[SERVER] Assigned id " << localPlayerId << std::endl;
    }
    else if (cmd == "SPAWN")
    {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, id == localPlayerId, "Player" + std::to_string(id) };
        std::cout << "[SERVER] Spawned player " << id << " at " << x << "," << y << std::endl;
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
        std::cout << "[SERVER] Player " << id << " joined\n";
    }
    else if (cmd == "PLAYER_LEAVE")
    {
        const int id = std::stoi(parts[1]);
        players.erase(id);
        std::cout << "[SERVER] Player " << id << " left\n";
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

        //TODO: make proper world to tile conversion function
        //coordinate conversions to get the tile the mouse is hovering over
        const int worldX = std::stoi(parts[1]);
        const int topDownWorldY = std::stoi(parts[2]);
        const int newTileType = std::stoi(parts[3]);
        constexpr int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;
        const int bottomUpWorldY = worldHeightInTiles - 1 - topDownWorldY;
        const int chunkX = static_cast<int>(std::floor(static_cast<float>(worldX) / Chunk::SIZE));
        const int chunkY_BottomUp = static_cast<int>(std::floor(static_cast<float>(bottomUpWorldY) / Chunk::SIZE));
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

    //freecam toggle
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1 && !e.key.repeat)
    {
        isFreecamActive = !isFreecamActive;
        if (isFreecamActive)

            //when active, anchor the camera's target to its exact screen position on the x/y
            camera.setScreenOffsetTarget(camera.getPreciseX(), camera.getPreciseY());
    }

    //handle movement
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
    {
        bool isKeyDown = (e.type == SDL_KEYDOWN);

        //player movement
        if (!isFreecamActive)
        {
            std::string action;
            switch (e.key.keysym.sym)
            {
                case SDLK_w: action = (isKeyDown ? "UP_DOWN" : "UP_UP"); break;       //jump
                case SDLK_s: action = (isKeyDown ? "DOWN_DOWN" : "DOWN_UP"); break;   //unused for now
                case SDLK_a: action = (isKeyDown ? "LEFT_DOWN" : "LEFT_UP"); break;   //move left
                case SDLK_d: action = (isKeyDown ? "RIGHT_DOWN" : "RIGHT_UP"); break; //move right
                default: break;
            }

            if (!action.empty())
            {
                std::ostringstream oss;
                oss << "INPUT," << localPlayerId << "," << action;

                if (network)
                    network->queueMessage(oss.str());
            }
        }

        //freecam movement
        switch (e.key.keysym.sym)
        {
            case SDLK_w: keysHeld[SDLK_w] = isKeyDown; break;
            case SDLK_s: keysHeld[SDLK_s] = isKeyDown; break;
            case SDLK_a: keysHeld[SDLK_a] = isKeyDown; break;
            case SDLK_d: keysHeld[SDLK_d] = isKeyDown; break;
            default: ;
        }
    }

    //tile interaction (when freecam is disabled)
    else if (e.type == SDL_MOUSEBUTTONDOWN && !isFreecamActive)
    {
        int tileToSend = -1;
        if (e.button.button == SDL_BUTTON_LEFT)         //left click to break (place air)
            tileToSend = 0;
        else if (e.button.button == SDL_BUTTON_RIGHT)   //right click to place (any tile)
            tileToSend = currentHeldItem;

        if (tileToSend == -1)
            return; //discard because value unchanged

        //get specific tile in world with zoom accounted for
        const float zoom = camera.getZoom();
        const float mouseWorldOffsetScaledX = static_cast<float>(e.button.x) - camera.getPreciseX();
        const float mouseWorldOffsetScaledY = static_cast<float>(e.button.y) - camera.getPreciseY();
        const float mouseWorldX = mouseWorldOffsetScaledX / zoom;
        const float mouseWorldY = mouseWorldOffsetScaledY / zoom;
        const int tileX = static_cast<int>(std::floor(mouseWorldX / World::TILE_PX_SIZE));
        const int tileY = static_cast<int>(std::floor(mouseWorldY / World::TILE_PX_SIZE));

        //conversion for sdl2 & java Y origin
        constexpr int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;
        const int fixedTileY = worldHeightInTiles - 1 - tileY;

        std::ostringstream oss;
        oss << "SET_TILE," << tileX << "," << fixedTileY << "," << tileToSend;

        if (network)
            network->queueMessage(oss.str());
    }
    //scrollwheel cycle for tiles/speed/zoom
    else if (e.type == SDL_MOUSEWHEEL)
    {
        if (SDL_GetModState() & KMOD_CTRL)
            camera.handleZoom(e.wheel.y);
        else if (isFreecamActive)
        {
            //scroll changes freecam move speed
            float newSpeed = freecamSpeed;    //increase
            if (e.wheel.y > 0)
                newSpeed += freecamSpeedStep; //decrease
            else if (e.wheel.y < 0)
                newSpeed -= freecamSpeedStep;

            freecamSpeed = std::clamp(newSpeed, minFreecamSpeed, maxFreecamSpeed);
        }
        else
        {
            //cycle hotbar
            auto index = std::find(tiles.begin(), tiles.end(),currentHeldItem);
            if (index == tiles.end())
                index = tiles.begin(); //reset if index is greater than tiles array size

            int currentIndex = std::distance(tiles.begin(), index);
            if (e.wheel.y > 0)
                currentIndex = (currentIndex + 1) % tiles.size();
            else if (e.wheel.y < 0)
                currentIndex = (currentIndex - 1 + tiles.size()) % tiles.size();

            currentHeldItem = tiles[currentIndex];
        }
    }
}

void Game::render(SDL_Renderer* renderer)
{
    const TextureManager& texManager = TextureManager::getInstance();
    std::string textureId;
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255); //basic sky backdrop (replace with parallax png later)
    SDL_RenderClear(renderer);

    if (!world) return;

    int winW, winH;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);

    const float cameraX = camera.getPreciseX();
    const float cameraY = camera.getPreciseY();
    const float zoom = camera.getZoom();

    //get the unscaled camera viewport
    const float viewLeftPix = (0.0f - cameraX) / zoom;
    const float viewTopPix = (0.0f - cameraY) / zoom;
    const float viewRightPix = (static_cast<float>(winW) - cameraX) / zoom;
    const float viewBottomPix = (static_cast<float>(winH) - cameraY) / zoom;

    float cullLeftPix, cullTopPix, cullRightPix, cullBottomPix;

    //define the culling bounds based on players position and camera zoom
    if (localPlayerId != -1 && players.count(localPlayerId))
    {
        const auto& p = players[localPlayerId];

        const int playerTileX = static_cast<int>(p.x);
        const int playerTileY_Up = static_cast<int>(p.y);
        constexpr int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;
        const int playerTileY_Down = worldHeightInTiles - 1 - playerTileY_Up;
        constexpr float TILE_PX_SIZE = static_cast<float>(World::TILE_PX_SIZE);
        const float playerCentreX_Pix = (static_cast<float>(playerTileX) + 0.5f) * TILE_PX_SIZE;
        const float playerCentreY_Pix = (static_cast<float>(playerTileY_Down) + 0.5f) * TILE_PX_SIZE;
        const float CULL_RADIUS_PX = 2500.0f;

        const float playerCullLeftPix = playerCentreX_Pix - CULL_RADIUS_PX;
        const float playerCullRightPix = playerCentreX_Pix + CULL_RADIUS_PX;
        const float playerCullTopPix = playerCentreY_Pix - CULL_RADIUS_PX;
        const float playerCullBottomPix = playerCentreY_Pix + CULL_RADIUS_PX;

        //the final rendering bounds are the INTERSECTION of the Camera View and the Player Culling Area
        //this means:
        //A) only draw what's on screen (cullLeftPix > viewLeftPix)
        //B) only draw what's near the player (cullLeftPix > playerCullLeftPix)
        cullLeftPix = std::max(viewLeftPix, playerCullLeftPix);
        cullTopPix = std::max(viewTopPix, playerCullTopPix);
        cullRightPix = std::min(viewRightPix, playerCullRightPix);
        cullBottomPix = std::min(viewBottomPix, playerCullBottomPix);
    }
    else
    {
        //if no player is spawned then just fall back to simple camera frustum culling idk
        cullLeftPix = viewLeftPix;
        cullTopPix = viewTopPix;
        cullRightPix = viewRightPix;
        cullBottomPix = viewBottomPix;
    }


    //get visible chunks based on the culling bounds
    constexpr float TILE_PX_SIZE = static_cast<float>(World::TILE_PX_SIZE);
    constexpr float CHUNK_SIZE_PX = static_cast<float>(Chunk::SIZE * World::TILE_PX_SIZE);
    int minChunkX = static_cast<int>(std::floor(cullLeftPix / CHUNK_SIZE_PX));
    int maxChunkX = static_cast<int>(std::ceil(cullRightPix / CHUNK_SIZE_PX)) - 1;
    int minChunkY_Down = static_cast<int>(std::floor(cullTopPix / CHUNK_SIZE_PX));
    int maxChunkY_Down = static_cast<int>(std::ceil(cullBottomPix / CHUNK_SIZE_PX)) - 1;
    const int worldChunksX = World::WORLD_WIDTH_IN_CHUNKS;
    const int worldChunksY = World::WORLD_HEIGHT_IN_CHUNKS;

    int startChunkX = std::clamp(minChunkX, 0, worldChunksX - 1);
    int endChunkX = std::clamp(maxChunkX, 0, worldChunksX - 1);
    int startChunkY_Down = std::clamp(minChunkY_Down, 0, worldChunksY - 1);
    int endChunkY_Down = std::clamp(maxChunkY_Down, 0, worldChunksY - 1);

    //iterate through visible chunks
    for (int cx = startChunkX; cx <= endChunkX; ++cx)
    {
        //convert the render y index
        for (int cy_Down = startChunkY_Down; cy_Down <= endChunkY_Down; ++cy_Down)
        {
            const int chunkY_BottomUp = worldChunksY - 1 - cy_Down;
            Chunk* chunk = world->getChunk(cx, chunkY_BottomUp);
            if (!chunk) continue;

            const float chunkWorldX = static_cast<float>(cx * CHUNK_SIZE_PX);
            const float chunkWorldY = static_cast<float>(cy_Down * CHUNK_SIZE_PX);

            //clamp x and y bounds
            int startTileX = std::max(0, static_cast<int>(std::floor((cullLeftPix - chunkWorldX) / TILE_PX_SIZE)));
            int startTileY_Down = std::max(0, static_cast<int>(std::floor((cullTopPix - chunkWorldY) / TILE_PX_SIZE)));
            int endTileX = std::min(Chunk::SIZE - 1, static_cast<int>(std::ceil((cullRightPix - chunkWorldX) / TILE_PX_SIZE)) - 1);
            int endTileY_Down = std::min(Chunk::SIZE - 1, static_cast<int>(std::ceil((cullBottomPix - chunkWorldY) / TILE_PX_SIZE)) - 1);

            //iterate through the visible tiles now
            for (int y_Down = startTileY_Down; y_Down <= endTileY_Down; ++y_Down)
            {
                //convert top-down Y index to bottom-up
                const int y_Storage = Chunk::SIZE - 1 - y_Down;

                for (int x_Local = startTileX; x_Local <= endTileX; ++x_Local)
                {
                    auto& [type] = chunk->tiles[y_Storage][x_Local];

                    //render world properly based on zoom + remove seams when zoom is floating point value
                    const float currentUnscaledWorldX = chunkWorldX + x_Local * TILE_PX_SIZE;
                    const float currentUnscaledWorldY = chunkWorldY + y_Down * TILE_PX_SIZE;
                    const float nextUnscaledWorldX = chunkWorldX + (x_Local + 1) * TILE_PX_SIZE;
                    const float nextUnscaledWorldY = chunkWorldY + (y_Down + 1) * TILE_PX_SIZE;

                    const int currentScreenX = static_cast<int>(std::floor(currentUnscaledWorldX * zoom + cameraX));
                    const int currentScreenY = static_cast<int>(std::floor(currentUnscaledWorldY * zoom + cameraY));
                    const int nextScreenX = static_cast<int>(std::floor(nextUnscaledWorldX * zoom + cameraX));
                    const int nextScreenY = static_cast<int>(std::floor(nextUnscaledWorldY * zoom + cameraY));

                    const int scaledTileWidth = nextScreenX - currentScreenX;
                    const int scaledTileHeight = nextScreenY - currentScreenY;

                    if (type == 0) continue; //air
                    if (scaledTileWidth <= 0 || scaledTileHeight <= 0) continue;

                    switch (type)
                    {
                    case 1: textureId = "grass"; break;
                    case 2: textureId = "dirt"; break;
                    case 3: textureId = "stone"; break;
                    case 4: textureId = "wood_log"; break;
                    case 5: textureId = "torch"; break;
                    case 6: textureId = "wood_plank"; break;
                    case 7: textureId = "wood_plank_bg"; break;
                    case 8: textureId = "stone_bg"; break;
                    default: textureId = "missing_texture"; break;
                    }

                    texManager.draw(renderer, textureId, currentScreenX, currentScreenY, scaledTileWidth, scaledTileHeight);
                }
            }
        }
    }

    const int originalScaledTilePxSize = static_cast<int>(std::round(World::TILE_PX_SIZE * zoom));

    //render all players
    for (auto& [id, p] : players)
    {
        const float playerWorldX = p.x * World::TILE_PX_SIZE; //unscaled float for X/Y (tile coordinate * tile size)
        const float playerWorldY = p.y * World::TILE_PX_SIZE;
        const int playerScreenX = static_cast<int>(std::floor(playerWorldX * zoom + cameraX));
        const int playerScreenY = static_cast<int>(std::floor(playerWorldY * zoom + cameraY));

        SDL_Rect rect
        {
            playerScreenX,
            playerScreenY,
            originalScaledTilePxSize,       //player width in scaled pixels (1 tile wide)
            originalScaledTilePxSize * 2    //player height in scaled pixels (2 tiles tall)
        };

        if (p.isLocal)
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);   //draw local client as blue
        else
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); //everyone else draw yellow

        SDL_RenderFillRect(renderer, &rect);
    }

    //render UI
    constexpr int margin = 10;
    SDL_Color textColor = { 255, 255, 255, 255 };

    if (!isFreecamActive)
    {
        constexpr int iconSize = 32;
        std::string blockName;

        switch (currentHeldItem)
        {
        case 1: textureId = "grass"; blockName = "Grass"; break;
        case 2: textureId = "dirt"; blockName = "Dirt"; break;
        case 3: textureId = "stone"; blockName = "Stone"; break;
        case 4: textureId = "wood_log"; blockName = "Wood Log"; break;
        case 5: textureId = "torch"; blockName = "Torch"; break;
        case 6: textureId = "wood_plank"; blockName = "Wood Plank"; break;
        case 7: textureId = "wood_plank_bg"; blockName = "Wood Wall"; break;
        case 8: textureId = "stone_bg"; blockName = "Stone Wall"; break;
        default: textureId = "missing_texture"; blockName = "Invalid Texture!"; break;
        }

        if (currentHeldItem != 0)
            texManager.draw(renderer, textureId, margin, margin, iconSize, iconSize);

        std::string hotbarText = "Selected: " + blockName;
        drawText(renderer, hotbarText, margin + iconSize + 5, margin + 8, textColor);
    }

    //zoom text set to 2 decimal places
    std::ostringstream zoomOss;
    zoomOss.precision(2);
    zoomOss << std::fixed << zoom;
    std::string zoomText = "Zoom: x" + zoomOss.str();
    drawText(renderer, zoomText, winW - 125, margin + 8, textColor);

    if (isFreecamActive)
    {
        SDL_Color freecamColor = { 255, 255, 0, 255 }; //yellow text so it stands out more
        std::string freecamText = "FREECAM ACTIVE (Culling fixed on Player)";
        drawText(renderer, freecamText, winW / 2 - 375, margin + 8, freecamColor);

        std::ostringstream speedOss;
        speedOss.precision(1);
        speedOss << std::fixed << freecamSpeed;
        std::string speedText = "Speed: x: " + speedOss.str();
        drawText(renderer, speedText, winW / 2 - 200, margin + 8 + 25, freecamColor);
    }


    SDL_RenderPresent(renderer);
}

void Game::update()
{
    if (isFreecamActive)
    {
        float currentScreenX = camera.getPreciseX();
        float currentScreenY = camera.getPreciseY();
        const float speed = freecamSpeed;

        if (keysHeld[SDLK_a])
            currentScreenX += speed;
        if (keysHeld[SDLK_d])
            currentScreenX -= speed;
        if (keysHeld[SDLK_w])
            currentScreenY += speed;
        if (keysHeld[SDLK_s])
            currentScreenY -= speed;

        camera.setScreenOffsetTarget(currentScreenX, currentScreenY);
    }
    else if (players.count(localPlayerId))
    {
        //p.x and p.y are tile coordinates. convert them to pixel center coordinates.
        const auto& p = players[localPlayerId];
        const float playerCentreX = p.x * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2.0f;
        const float playerCentreY = p.y * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2.0f;

        camera.setTarget(playerCentreX, playerCentreY);
    }

    camera.update();
}

void Game::drawText(SDL_Renderer* renderer, const std::string& text, const int x, const int y, const SDL_Color color) const
{
    if (!font) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    const SDL_Rect dstRect = { x, y, surface->w, surface->h };

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}