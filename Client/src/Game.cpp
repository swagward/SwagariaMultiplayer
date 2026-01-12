#include "../include/Game.h"
#include "../include/Network.h"
#include "../include/TextureManager.h"
#include "../include/ItemRegistry.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>

#include "SDL_mixer.h"
#include "../include/AudioManager.h"

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
    }

    world = std::make_unique<World>();
    particleManager = std::make_unique<ParticleManager>();
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
        localPlayerId = std::stoi(parts[1]);
    else if (cmd == "SPAWN")
    {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, x, y, id == localPlayerId, "Player" + std::to_string(id) };
    }
    else if (cmd == "ITEM_DEF_SYNC")
    {
        ItemRegistry::getInstance().clear();

        //format:
        //ITEM_DEF_SYNC,ID:Name:MaxStackSize:Type:Prop1:Prop2|ID:Name:MaxStackSize:Type:Prop1:Prop2|...
        //TILE: ID:Name:MaxStackSize:T:TileTypeID
        //TOOL: ID:Name:MaxStackSize:R:ToolType:Damage

        if (parts.size() < 2) return;
        std::istringstream defsSs(parts[1]);
        std::string defString;

        while (std::getline(defsSs, defString, '|'))
        {
            if (defString.empty()) continue;

            std::istringstream itemSs(defString);
            std::string itemPart;
            std::vector<std::string> itemProps;

            // re-parse message using ":" to identify properties
            while (std::getline(itemSs, itemPart, ':'))
                itemProps.push_back(itemPart);

            //needs: ID:Name:MaxStackSize:Type
            if (itemProps.size() < 4) continue;

            ItemDefinition itemDef;
            itemDef.id = std::stoi(itemProps[0]);
            itemDef.name = itemProps[1];
            itemDef.maxStack = std::stoi(itemProps[2]);

            if (std::string typeCode = itemProps[3]; typeCode == "T")
            {
                if (itemProps.size() < 5) continue;
                itemDef.isTile = true;
                itemDef.tileTypeID = std::stoi(itemProps[4]);
            }
            else if (typeCode == "R")
            {
                if (itemProps.size() < 6) continue;
                itemDef.isTile = false;
                itemDef.tileTypeID = 0;

                const std::string& toolTypeName = itemProps[4];
                const int damage = std::stoi(itemProps[5]);
            }
            else
            {
                 itemDef.isTile = false;
                 itemDef.tileTypeID = 0;
            }

            // generate textureID from name
            std::string name = itemDef.name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            std::replace(name.begin(), name.end(), ' ', '_');
            itemDef.textureID = name;

            ItemRegistry::getInstance().addDefinition(itemDef);
        }
    }
    else if (cmd == "PLAYER_MOVE")
    {
        const int id = std::stoi(parts[1]);
        const float targetX = std::stof(parts[2]);
        const float targetY = std::stof(parts[3]);

        if (players.count(id))
        {
            //store targetX/Y for smoothing
            players[id].targetX = targetX;
            players[id].targetY = targetY;

            //teleport in worst case scenario
            if (float dist = std::sqrt(std::pow(players[id].visualX - targetX, 2) + std::pow(players[id].visualY - targetY, 2)); dist > 5.0f)
            {
                players[id].visualX = targetX;
                players[id].visualY = targetY;
            }
        }
    }
    else if (cmd == "PLAYER_JOIN")
    {
        const int id = std::stoi(parts[1]);
        const float x = std::stof(parts[2]);
        const float y = std::stof(parts[3]);
        players[id] = { id, x, y, x, y, false, "Player" + std::to_string(id) };
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

        constexpr int tilesPerLayer = Chunk::SIZE * Chunk::SIZE;
        int tileIndexOffset = 3; //skip CHUNK_DATA, chunkX and chunkY

        for (int layer = 0; layer < TileLayer::NUM_LAYERS; layer++)
        {
            for (int i = 0; i < tilesPerLayer; i++)
            {
                if (tileIndexOffset + i >= parts.size())
                {
                    std::cerr << "[ERROR] CHUNK_DATA message truncated. Missing tile data.\n";
                    return;
                }

                const int type = std::stoi(parts[tileIndexOffset + i]);
                const int x = i % Chunk::SIZE;
                const int y = i / Chunk::SIZE;

                chunk->setTile(x, y, layer, type);
            }
            tileIndexOffset += tilesPerLayer;
        }

        world->addChunk(std::move(chunk));
    }
    else if (cmd == "UPDATE_TILE")
    {
        if (!world) return;
        if (parts.size() < 5) return;

        const int worldX = std::stoi(parts[1]);
        const int topDownWorldY = std::stoi(parts[2]);
        const int newTileType = std::stoi(parts[3]);
        const int layerIndex = std::stoi(parts[4]);

        if (layerIndex < 0 || layerIndex >= TileLayer::NUM_LAYERS) return;

        constexpr int worldWidthInTiles = World::WORLD_WIDTH_IN_CHUNKS * Chunk::SIZE;
        constexpr int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;

        //stop out of bounds
        if (worldX < 0 || worldX >= worldWidthInTiles || topDownWorldY < 0 || topDownWorldY >= worldHeightInTiles)
            return;

        const int bottomUpWorldY = worldHeightInTiles - 1 - topDownWorldY;
        const int chunkX = static_cast<int>(std::floor(static_cast<float>(worldX) / Chunk::SIZE));
        const int chunkY_BottomUp = static_cast<int>(std::floor(static_cast<float>(bottomUpWorldY) / Chunk::SIZE));

        //another oob check
        if (chunkX < 0 || chunkX >= World::WORLD_WIDTH_IN_CHUNKS || chunkY_BottomUp < 0 || chunkY_BottomUp >= World::WORLD_HEIGHT_IN_CHUNKS)
            return;

        const int tileX = (worldX % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;
        const int tileY_BottomUp = (bottomUpWorldY % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;

        if (newTileType == 0)
        {
            if (Chunk* chunk = world->getChunk(chunkX, chunkY_BottomUp))
                if (int prevBlock = chunk->getTile(tileX, tileY_BottomUp, layerIndex).type; prevBlock != 0)
                    particleManager->spawnEffect(static_cast<float>(worldX), static_cast<float>(topDownWorldY), getTextureIDFromType(prevBlock));
            AudioManager::getInstance().playSFX("block_break");
        }

        if (Chunk* chunk = world->getChunk(chunkX, chunkY_BottomUp))
            chunk->setTile(tileX, tileY_BottomUp, layerIndex, newTileType);
    }
    else if (cmd == "INV_UPDATE")
    {
        //format: INV_UPDATE,playerID,slotIndex,itemID,quantity
        if (parts.size() < 5) return;
        const int slotIndex = std::stoi(parts[2]);
        const int itemID = std::stoi(parts[3]);
        const int quantity = std::stoi(parts[4]);

        inventory.updateSlot(slotIndex, itemID, quantity);
        if (ItemRegistry::getInstance().getDefinition(itemID).id == 0 && itemID != 0) {
            std::cerr << "[WARNING] Received INV_UPDATE for unknown item ID: " << itemID
                      << " in slot " << slotIndex << ". Check if ITEM_DEF_SYNC ran first.\n";
        }
    }
    else if (cmd == "INV_SYNC")
    {
        //TODO: hardcoded because im lazy | change if java inventory size changes in the future x
        constexpr int TOTAL_SLOTS = 40;
        for (int i = 0; i < TOTAL_SLOTS; i++)
        {
            if (const int partIndex = 2 + (i * 2); partIndex + 1 < parts.size())
            {
                try
                {
                    const int itemID = std::stoi(parts[partIndex]);
                    const int quantity = std::stoi(parts[partIndex + 1]);
                    inventory.updateSlot(i, itemID, quantity);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "[ERROR] Failed to parse INV_SYNC data for slot " << i << ": " << e.what() << std::endl;
                    return; //if one slot is corrupted more might be so just stop
                }
            }
            else //message is shorter than expected
            {
                std::cerr << "[ERROR] INV_SYNC message truncated, stopping load at slot: " << i << std::endl;
                break;
            }
        }
    }
    else std::cerr << "[ERROR] Unknown command: " << cmd << std::endl;
}

void Game::handleInput(const SDL_Event& e)
{
    if (localPlayerId == -1)
        return;
    //inventory toggle
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e && !e.key.repeat)
    {
        isInventoryOpen = !isInventoryOpen;
        return;
    }
    //freecam toggle
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1 && !e.key.repeat)
    {
        isFreecamActive = !isFreecamActive;
        if (isFreecamActive)

            //when active, anchor the camera's target to its exact screen position on the x/y
            camera.setScreenOffsetTarget(camera.getPreciseX(), camera.getPreciseY());
    }
    //inventory interaction
    if (isInventoryOpen)
    {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            int winW, winH;
            if (SDL_Renderer* renderer = SDL_GetRenderer(SDL_GetWindowFromID(1)))
                SDL_GetRendererOutputSize(renderer, &winW, &winH);
            else return;

            if (const int targetSlotIndex = getSlotIndexAt(e.button.x, e.button.y, winW, winH); targetSlotIndex != -1)
            {
                ItemSlot& targetSlot = inventory.slots[targetSlotIndex];
                if (ItemSlot& heldItem = inventory.mouseHeldItem; !heldItem.isEmpty())
                {
                    //merge attempt
                    if (!targetSlot.isEmpty() && heldItem.itemID == targetSlot.itemID)
                    {
                        const auto& def = ItemRegistry::getInstance().getDefinition(heldItem.itemID);
                        const int maxStack = def.maxStack;

                        int canAdd = maxStack - targetSlot.quantity;
                        if (int transfer = std::min(heldItem.quantity, canAdd); transfer > 0)
                        {
                            targetSlot.quantity += transfer;
                            heldItem.quantity -= transfer;

                            if (heldItem.quantity <= 0)
                                heldItem = { 0, 0 };
                        }
                    }
                    else
                        //handles dropping into empty slot/swapping two different stacks
                        std::swap(targetSlot, heldItem);
                }
                else if (!targetSlot.isEmpty())
                    std::swap(targetSlot, heldItem);


                //tell server it needs to rearrange the items otherwise big problems (items are only a visual update 🤬)
                if (network)
                {
                    std::ostringstream oss;
                    oss << "INV_MOVE_ITEM," << targetSlotIndex << "," << targetSlot.itemID << "," << targetSlot.quantity;
                    network->queueMessage(oss.str());
                }
            }
        }
        return;
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
        const int slotIndex = inventory.selectedHotbarIndex;

        //get target coords
        const float zoom = camera.getZoom();
        const float mouseWorldOffsetX = static_cast<float>(e.button.x) - camera.getPreciseX();
        const float mouseWorldOffsetY = static_cast<float>(e.button.y) - camera.getPreciseY();
        const float mouseWorldX = mouseWorldOffsetX / zoom;
        const float mouseWorldY = mouseWorldOffsetY / zoom;

        //convert pixel to tile
        const int tileX = static_cast<int>(std::floor(mouseWorldX / World::TILE_PX_SIZE));
        const int tileY = static_cast<int>(std::floor(mouseWorldY / World::TILE_PX_SIZE));

        //conversion for sdl2 (top-down) & java/server Y origin (bottom-up)
        constexpr int worldWidthInTiles = World::WORLD_WIDTH_IN_CHUNKS * Chunk::SIZE;
        constexpr int worldHeightInTiles = World::WORLD_HEIGHT_IN_CHUNKS * Chunk::SIZE;
        const int tileYFlipped = worldHeightInTiles - 1 - tileY;

        if (tileX <= 0 || tileX >= worldWidthInTiles - 1 || tileYFlipped <= 0 || tileYFlipped >= worldHeightInTiles - 1)
            return;

        std::ostringstream oss;
        oss << "USE_ITEM," << slotIndex << "," << tileX << "," << tileYFlipped;
        if (network)
            network->queueMessage(oss.str());
    }
    //scroll wheel cycle for tiles/speed/zoom
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
        else if (!isInventoryOpen)
        {
            int currentIndex = inventory.selectedHotbarIndex;
            if (e.wheel.y < 0)
                currentIndex = (currentIndex + 1) % HOTBAR_SIZE;
            else if (e.wheel.y > 0)
                currentIndex = (currentIndex - 1 + HOTBAR_SIZE) % HOTBAR_SIZE;

            inventory.selectedHotbarIndex = currentIndex;
        }
    }
    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_9 && !isInventoryOpen)
        inventory.selectedHotbarIndex = (e.key.keysym.sym - SDLK_1);
    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_0 && !isInventoryOpen)
        inventory.selectedHotbarIndex = 9;
}

void Game::renderInventory(SDL_Renderer* renderer, const int winW, const int winH) const
{
    const TextureManager& texManager = TextureManager::getInstance();
    constexpr SDL_Color textColor = { 255, 255, 255, 255 };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int maxSlot = HOTBAR_SIZE;
    if (isInventoryOpen)
        maxSlot = INVENTORY_SIZE;

    for (int i = 0; i < maxSlot; i++)
    {
        int x, y;
        Inventory::getSlotScreenPosition(i, x, y, winW, winH);
        const auto& slot = inventory.slots[i];

        SDL_Rect slotRect = { x, y, SLOT_SIZE, SLOT_SIZE };
        if (i < HOTBAR_SIZE && i == inventory.selectedHotbarIndex)
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); //selected slot will be yellow
        else
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); //else be dark grey

        SDL_RenderFillRect(renderer, &slotRect);

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); //light grey border around slots
        SDL_RenderDrawRect(renderer, &slotRect);

        //adding textures to filled slots
        if (!slot.isEmpty())
        {
            constexpr int iconSize = static_cast<int>(SLOT_SIZE * 0.75f);
            constexpr int iconOffset = (SLOT_SIZE - iconSize) / 2;
            texManager.draw(renderer, slot.getTextureID(), x + iconOffset, y + iconOffset, iconSize, iconSize);

            if (slot.quantity > 1)
            {
                std::string itemQuantity = std::to_string(slot.quantity);
                drawText(renderer, itemQuantity, x + SLOT_SIZE - static_cast<int>(itemQuantity.length()) * 10, y + SLOT_SIZE - 25, textColor);
            }
        }
    }

    if (!inventory.mouseHeldItem.isEmpty())
    {
        //get mouse position and render block image to it
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        const auto& heldSlot = inventory.mouseHeldItem;

        //draw item centred
        constexpr int iconSize = static_cast<int>(SLOT_SIZE * 0.75f);
        texManager.draw(renderer, heldSlot.getTextureID(),
                        mouseX - iconSize / 2, mouseY - iconSize / 2,
                        iconSize, iconSize);

        //draw quantity next to item
        const std::string heldItemQuantity = std::to_string(heldSlot.quantity);
        drawText(renderer, heldItemQuantity,
                 mouseX + iconSize / 2 + 5, mouseY - 15, textColor);
    }
}

int Game::getSlotIndexAt(const int mouseX, const int mouseY, const int winW, const int winH) const
{
    int maxSlot = HOTBAR_SIZE;
    if (isInventoryOpen)
        maxSlot = INVENTORY_SIZE;

    for (int i = 0; i < maxSlot; ++i)
    {
        int slotX, slotY;
        Inventory::getSlotScreenPosition(i, slotX, slotY, winW, winH);

        if (mouseX >= slotX && mouseX < slotX + SLOT_SIZE &&
            mouseY >= slotY && mouseY < slotY + SLOT_SIZE)
            return i;
    }

    return -1; //no slot found
}

void Game::render(SDL_Renderer* renderer)
{
    const TextureManager& texManager = TextureManager::getInstance();
    std::string textureId;
    SDL_SetRenderDrawColor(renderer, 30, 160, 230, 255);
    SDL_RenderClear(renderer);

    if (!world) return;
    int winW, winH;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);

    const float cameraX = camera.getPreciseX();
    const float cameraY = camera.getPreciseY();
    const float zoom = camera.getZoom();

    //culling
    float cullLeftPix = -cameraX / zoom;
    float cullTopPix = -cameraY / zoom;
    float cullRightPix = (winW - cameraX) / zoom;
    float cullBottomPix = (winH - cameraY) / zoom;

    constexpr float TILE_PX_SIZE = static_cast<float>(World::TILE_PX_SIZE);
    constexpr float CHUNK_SIZE_PX = static_cast<float>(Chunk::SIZE * World::TILE_PX_SIZE);

    int startChunkX = std::clamp(static_cast<int>(std::floor(cullLeftPix / CHUNK_SIZE_PX)), 0, World::WORLD_WIDTH_IN_CHUNKS - 1);
    int endChunkX = std::clamp(static_cast<int>(std::ceil(cullRightPix / CHUNK_SIZE_PX)) - 1, 0, World::WORLD_WIDTH_IN_CHUNKS - 1);
    int startChunkY_Down = std::clamp(static_cast<int>(std::floor(cullTopPix / CHUNK_SIZE_PX)), 0, World::WORLD_HEIGHT_IN_CHUNKS - 1);
    int endChunkY_Down = std::clamp(static_cast<int>(std::ceil(cullBottomPix / CHUNK_SIZE_PX)) - 1, 0, World::WORLD_HEIGHT_IN_CHUNKS - 1);

    //render the world layers
    for (int layer = TileLayer::NUM_LAYERS - 1; layer >= 0; --layer)
    {
        for (int cx = startChunkX; cx <= endChunkX; ++cx)
        {
            for (int cy_Down = startChunkY_Down; cy_Down <= endChunkY_Down; ++cy_Down)
            {
                const int chunkY_BottomUp = World::WORLD_HEIGHT_IN_CHUNKS - 1 - cy_Down;
                Chunk* chunk = world->getChunk(cx, chunkY_BottomUp);
                if (!chunk) continue;

                const float chunkWorldX = cx * CHUNK_SIZE_PX;
                const float chunkWorldY = cy_Down * CHUNK_SIZE_PX;

                int startTileX = std::max(0, static_cast<int>(std::floor((cullLeftPix - chunkWorldX) / TILE_PX_SIZE)));
                int startTileY_Down = std::max(0, static_cast<int>(std::floor((cullTopPix - chunkWorldY) / TILE_PX_SIZE)));
                int endTileX = std::min(Chunk::SIZE - 1, static_cast<int>(std::ceil((cullRightPix - chunkWorldX) / TILE_PX_SIZE)) - 1);
                int endTileY_Down = std::min(Chunk::SIZE - 1, static_cast<int>(std::ceil((cullBottomPix - chunkWorldY) / TILE_PX_SIZE)) - 1);

                for (int y_Down = startTileY_Down; y_Down <= endTileY_Down; ++y_Down)
                {
                    const int y_Storage = Chunk::SIZE - 1 - y_Down;
                    for (int x_Local = startTileX; x_Local <= endTileX; ++x_Local)
                    {
                        const int type = chunk->tiles[y_Storage][x_Local][layer].type;
                        if (type == 0) continue;

                        const float currentUnscaledWorldX = chunkWorldX + x_Local * TILE_PX_SIZE;
                        const float currentUnscaledWorldY = chunkWorldY + y_Down * TILE_PX_SIZE;

                        const int currentScreenX = static_cast<int>(std::floor(currentUnscaledWorldX * zoom + cameraX));
                        const int currentScreenY = static_cast<int>(std::floor(currentUnscaledWorldY * zoom + cameraY));
                        const int nextScreenX = static_cast<int>(std::floor((currentUnscaledWorldX + TILE_PX_SIZE) * zoom + cameraX));
                        const int nextScreenY = static_cast<int>(std::floor((currentUnscaledWorldY + TILE_PX_SIZE) * zoom + cameraY));

                        const int scaledTileWidth = nextScreenX - currentScreenX;
                        const int scaledTileHeight = nextScreenY - currentScreenY;

                        texManager.draw(renderer, getTextureIDFromType(type), currentScreenX, currentScreenY, scaledTileWidth, scaledTileHeight);
                    }
                }
            }
        }
    }

    if (particleManager) particleManager->render(renderer, cameraX, cameraY, zoom);

    //Almas recommended I implement some animations using maths
    const ItemSlot& selectedSlot = inventory.slots[inventory.selectedHotbarIndex];
    const auto& itemDef = ItemRegistry::getInstance().getDefinition(selectedSlot.itemID);

    if (!isInventoryOpen && !isFreecamActive && selectedSlot.quantity > 0)
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        //mouse to world coords
        float mouseWorldX = (static_cast<float>(mx) - cameraX) / zoom;
        float mouseWorldY = (static_cast<float>(my) - cameraY) / zoom;
        int tileX = static_cast<int>(std::floor(mouseWorldX / TILE_PX_SIZE));
        int tileY = static_cast<int>(std::floor(mouseWorldY / TILE_PX_SIZE));

        //render
        int ghostScreenX = static_cast<int>(std::floor((tileX * TILE_PX_SIZE) * zoom + cameraX));
        int ghostScreenY = static_cast<int>(std::floor((tileY * TILE_PX_SIZE) * zoom + cameraY));
        int nextGhostScreenX = static_cast<int>(std::floor(((tileX + 1) * TILE_PX_SIZE) * zoom + cameraX));
        int nextGhostScreenY = static_cast<int>(std::floor(((tileY + 1) * TILE_PX_SIZE) * zoom + cameraY));

        int drawW = nextGhostScreenX - ghostScreenX;
        int drawH = nextGhostScreenY - ghostScreenY;

        if (itemDef.isTile)
        {
            //pulsing block effect
            float time = static_cast<float>(SDL_GetTicks()) / 1000.0f;
            float pulse = (std::sin(time * 8.0f) + 1.0f) / 2.0f;
            auto alpha = static_cast<Uint8>(120 + (pulse * 80));

            if (SDL_Texture* ghostTex = texManager.getTexture(itemDef.textureID))
            {
                SDL_SetTextureBlendMode(ghostTex, SDL_BLENDMODE_BLEND);
                SDL_SetTextureAlphaMod(ghostTex, alpha);
                texManager.draw(renderer, itemDef.textureID, ghostScreenX, ghostScreenY, drawW, drawH);
                SDL_SetTextureAlphaMod(ghostTex, 255);
            }
        }
        else
        {
            //tool selection outline
            //render a white outline around the block if a tool is being held
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_Rect outlineRect = { ghostScreenX, ghostScreenY, drawW, drawH };

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
            SDL_RenderDrawRect(renderer, &outlineRect);

            outlineRect.x += 1; outlineRect.y += 1;
            outlineRect.w -= 2; outlineRect.h -= 2;
            SDL_RenderDrawRect(renderer, &outlineRect);
        }
    }

    //render players on top of world
    const int originalScaledTilePxSize = static_cast<int>(std::round(World::TILE_PX_SIZE * zoom));
    for (auto& [id, p] : players)
    {
        const int playerScreenX = static_cast<int>(std::floor((p.visualX * World::TILE_PX_SIZE) * zoom + cameraX));
        const int playerScreenY = static_cast<int>(std::floor((p.visualY * World::TILE_PX_SIZE) * zoom + cameraY));

        SDL_Rect rect { playerScreenX, playerScreenY, originalScaledTilePxSize, originalScaledTilePxSize * 2 };
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);

        if (!p.name.empty()) {
            drawText(renderer, p.name, playerScreenX + (originalScaledTilePxSize / 2) - (static_cast<int>(p.name.length()) * 5), playerScreenY - 25, { 255, 255, 255, 255 });
        }
    }

    renderInventory(renderer, winW, winH);

    constexpr int margin = 10;
    std::ostringstream zoomOss;
    zoomOss.precision(2);
    zoomOss << std::fixed << zoom;
    drawText(renderer, "Zoom: x" + zoomOss.str(), winW - 125, margin + 8, { 255, 255, 255, 255 });

    if (isFreecamActive)
        drawText(renderer, "Explore Mode [WASD]", winW / 2 - 375, margin + 8, { 255, 255, 0, 255 });

    SDL_RenderPresent(renderer);
}

void Game::update()
{
    if (particleManager) particleManager->update(0.016f); //idk the delta time sdl stuff

    float lerpSpeed = 0.8f; //lower = smoother but laggier | higher = snappier but more jitter
    for (auto& [id, p] : players)
    {
        //move current x/y to targetX/Y gradually
        p.visualX += (p.targetX - p.visualX) * lerpSpeed;
        p.visualY += (p.targetY - p.visualY) * lerpSpeed;
    }

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
        const float playerCentreX = p.visualX * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2.0f;
        const float playerCentreY = p.visualY * World::TILE_PX_SIZE + World::TILE_PX_SIZE / 2.0f;

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

//helper to get ID easily from type
std::string Game::getTextureIDFromType(const int type) {
    switch (type) {
    case 1: return "grass";
    case 2: return "dirt";
    case 3: return "stone";
    case 4: return "wood_log";
    case 5: return "torch";
    case 6: return "wood_plank";
    case 7: return "wood_plank_bg";
    case 8: return "stone_bg";
    case 9: return "dirt_bg";
    case 10: return "leaves";
    case 11: return "tall_grass";
    case 12: return "flowers";
    case 13: return "slate";
    case 14: return "slate_bg";
    case 15: return "bedrock";
    case 16: return "wood_platform";
    case 17: return "glass";
    default: return "missing_texture";
    }
}