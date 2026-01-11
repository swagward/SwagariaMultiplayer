#pragma once
#include <cmath>
#include <SDL.h>
#include <string>
#include <vector>

#include "TextureManager.h"

struct Particle
{
    float x, y;
    float vx, vy; //velocity
    float lifetime; //between 0 and 1
    std::string textureID;
    SDL_Rect srcRect; //this is the little chip of a texture
    float size;
    float rotation;
    float rotSpeed;
};

class ParticleManager
{
public:
    void spawnEffect(const float worldX, const float worldY, const std::string& textureID) {
        for (int i = 0; i < 6; ++i) {
            Particle particle;
            particle.textureID = textureID;

            //random spawn position
            particle.x = worldX + (static_cast<float>(rand() % 100) / 100.0f);
            particle.y = worldY + (static_cast<float>(rand() % 100) / 100.0f);

            particle.vx = ((rand() % 100) - 50) / 1000.0f;
            particle.vy = -((rand() % 100) / 600.0f);

            particle.lifetime = 1.0f;
            particle.size = (rand() % 3 + 2) / 16.0f; //2 to 4 pixels wide

            //pick random 4x4 area of the 16x16 texture
            particle.srcRect = { rand() % 12, rand() % 12, 4, 4 };

            particle.rotation = static_cast<float>(rand() % 360);
            particle.rotSpeed = static_cast<float>((rand() % 10) - 5);

            particles.push_back(particle);
        }
    }

    void update(float deltaTime) {
        const float gravity = 0.006f;

        for (auto it = particles.begin(); it != particles.end();) {
            //velocity
            it->x += it->vx / 5.0f;
            it->y += it->vy / 5.0f;

            //gravity
            it->vy += gravity;

            it->rotation += it->rotSpeed;

            //slow fade out
            it->lifetime -= deltaTime * 1.5f;

            if (it->lifetime <= 0) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
    }

    void render(SDL_Renderer* renderer, float cameraX, float cameraY, float zoom) {
        const TextureManager& texManager = TextureManager::getInstance();

        for (const auto& p : particles) {
            SDL_Texture* tex = texManager.getTexture(p.textureID);
            if (!tex) continue;

            const int sX = static_cast<int>(std::floor(p.x * 16.0f * zoom + cameraX));
            const int sY = static_cast<int>(std::floor(p.y * 16.0f * zoom + cameraY));
            const int sSize = static_cast<int>(std::ceil(p.size * 16.0f * zoom));

            SDL_Rect destRect = { sX, sY, sSize, sSize };

            SDL_SetTextureAlphaMod(tex, static_cast<Uint8>(p.lifetime * 255));
            SDL_RenderCopyEx(renderer, tex, &p.srcRect, &destRect, p.rotation, NULL, SDL_FLIP_NONE);
            SDL_SetTextureAlphaMod(tex, 255);
        }
    }

private:
    std::vector<Particle> particles;
};