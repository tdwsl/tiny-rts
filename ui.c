#include <SDL2/SDL.h>
#include "initSDL.h"
#include "ui.h"

SDL_Texture *fontTex = NULL;
SDL_Texture *uiTex = NULL;
SDL_Texture *sidebarTex = NULL;

void drawText(int x, int y, const char *text) {
    int cx=0, cy=0;
    for(const char *c = text; *c; c++) {
        SDL_Rect src = {(*c%32)*4, (*c/32)*6, 4, 6};
        SDL_Rect dst = {x+cx*4, y+cy*6, 4, 6};
        SDL_RenderCopy(renderer, fontTex, &src, &dst);
        cx++;
        if(*c == '\n') {
            cx = 0;
            cy++;
        }
        else if(*c == '\t')
            cx += 4 - cx%5;
    }
}

void drawCursor() {
    SDL_Rect src = {0, 0, 4, 4};
    int mx, my;
    getMouseState(&mx, &my);
    SDL_Rect dst = {mx, my, 4, 4};
    SDL_RenderCopy(renderer, uiTex, &src, &dst);
}

void drawSidebar() {
    SDL_Rect src = {0, 0, 40, 100};
    SDL_Rect dst = {WIDTH-40, 0, 40, 100};
    SDL_RenderCopy(renderer, sidebarTex, &src, &dst);
}
