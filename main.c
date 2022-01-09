#include <SDL2/SDL.h>
#include <stdbool.h>
#include "initSDL.h"
#include "map.h"
#include "unit.h"
#include "ui.h"
#include "fov.h"
#include "level.h"

void draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

    float xd, yd;
    getDragXY(&xd, &yd);

    float xo = -cameraX, yo = -cameraY;
    if(dragging && (oldBtn & SDL_BUTTON_RMASK) && !clicking) {
        xo -= xd;
        yo -= yd;
    }

    drawMap(xo, yo);
    drawUnits(xo, yo);
    drawFov(xo, yo);

    for(int i = 0; i < numSelectedUnits; i++)
        drawUnitUI(selectedUnits[i], xo, yo);

    drawSelectRect(xo, yo);

    drawSidebar();

    drawCursor();

    updateDisplay();
}

void click() {
    int mx, my;
    Uint8 btn = getMouseState(&mx, &my);

    if(mx >= WIDTH-40) {
        return;
    }

    if(btn & SDL_BUTTON_LMASK)
        leftClickMap(mx, my);
    if(btn & SDL_BUTTON_RMASK)
        rightClickMap(mx, my);
}

void update(int diff) {
    updateLevel(diff);

    int mx, my;
    Uint32 btn = getMouseState(&mx, &my);

    if(mx >= WIDTH-40+4 && my >= 4 && mx < WIDTH-4 && my < 4+32) {
        mx -= WIDTH-40+4;
        my -= 4;

        if(btn & SDL_BUTTON_LMASK)
            leftClickMinimap(mx, my);
        else if(btn & SDL_BUTTON_RMASK)
            rightClickMinimap(mx, my);
    }
}

int main() {
    initSDL();

    tileset = loadTexture("img/terrain.bmp");
    infantryTex = loadTexture("img/infantry.bmp");
    vehicleTex = loadTexture("img/vehicles.bmp");
    fontTex = loadTexture("img/font.bmp");
    uiTex = loadTexture("img/ui.bmp");
    sidebarTex = loadTexture("img/sidebar.bmp");
    fovTex = loadTexture("img/fov.bmp");
    SDL_SetTextureAlphaMod(fovTex, 0xd0);

    initLevel();

    bool quit = false;
    int lastUpdate = SDL_GetTicks();
    int lastDraw = lastUpdate;

    while(!quit) {
        SDL_Event ev;
        while(SDL_PollEvent(&ev))
            switch(ev.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                startDrag();
                break;
            case SDL_MOUSEBUTTONUP:
                if(!dragging)
                  startDrag();
                endDrag();
                break;
            case SDL_KEYDOWN:
                switch(ev.key.keysym.sym) {
                case SDLK_UP:
                    cameraYV = -0.1;
                    break;
                case SDLK_DOWN:
                    cameraYV = 0.1;
                    break;
                case SDLK_LEFT:
                    cameraXV = -0.1;
                    break;
                case SDLK_RIGHT:
                    cameraXV = 0.1;
                    break;
                }
                break;
            case SDL_KEYUP:
                switch(ev.key.keysym.sym) {
                case SDLK_UP:
                    if(cameraYV < 0)
                        cameraYV = 0;
                    break;
                case SDLK_DOWN:
                    if(cameraYV > 0)
                        cameraYV = 0;
                    break;
                case SDLK_LEFT:
                    if(cameraXV < 0)
                        cameraXV = 0;
                    break;
                case SDLK_RIGHT:
                    if(cameraXV > 0)
                        cameraXV = 0;
                    break;
                }
                break;
            }

        int currentTime = SDL_GetTicks();
        update(currentTime-lastUpdate);
        lastUpdate = currentTime;

        if(currentTime-lastDraw > 10) {
            draw();
            lastDraw = currentTime;
        }
    }

    endLevel();

    endSDL();
    return 0;
}
