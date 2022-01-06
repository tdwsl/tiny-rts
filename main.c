#include <SDL2/SDL.h>
#include <stdbool.h>
#include "initSDL.h"
#include "map.h"
#include "unit.h"
#include "ui.h"

#define EDGEPAN_SPEED 0.1

struct unit *selectedUnit=0;
float cameraX = 0, cameraY = 0;
float cursorX = 80, cursorY = 60;
float cursorXV = 0, cursorYV = 0;

void drawCursor() {
    SDL_Rect src = {0, 0, 4, 4};
    SDL_Rect dst = {(int)cursorX, (int)cursorY, 4, 4};
    SDL_RenderCopy(renderer, uiTex, &src, &dst);
}

void draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(renderer);

	drawMap(-cameraX, -cameraY);
	drawUnits(-cameraX, -cameraY);
	if(selectedUnit)
        drawUnitUI(selectedUnit, -cameraX, -cameraY);
	drawText(0, 0, "Hello, world!");

	drawCursor();

	updateDisplay();
}

void getMouse() {
    int mx, my;
    Uint8 btn = getMouseState(&mx, &my);
    cursorX = mx;
    cursorY = my;
}

void rightClickMap(int mx, int my) {
    if(!selectedUnit)
        return;

    int tx = (mx+cameraX)/4;
    int ty = (my+cameraY)/4;

    struct unit *u = unitAt(tx, ty);
    if(u) {
        if(u == selectedUnit) {
            if(tileBlocks(u->x/2, u->y/2, HEIGHTLEVEL_SHALLOW) == 1)
                return;

            struct unit_stats stats = getUnitStats(u->type);
            for(int i = 0; i < stats.capacity; i++)
                if(u->cargo[i]) {
                    u->cargo[i]->x = u->x;
                    u->cargo[i]->y = u->y;
                    u->cargo[i]->px = u->x;
                    u->cargo[i]->py = u->y;
                    addUnit(u->cargo[i]);
                    u->cargo[i] = 0;
                }
        }

        if(selectedUnit) {
            struct unit_stats stats = getUnitStats(selectedUnit->type);
            if(!stats.infantry)
                return;
            struct unit_stats vstats = getUnitStats(u->type);
            for(int i = 0; i < vstats.capacity; i++)
                if(!u->cargo[i]) {
                    unitTargetVehicle(selectedUnit, u);
                    break;
                }
        }
    }
    else if(selectedUnit) {
        struct unit_stats stats = getUnitStats(selectedUnit->type);
        if(!stats.infantry) {
            tx = (mx+cameraX-2)/4;
            ty = (my+cameraY-2)/4;
        }
        unitTarget(selectedUnit, tx, ty);
    }
}

void leftClickMap(int mx, int my) {
    int tx = (mx+cameraX)/4;
    int ty = (my+cameraY)/4;

    struct unit *u = unitAt(tx, ty);
    if(u) {
        if(u == selectedUnit)
            selectedUnit = 0;
        else
            selectedUnit = u;
    }
    else
        selectedUnit = 0;
}

void update(int diff) {
    cursorX += cursorXV*diff;
    cursorY += cursorYV*diff;
    if(cursorX < 0)
        cursorX = 0;
    if(cursorY < 0)
        cursorY = 0;
    if(cursorX >= 160)
        cursorX = 160-1;
    if(cursorY >= 120)
        cursorY = 120-1;

    updateUnits(diff);

    bool selected = false;
    for(int i = 0; i < numUnits; i++)
        if(units[i] == selectedUnit)
            selected = true;
    if(!selected)
        selectedUnit = 0;

    if(cursorX < 1)
        cameraX -= EDGEPAN_SPEED*diff;
    if(cursorY < 1)
        cameraY -= EDGEPAN_SPEED*diff;
    if(cursorX >= 160-1)
        cameraX += EDGEPAN_SPEED*diff;
    if(cursorY >= 120-1)
        cameraY += EDGEPAN_SPEED*diff;

    if(cameraX < -80)
        cameraX = -80;
    if(cameraY < -60)
        cameraY = -60;
    if(cameraX+160 >= map.w*8+80)
        cameraX = map.w*8+80-1 - 160;
    if(cameraY+120 >= map.h*8+60)
        cameraY = map.h*8+60-1 - 120;
}

int main() {
	initSDL();
	tileset = loadTexture("img/terrain.bmp");
	infantryTex = loadTexture("img/infantry.bmp");
	vehicleTex = loadTexture("img/vehicles.bmp");
	fontTex = loadTexture("img/font.bmp");
	uiTex = loadTexture("img/ui.bmp");

	loadMap("lvl/0/map.txt");
	newUnit(8, 11, UNIT_BUGGY);
	newUnit(10, 11, UNIT_RIFLEMAN);
	newUnit(10, 12, UNIT_ROCKETLAUNCHER);
	newUnit(2, 1, UNIT_GUNBOAT);
	newUnit(5, 1, UNIT_CARGOSHIP);
	newUnit(14, 9, UNIT_TANK);

	bool quit = false;
	int lastUpdate = SDL_GetTicks();
	int lastDraw = lastUpdate;

	while(!quit) {
		SDL_Event ev;
		Uint8 btn;
		while(SDL_PollEvent(&ev))
			switch(ev.type) {
			case SDL_QUIT:
				quit = true;
				break;
            case SDL_MOUSEBUTTONDOWN:
                btn = getMouseState(NULL, NULL);
                if(btn & SDL_BUTTON_LMASK)
                    leftClickMap(cursorX, cursorY);
                if(btn & SDL_BUTTON_RMASK)
                    rightClickMap(cursorX, cursorY);
                break;
            case SDL_MOUSEMOTION:
                getMouse();
                break;
            case SDL_KEYDOWN:
                switch(ev.key.keysym.sym) {
                case SDLK_UP:
                    cursorYV = -0.05;
                    break;
                case SDLK_DOWN:
                    cursorYV = 0.05;
                    break;
                case SDLK_LEFT:
                    cursorXV = -0.05;
                    break;
                case SDLK_RIGHT:
                    cursorXV = 0.05;
                    break;
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                    rightClickMap(cursorX, cursorY);
                    break;
                case SDLK_SPACE:
                case SDLK_RETURN:
                    leftClickMap(cursorX, cursorY);
                    break;
                }
                break;
            case SDL_KEYUP:
                switch(ev.key.keysym.sym) {
                case SDLK_UP:
                    if(cursorYV < 0)
                        cursorYV = 0;
                    break;
                case SDLK_DOWN:
                    if(cursorYV > 0)
                        cursorYV = 0;
                    break;
                case SDLK_LEFT:
                    if(cursorXV < 0)
                        cursorXV = 0;
                    break;
                case SDLK_RIGHT:
                    if(cursorXV > 0)
                        cursorXV = 0;
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

	freeUnits();
	free(map.arr);
	endSDL();
	return 0;
}
