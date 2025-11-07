#pragma once
#include <unordered_map>
#include "Chunk.h"

class World {
public:
    static constexpr int WORLD_WIDTH_IN_CHUNKS = 16;
    static constexpr int WORLD_HEIGHT_IN_CHUNKS = 16;

    std::unordered_map<std::string, std::unique_ptr<Chunk>> chunks;

    void addChunk(std::unique_ptr<Chunk> chunk) {
        const std::string key = std::to_string(chunk->chunkX) + "," + std::to_string(chunk->chunkY);
        chunks[key] = std::move(chunk);
    }

    Chunk* getChunk(const int cx, const int cy) {
        const std::string key = std::to_string(cx) + "," + std::to_string(cy);
        const auto it = chunks.find(key);
        return it != chunks.end() ? it->second.get() : nullptr;
    }
};