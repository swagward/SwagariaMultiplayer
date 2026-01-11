#pragma once
#include "Game.h"

struct Button
{
    SDL_Rect buttonRect;
    std::string text;
    SDL_Color colour;
    SDL_Color hoverColour;

    [[nodiscard]] bool isHovering(const int mouseX, const int mouseY) const
    {
        return mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
            mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h;
    }

    void render(SDL_Renderer* renderer, const Game& game, const int mouseX, const int mouseY) const
    {
        SDL_SetRenderDrawColor(renderer, 40, 40, 45, 255);
        if (isHovering(mouseX, mouseY))
            SDL_SetRenderDrawColor(renderer, hoverColour.r, hoverColour.g, hoverColour.b, 255);

        SDL_RenderFillRect(renderer, &buttonRect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &buttonRect);

        //centre the text in the button
        const int textX = buttonRect.x + buttonRect.w / 2 - text.length() * 6;
        const int textY = buttonRect.y + buttonRect.h / 2 - 12;
        game.drawText(renderer, text, textX, textY, {255, 255, 255, 255});
    }
};


