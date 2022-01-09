#include <SDL2/SDL.h>
#include "initSDL.h"
#include "ui.h"
#include "level.h"
#include "map.h"
#include "fov.h"

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

void drawMinimapBox() {

    float xo = cameraX, yo = cameraY;

    if((oldBtn & SDL_BUTTON_RMASK) && !clicking) {
        float xd, yd;
        getDragXY(&xd, &yd);

        xo += xd;
        yo += yd;
    }

    int x1 = xo/8, y1 = yo/8;
    int x2 = x1+(WIDTH-40)/8, y2 = y1+HEIGHT/8;

    int udlr = 0;

    if(x1 < 0) {
        x1 = 0;
        udlr |= 3;
    }
    if(y1 < 0) {
        y1 = 0;
        udlr |= 1;
    }
    if(x2 >= 32) {
        x2 = 31;
        udlr |= 4;
    }
    if(y2 >= 32) {
        y2 = 31;
        udlr |= 2;
    }

    if(x2 < 0)
        x2 = 0;
    if(y2 < 0)
        y2 = 0;

    x1 += WIDTH-40+4;
    y1 += 4;
    x2 += WIDTH-40+4;
    y2 += 4;

    if(x2 < x1)
        x1 = x2;
    if(y2 < y1)
        y1 = y2;

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    drawLine(x1, y1, x2, y1);
    drawLine(x1, y2, x2+1, y2);
    drawLine(x1, y1, x1, y2);
    drawLine(x2, y1, x2, y2);
}

void drawSidebar() {
    SDL_Rect src = {0, 0, 40, 100};
    SDL_Rect dst = {WIDTH-40, 0, 40, 100};
    SDL_RenderCopy(renderer, sidebarTex, &src, &dst);

    drawMinimap();
    drawMinimapUnits();
    drawMinimapFov();
    drawMinimapBox();
}

void leftClickMinimap(int mx, int my) {
    cameraX = ((mx)/32.0)*map.w*8 - WIDTH/2;
    cameraY = ((my)/32.0)*map.h*8 - HEIGHT/2;
    return;
}

void rightClickMinimap(int mx, int my) {
    int x = (mx/32.0)*map.w*8, y = (my/32.0)*map.h*8;
    rightClickMap(x-cameraX, y-cameraY);
}
