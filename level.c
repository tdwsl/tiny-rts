#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "level.h"
#include "initSDL.h"
#include "map.h"
#include "unit.h"
#include "fov.h"

#define DRAG_THRESHHOLD 2

struct unit **selectedUnits=0;
int numSelectedUnits = 0;

float cameraX = 0, cameraY = 0;
float cameraXV = 0, cameraYV = 0;

int oldX, oldY;
float oldCamX, oldCamY;
bool dragging = false;
bool clicking = true;
Uint32 oldBtn;

void deselectUnit(struct unit *u) {
    for(int i = 0; i < numSelectedUnits; i++)
        if(selectedUnits[i] == u) {
            numSelectedUnits--;
            selectedUnits[i] = selectedUnits[numSelectedUnits];
        }
}

bool unitSelected(struct unit *u) {
    for(int i = 0; i < numSelectedUnits; i++)
        if(selectedUnits[i] == u)
            return true;
    return false;
}

void selectUnit(struct unit *u) {
    if(unitSelected(u))
        return;

    numSelectedUnits++;
    selectedUnits = realloc(selectedUnits, sizeof(struct unit**)*numSelectedUnits);
    selectedUnits[numSelectedUnits-1] = u;
}

void deselectUnits() {
    free(selectedUnits);
    selectedUnits = 0;
    numSelectedUnits = 0;
}

void rightClickMap(int mx, int my) {
    for(int i = 0; i < numSelectedUnits; i++) {
        struct unit *selectedUnit = selectedUnits[i];

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
}

void leftClickMap(int mx, int my) {
    int tx = (mx+cameraX)/4;
    int ty = (my+cameraY)/4;

    struct unit *u = unitAt(tx, ty);
    if(u) {
        if(unitSelected(u) && numSelectedUnits == 1)
            deselectUnits();
        else {
            deselectUnits();
            selectUnit(u);
        }
    }
    else
        deselectUnits();
}

void getDragXY(float *x, float *y) {
    if(!dragging) {
        *x = 0;
        *y = 0;
        return;
    }
    int mx, my;
    getMouseState(&mx, &my);
    *x = oldX+oldCamX - mx-cameraX;
    *y = oldY+oldCamY - my-cameraY;
}

SDL_Rect getSelectRect() {
    float xd, yd;
    getDragXY(&xd, &yd);

    SDL_Rect r = {oldX+oldCamX, oldY+oldCamY, xd*-1, yd*-1};

    if(clicking || (oldBtn & SDL_BUTTON_RMASK)) {
        r.w = 0;
        r.h = 0;
        return r;
    }

    if(r.w < 0) {
        r.x += r.w;
        r.w *= -1;
    }
    if(r.h < 0) {
        r.y += r.h;
        r.h *= -1;
    }

    return r;
}

void endDrag() {
    int mx, my;
    getMouseState(&mx, &my);

    float xd, yd;
    getDragXY(&xd, &yd);

    if(clicking) {
        dragging = false;

        if(oldX >= WIDTH-40)
            return;
        if(mx >= WIDTH-40)
            return;

        if(oldBtn & SDL_BUTTON_LMASK)
            leftClickMap(mx, my);
        if(oldBtn & SDL_BUTTON_RMASK)
            rightClickMap(mx, my);

        return;
    }

    if(oldBtn & SDL_BUTTON_RMASK) {
        cameraX += xd;
        cameraY += yd;
    }

    /* select units */
    if(oldBtn & SDL_BUTTON_LMASK) {
        SDL_Rect r = getSelectRect();
        deselectUnits();

        for(int x = r.x/4; x < r.x/4+r.w/4; x++)
            for(int y = r.y/4; y < r.y/4+r.h/4; y++)
                for(int i = 0; i < numUnits; i++)
                    if(unitIsAt(units[i], x, y))
                        selectUnit(units[i]);
    }

    dragging = false;
}

void startDrag() {
    dragging = true;
    clicking = true;
    oldCamX = cameraX;
    oldCamY = cameraY;
    oldBtn = getMouseState(&oldX, &oldY);
}

void updateLevel(int diff) {
    int mx, my;
    getMouseState(&mx, &my);

    float xd, yd;
    getDragXY(&xd, &yd);
    if(fabs(xd) > DRAG_THRESHHOLD || fabs(yd) > DRAG_THRESHHOLD)
        clicking = false;

    updateUnits(diff);

    for(int i = 0; i < numSelectedUnits; i++) {
        bool present = false;

        for(int j = 0; j < numUnits && !present; j++)
            if(selectedUnits[i] == units[j])
                present = true;

        if(!present)
            deselectUnit(selectedUnits[i]);
    }

    if(!dragging || !(oldBtn & SDL_BUTTON_RMASK)) {
        if(mx < 1)
            cameraX -= EDGEPAN_SPEED*diff;
        if(my < 1)
            cameraY -= EDGEPAN_SPEED*diff;
        if(mx >= WIDTH-1)
            cameraX += EDGEPAN_SPEED*diff;
        if(my >= HEIGHT-1)
            cameraY += EDGEPAN_SPEED*diff;
    }

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

void drawSelectRect(int x, int y) {
    if(!dragging || clicking || (oldBtn & SDL_BUTTON_RMASK))
        return;

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x40);
    SDL_Rect r = getSelectRect();
    r.x += x;
    r.y += y;

    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawRect(renderer, &r);
}

void initLevel() {
	loadMap("lvl/0/map.txt");
	newUnit(8, 11, UNIT_BUGGY);
	newUnit(10, 11, UNIT_RIFLEMAN);
	newUnit(10, 12, UNIT_ROCKETLAUNCHER);
	newUnit(2, 1, UNIT_GUNBOAT);
	newUnit(5, 1, UNIT_CARGOSHIP);
	newUnit(14, 9, UNIT_TANK);

	initFov();
	initMinimap();
}

void endLevel() {
    deselectUnits();

    freeMinimap();
    freeFov();

    freeUnits();
    free(map.arr);
}
