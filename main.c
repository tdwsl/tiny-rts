#include <SDL2/SDL.h>
#include <stdbool.h>
#include "initSDL.h"
#include "map.h"
#include "unit.h"
#include "ui.h"
#include "fov.h"

#define EDGEPAN_SPEED 0.1

struct unit *selectedUnit=0;
float cameraX = 0, cameraY = 0;
float cameraXV = 0, cameraYV = 0;

void drawMinimapBox() {
    int x1 = cameraX/8, y1 = cameraY/8;
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

    x1 += WIDTH-40+4;
    y1 += 4;
    x2 += WIDTH-40+4;
    y2 += 4;

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    drawLine(x1, y1, x2, y1);
    drawLine(x1, y2, x2+1, y2);
    drawLine(x1, y1, x1, y2);
    drawLine(x2, y1, x2, y2);
}

void draw() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(renderer);

	drawMap(-cameraX, -cameraY);
	drawUnits(-cameraX, -cameraY);
	drawFov(-cameraX, -cameraY);
	if(selectedUnit)
        drawUnitUI(selectedUnit, -cameraX, -cameraY);

    drawSidebar();

    drawMinimap();
    drawMinimapUnits();
    drawMinimapFov();
    drawMinimapBox();

	drawCursor();

	updateDisplay();
}

void rightClickMap(int mx, int my) {
    if(!selectedUnit)
        return;

    int tx = (mx+cameraX)/4;
    int ty = (my+cameraY)/4;

    struct unit *u = unitAt(tx, ty);
    if(u) {
        if(u == selectedUnit) {
            unitUnload(u);
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
    int mx, my;
    getMouseState(&mx, &my);

    updateUnits(diff);

    bool selected = false;
    for(int i = 0; i < numUnits; i++)
        if(units[i] == selectedUnit)
            selected = true;
    if(!selected)
        selectedUnit = 0;

    if(mx < 1)
        cameraX -= EDGEPAN_SPEED*diff;
    if(my < 1)
        cameraY -= EDGEPAN_SPEED*diff;
    if(mx >= WIDTH-1)
        cameraX += EDGEPAN_SPEED*diff;
    if(my >= HEIGHT-1)
        cameraY += EDGEPAN_SPEED*diff;

    cameraX += cameraXV*diff;
    cameraY += cameraYV*diff;

    if(cameraX < -WIDTH/2)
        cameraX = -WIDTH/2;
    if(cameraY < -HEIGHT/2)
        cameraY = -HEIGHT/2;
    if(cameraX+WIDTH >= map.w*8+WIDTH/2)
        cameraX = map.w*8+WIDTH/2-1 - WIDTH;
    if(cameraY+HEIGHT >= map.h*8+HEIGHT/2)
        cameraY = map.h*8+HEIGHT/2-1 - HEIGHT;
}

void click() {
    int mx, my;
    Uint8 btn = getMouseState(&mx, &my);
    if(btn & SDL_BUTTON_LMASK)
        leftClickMap(mx, my);
    if(btn & SDL_BUTTON_RMASK)
        rightClickMap(mx, my);
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

	loadMap("lvl/0/map.txt");
	newUnit(8, 11, UNIT_BUGGY);
	newUnit(10, 11, UNIT_RIFLEMAN);
	newUnit(10, 12, UNIT_ROCKETLAUNCHER);
	newUnit(2, 1, UNIT_GUNBOAT);
	newUnit(5, 1, UNIT_CARGOSHIP);
	newUnit(14, 9, UNIT_TANK);

	initFov();
	initMinimap();

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
                click();
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

	freeMinimap();
	freeFov();

	freeUnits();
	free(map.arr);
	endSDL();
	return 0;
}
