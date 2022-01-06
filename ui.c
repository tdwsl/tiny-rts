#include <SDL2/SDL.h>
#include "initSDL.h"
#include "ui.h"

SDL_Texture *fontTex = NULL;
SDL_Texture *uiTex = NULL;

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
