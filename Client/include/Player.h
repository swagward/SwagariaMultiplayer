#pragma once
#include <iostream>

struct Player
{
    int id;
    float x, y;
    bool isLocal;
    std::string name = "Player";
};