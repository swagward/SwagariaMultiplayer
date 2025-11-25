#include "../include/Game.h"
#include "../include/Network.h"
#include <algorithm>
#include <cmath>
#include <sstream>

#include "../include/TextureManager.h"


class TextureManager;

Game::Game() : camera(800, 600)
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

        //TODO: make proper world to tile conversion function
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

        // Use the float camera position for accurate transformation reversal
        const float zoom = camera.getZoom();

        // 1. Convert Screen Mouse Pos (e.button.x/y) to Scaled World Offset (relative to 0,0)
        // Note: Using getPreciseX/Y as defined in your provided code
        const float mouseWorldOffsetScaledX = static_cast<float>(e.button.x) - camera.getPreciseX();
        const float mouseWorldOffsetScaledY = static_cast<float>(e.button.y) - camera.getPreciseY();

        // 2. Convert Scaled World Offset to UNscaled World Pixel Coordinates
        const float mouseWorldX = mouseWorldOffsetScaledX / zoom;
        const float mouseWorldY = mouseWorldOffsetScaledY / zoom;

        // 3. Convert UNscaled World Pixel Coordinates to UNscaled World Tile Coordinates
        const int tileX = static_cast<int>(std::floor(mouseWorldX / World::TILE_PX_SIZE));
        const int tileY = static_cast<int>(std::floor(mouseWorldY / World::TILE_PX_SIZE));

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
        if (SDL_GetModState() & KMOD_CTRL)
            camera.handleZoom(e.wheel.y);
        else
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
}

void Game::render(SDL_Renderer* renderer)
{
    const TextureManager& texManager = TextureManager::getInstance();
    std::string textureId;
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255); //basic sky backdrop (replace with png later)
    SDL_RenderClear(renderer);

    int winW, winH;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);

    // Use float camera position
    const float cameraX = camera.getPreciseX();
    const float cameraY = camera.getPreciseY();
    const float zoom = camera.getZoom();

    // The key to eliminating seams: calculate size based on the rounded difference
    // between tile start coordinates. This ensures the start of tile (x+1) is
    // the end of tile (x), pixel-perfectly.
    const float TILE_SIZE_PX = static_cast<float>(World::TILE_PX_SIZE);

    //render all chunks (add culling later)
    for (auto& [key, chunkPtr] : world->chunks)
    {
        Chunk* chunk = chunkPtr.get();
        const float chunkWorldX = static_cast<float>(chunk->chunkX * Chunk::SIZE * World::TILE_PX_SIZE);
        const float chunkWorldY = static_cast<float>((World::WORLD_HEIGHT_IN_CHUNKS - 1 - chunk->chunkY) * Chunk::SIZE * World::TILE_PX_SIZE);

        for (int y = 0; y < Chunk::SIZE; ++y)
        {
            const int flippedY = Chunk::SIZE - 1 - y;
            for (int x = 0; x < Chunk::SIZE; ++x)
            {
                auto& [type] = chunk->tiles[flippedY][x];

                // 1. Calculate unscaled world position for current tile (start)
                const float currentUnscaledWorldX = chunkWorldX + x * TILE_SIZE_PX;
                const float currentUnscaledWorldY = chunkWorldY + y * TILE_SIZE_PX;

                // 2. Calculate unscaled world position for next tile (end)
                const float nextUnscaledWorldX = chunkWorldX + (x + 1) * TILE_SIZE_PX;
                const float nextUnscaledWorldY = chunkWorldY + (y + 1) * TILE_SIZE_PX;

                // 3. Calculate rounded screen positions for current tile start
                // Use std::floor() for the top-left corner coordinates for consistency.
                const int currentScreenX = static_cast<int>(std::floor(currentUnscaledWorldX * zoom + cameraX));
                const int currentScreenY = static_cast<int>(std::floor(currentUnscaledWorldY * zoom + cameraY));

                // 4. Calculate rounded screen positions for next tile start (i.e., current tile end)
                const int nextScreenX = static_cast<int>(std::floor(nextUnscaledWorldX * zoom + cameraX));
                const int nextScreenY = static_cast<int>(std::floor(nextUnscaledWorldY * zoom + cameraY));

                // 5. Calculate the size of the tile based on the difference (differential size)
                const int scaledTileWidth = nextScreenX - currentScreenX;
                const int scaledTileHeight = nextScreenY - currentScreenY;

                if (type == 0) continue; //air

                // Check for invalid size (should not happen if math is right)
                if (scaledTileWidth <= 0 || scaledTileHeight <= 0) continue;

                switch (type)
                {
                    case 1: textureId = "grass"; break;
                    case 2: textureId = "dirt"; break;
                    case 3: textureId = "stone"; break;
                    case 4: textureId = "wood_log"; break;
                    case 5: textureId = "wood_plank"; break;
                    default: //if tile doesn't have a texture or is unknown just draw pink square
                        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                        SDL_Rect missingTile { currentScreenX, currentScreenY, scaledTileWidth, scaledTileHeight };
                        SDL_RenderFillRect(renderer, &missingTile);
                        continue;
                }

                texManager.draw(renderer, textureId, currentScreenX, currentScreenY, scaledTileWidth, scaledTileHeight);
            }
        }
    }

    // Since player positioning must be pixel-perfect relative to the tiles,
    // we must also update the player rendering logic to use the differential position calculation.
    // We will use the original scaled size calculation for the player size, but ensure the position is floored/rounded.
    const int originalScaledTilePxSize = static_cast<int>(std::round(TILE_SIZE_PX * zoom));

    //render all players
    for (auto& [id, p] : players)
    {
        const float playerWorldX = p.x * TILE_SIZE_PX; //unscaled float for X/Y (tile coordinate * tile size)
        const float playerWorldY = p.y * TILE_SIZE_PX;

        // Apply zoom, float camera offset, and then FLOOR/ROUND for integer pixel
        const int playerScreenX = static_cast<int>(std::floor(playerWorldX * zoom + cameraX));
        const int playerScreenY = static_cast<int>(std::floor(playerWorldY * zoom + cameraY));

        SDL_Rect rect
        {
            playerScreenX,
            playerScreenY,
            originalScaledTilePxSize,       // player width in scaled pixels (1 tile wide)
            originalScaledTilePxSize * 2    // player height in scaled pixels (2 tiles tall)
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

    // Minor fix to zoom display to ensure it shows two decimal places
    std::ostringstream zoomOss;
    zoomOss.precision(2);
    zoomOss << std::fixed << zoom;
    std::string zoomText = "Zoom: x" + zoomOss.str();
    drawText(renderer, zoomText, winW - 150, margin + 8, textColor);

    SDL_RenderPresent(renderer);
}

void Game::update()
{
    if (players.count(localPlayerId))
    {
        const auto& p = players[localPlayerId];
        // Note: p.x and p.y are tile coordinates. Convert them to pixel center coordinates.
        // We must use floats here to correctly calculate the center point.
        const float playerCentreX = p.x * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2.0f;
        const float playerCentreY = p.y * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2.0f;

        camera.setTarget(playerCentreX, playerCentreY);
    }

    camera.update();
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