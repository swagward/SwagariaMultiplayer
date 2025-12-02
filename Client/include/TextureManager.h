#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>

class TextureManager
{
    public:
    static TextureManager& getInstance();
    ~TextureManager();

    bool loadTexture(const std::string& id, const std::string& path, SDL_Renderer* renderer);
    SDL_Texture* getTexture(const std::string& id) const;
    void draw(SDL_Renderer* renderer, const std::string& id, int x, int y, int w, int h) const;

private:
    TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    std::unordered_map<std::string, SDL_Texture*> textureMap;
};