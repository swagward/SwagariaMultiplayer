#pragma once
#include <iostream>

struct Player
{
    int id;
    float visualX, visualY; //purely visual coordinates
    float targetX, targetY; //actual server coords
    bool isLocal;
    std::string name = "Player";
};