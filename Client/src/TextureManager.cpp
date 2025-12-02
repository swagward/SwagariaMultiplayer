#include "../include/TextureManager.h"

#include <iostream>
#include <ostream>

TextureManager& TextureManager::getInstance()
{
    static TextureManager instance;
    return instance;
}

TextureManager::~TextureManager()
{
    //clean up all textures
    for (const auto& [fst, snd] : textureMap)
    {
        if (snd)
            SDL_DestroyTexture(snd);
    }
    textureMap.clear();
    IMG_Quit();
}

bool TextureManager::loadTexture(const std::string& id, const std::string& path, SDL_Renderer* renderer)
{
    SDL_Surface* temp = IMG_Load(path.c_str());
    if (!temp)
    {
        std::cerr << "[SDL_IMG] Failed to load " << path << std::endl;
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_FreeSurface(temp);

    if (!texture)
    {
        std::cerr << "[SDL_IMG] Failed to create texture" << std::endl;
        return false;
    }

    textureMap[id] = texture;
    std::cout << "[SDL_IMG] Texture " << id << " loaded" << std::endl;
    return true;
}

SDL_Texture* TextureManager::getTexture(const std::string& id) const
{
    if (textureMap.count(id))
        return textureMap.at(id);
    return nullptr;
}

void TextureManager::draw(SDL_Renderer* renderer, const std::string& id, const int x, const int y, const int w, const int h) const
{
    SDL_Texture* texture = getTexture(id);
    if (!texture)
        return;

    const SDL_Rect rect = { x, y, w, h };
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
}
