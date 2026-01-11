#pragma once
#include <SDL_mixer.h>
#include <string>
#include <unordered_map>

class AudioManager
{
public:
    static AudioManager& getInstance();
    ~AudioManager();

    bool loadSFX(const std::string& id, const std::string& path);
    void playSFX(const std::string& id, int loops = 0, int channel = -1);

private:
    AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    std::unordered_map<std::string, Mix_Chunk*> sfxMap;
};