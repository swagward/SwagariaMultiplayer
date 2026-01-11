#include "../include/AudioManager.h"
#include <iostream>

AudioManager& AudioManager::getInstance()
{
    static AudioManager instance;
    return instance;
}

AudioManager::~AudioManager() {
    for (auto const& [id, chunk] : sfxMap)
        if (chunk) Mix_FreeChunk(chunk);

    sfxMap.clear();
}

bool AudioManager::loadSFX(const std::string& id, const std::string& path)
{
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk)
    {
        std::cerr << "[SDL_Mixer] Failed to load " << path << ": " << Mix_GetError() << std::endl;
        return false;
    }

    sfxMap[id] = chunk;
    return true;
}

void AudioManager::playSFX(const std::string& id, int loops, int channel)
{
    if (sfxMap.count(id))
        Mix_PlayChannel(channel, sfxMap[id], loops);
}