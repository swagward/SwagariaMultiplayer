#pragma once

#include <unordered_map>

struct ItemDefinition
{
    int id = 0;
    std::string name = "Unknown Item";
    std::string textureID = "missing_texture";
    int maxStack = 1;
    bool isTile = false;
    int tileTypeID = 0;
};

class ItemRegistry
{
public:
    static ItemRegistry& getInstance()
    {
        static ItemRegistry instance;
        return instance;
    }

    void addDefinition(const ItemDefinition& definition)
    {
        definitions[definition.id] = definition;
    }

    const ItemDefinition& getDefinition(const int id) const
    {
        static const ItemDefinition unknownDef;
        if (definitions.count(id))
            return definitions.at(id);
        return unknownDef;
    }

    void clear()
    {
        definitions.clear();
    }

private:
    ItemRegistry() = default;
    std::unordered_map<int, ItemDefinition> definitions;
};