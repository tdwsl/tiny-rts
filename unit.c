#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include "unit.h"
#include "initSDL.h"
#include "map.h"
#include "ui.h"
#include "fov.h"
#include "level.h"

SDL_Texture *infantryTex = NULL;
SDL_Texture *vehicleTex = NULL;

struct unit **units = 0;
int numUnits = 0;

void addUnit(struct unit *u) {
    numUnits++;
    units = realloc(units, sizeof(struct unit*)*numUnits);
    units[numUnits-1] = u;
    unitRevealFov(u);
}

struct unit *newUnit(int x, int y, int type, int team) {
    struct unit *u = malloc(sizeof(struct unit));
    u->px = x;
    u->py = y;
    u->x = x;
    u->y = y;
    u->type = type;
    u->team = team;
    u->progress = 0;
    u->d = 4;
    u->pd = 4;
    u->targetMode = 0;
    u->transport = 0;
    for(int i = 0; i < MAX_CARGOCAPACITY; i++)
        u->cargo[i] = 0;
    u->mapBlocked = false;

    addUnit(u);
    return u;
}

void freeUnits() {
    for(int i = 0; i < numUnits; i++) {
        for(int j = 0; j < MAX_CARGOCAPACITY; j++)
            if(units[i]->cargo[j])
                free(units[i]->cargo[j]);

        free(units[i]);
    }

    if(units)
        free(units);
    units = 0;
    numUnits = 0;
}

struct unit_stats getUnitStats(int type) {
    struct unit_stats stats;

    switch(type) {
    case UNIT_RIFLEMAN:
        stats.infantry = true;
        stats.hp = 10;
        stats.graphic = 0;
        stats.speed = 0.0015;
        stats.heightLevel = HEIGHTLEVEL_SHALLOW;
        stats.capacity = 0;
        break;

    case UNIT_ROCKETLAUNCHER:
        stats.infantry = true;
        stats.hp = 15;
        stats.graphic = 1;
        stats.speed = 0.0010;
        stats.heightLevel = HEIGHTLEVEL_SHALLOW;
        stats.capacity = 0;
        break;

    case UNIT_TANK:
        stats.infantry = false;
        stats.hp = 30;
        stats.graphic = 1;
        stats.speed = 0.0030;
        stats.heightLevel = HEIGHTLEVEL_LAND;
        stats.capacity = 0;
        break;

    case UNIT_BUGGY:
        stats.infantry = false;
        stats.hp = 20;
        stats.graphic = 0;
        stats.speed = 0.0080;
        stats.heightLevel = HEIGHTLEVEL_LAND;
        stats.capacity = 0;
        break;

    case UNIT_CARGOSHIP:
        stats.infantry = false;
        stats.hp = 35;
        stats.graphic = 3;
        stats.speed = 0.0040;
        stats.heightLevel = HEIGHTLEVEL_SEA;
        stats.capacity = 4;
        break;

    case UNIT_GUNBOAT:
        stats.infantry = false;
        stats.hp = 25;
        stats.graphic = 2;
        stats.speed = 0.0065;
        stats.heightLevel = HEIGHTLEVEL_SEA;
        stats.capacity = 0;
        break;
    }

    return stats;
}

struct unit *unitAt(int x, int y) {
    for(int i = numUnits-1; i >= 0; i--)
        if(unitIsAt(units[i], x, y))
            return units[i];
    return 0;
}

bool unitIsAt(struct unit *u, int x, int y) {
    if(u->x == x && u->y == y)
        return true;

    struct unit_stats stats = getUnitStats(u->type);

    if(!stats.infantry)
        if(u->x <= x && u->y <= y && u->x+1 >= x && u->y+1 >= y)
            return true;

    return false;
}

void moveUnit(struct unit *u, int xm, int ym) {
    if(!xm && !ym)
        return;

    int dx = u->x+xm, dy = u->y+ym;

    struct unit_stats stats = getUnitStats(u->type);

    for(int i = 0; i < 1 + 3*!stats.infantry; i++)
        if(tileBlocks((dx+i%2)/2, (dy+i/2)/2, stats.heightLevel) == 1) {
            u->mapBlocked = true;
            return;
        }

    if(dx < 0 || dy < 0 || dx+!stats.infantry >= map.w*2 || dy+!stats.infantry >= map.h*2)
        return;

    for(int x = 0; x <= !stats.infantry; x++)
        for(int y = 0; y <= !stats.infantry; y++) {
            struct unit *ua = unitAt(dx+x, dy+y);
            if(ua && ua != u)
                if(u->target != ua || u->targetMode != TARGET_VEHICLE)
                    return;
        }

    u->x = dx;
    u->y = dy;

    if(u->team == playerTeam)
        unitRevealFov(u);

    u->d = -1;
    for(int d = 0; d < 8 && u->d == -1; d++)
        if(ddirs[d*2] == xm && ddirs[d*2+1] == ym)
            u->d = d;
}

void unitNavTo(struct unit *u, int tx, int ty) {
    if(u->mapBlocked)
        return;

    if(u->x != u->px || u->y != u->py)
        return;

    struct unit_stats stats = getUnitStats(u->type);

    if(stats.heightLevel == HEIGHTLEVEL_SKY) {
        int xd = 0, yd = 0;
        if(u->x < tx) xd = 1;
        if(u->x > tx) xd = -1;
        if(u->y < ty) yd = 1;
        if(u->y > ty) yd = -1;
        moveUnit(u, xd, yd);
        return;
    }

    struct map pmap = generatePathmap(u, tx, ty);
    if(!pmap.arr)
        return;

    int t = getTile(pmap, u->x, u->y);
    int xd=0, yd=0;
    for(int d = 0; d < 4 && !xd && !yd; d++)
        if(getTile(pmap, u->x+dirs[d*2], u->y+dirs[d*2+1]) == t-1) {
            xd = dirs[d*2];
            yd = dirs[d*2+1];
            break;
        }
    for(int d = 0; d < 8 && !xd && !yd; d++)
        if(getTile(pmap, u->x+ddirs[d*2], u->y+ddirs[d*2+1]) == t-1) {
            xd = ddirs[d*2];
            yd = ddirs[d*2+1];
            break;
        }

    free(pmap.arr);
    moveUnit(u, xd, yd);
}

void addUnitCargo(struct unit *v, struct unit *u) {
    struct unit_stats stats = getUnitStats(v->type);
    for(int i = 0; i < stats.capacity; i++)
        if(!v->cargo[i]) {
            v->cargo[i] = u;
            return;
        }
}

void updateUnits(int diff) {
    for(int i = 0; i < numUnits; i++) {
        struct unit *u = units[i];
        struct unit_stats stats = getUnitStats(u->type);

        if(getTile(fov, u->x/2, u->y/2) && u->team == playerTeam)
            unitRevealFov(u);

        if(u->x != u->px || u->y != u->py) {
            float inc = stats.speed;
            if(tileBlocks(u->px/2, u->py/2, stats.heightLevel) == 2)
                inc *= 0.6;
            u->progress += inc*diff;
            if(u->progress >= 1.0) {
                u->progress = 0;
                u->px = u->x;
                u->py = u->y;
                u->pd = u->d;
            }
        }

        if(u->targetMode == TARGET_TILE)
            if(u->x == u->px && u->y == u->py) {
                if(u->px == u->targetX && u->py == u->targetY)
                    u->targetMode = TARGET_NONE;
                else
                    unitNavTo(u, u->targetX, u->targetY);
            }
        if(u->targetMode == TARGET_VEHICLE)
            if(u->x == u->px && u->y == u->py) {
                struct unit_stats tstats = getUnitStats(u->target->type);
                if(u->x >= u->target->x && u->y >= u->target->y
                        && u->x <= u->target->x+!tstats.infantry
                        && u->y <= u->target->y+!tstats.infantry) {
                    u->targetMode = TARGET_NONE;
                    addUnitCargo(u->target, u);
                    units[i] = units[numUnits-1];
                    u->target = 0;
                    numUnits--;
                    i--;
                }
                else {
                    if(u->targetX != u->target->x || u->targetY != u->target->y) {
                        u->targetX = u->target->x;
                        u->targetY = u->target->y;
                        u->mapBlocked = false;
                    }
                    unitNavTo(u, u->target->x, u->target->y);
                }
            }
    }
}

void unitTarget(struct unit *u, int tx, int ty) {
    struct unit_stats stats = getUnitStats(u->type);
    u->targetMode = TARGET_NONE;

    if(tx < 0 || ty < 0 || tx+!stats.infantry >= map.w*2 || ty+!stats.infantry >= map.h*2)
        return;

    /*for(int i = 0; i <= (!stats.infantry)*3; i++)
        if(tileBlocks((tx+i%2)/2, (ty+i/2)/2, stats.heightLevel) == 1)
            return;*/

    u->targetX = tx;
    u->targetY = ty;
    u->targetMode = TARGET_TILE;
    u->mapBlocked = false;
}

void unitTargetVehicle(struct unit *u, struct unit *v) {
    if(u->team != v->team)
        return;
        
    struct unit_stats vstats = getUnitStats(v->type);
    struct unit_stats ustats = getUnitStats(u->type);

    if(!ustats.infantry)
        return;

    bool seat = false;
    for(int i = 0; i < vstats.capacity && !seat; i++)
        if(!v->cargo[i])
            seat = true;

    if(!seat)
        return;

    u->targetMode = TARGET_VEHICLE;
    u->targetX = v->x;
    u->targetY = v->y;
    u->target = v;
    u->mapBlocked = false;
}

void getUnitXY(struct unit *u, int *x, int *y) {
    *x = u->px*4 + (u->x-u->px)*u->progress*4;
    *y = u->py*4 + (u->y-u->py)*u->progress*4;
}

void drawUnits(int x, int y) {
    for(int i = 0; i < numUnits; i++) {
        struct unit *u = units[i];
        struct unit_stats stats = getUnitStats(u->type);
        
        bool visible = false;
        for(int i = 0; i < 1+3*!stats.infantry; i++)
            if(getTile(fov, (u->x+i%2)/2, (u->y+i/2)/2) == 0) {
                visible = true;
                break;
            }
        if(!visible)
            continue;
        
        SDL_Texture *tex;
        int sz;

        if(stats.infantry) {
            sz = 4;
            tex = infantryTex;
        }
        else {
            sz = 8;
            tex = vehicleTex;
        }

        SDL_Rect src;
        src.x = (stats.graphic%2)*sz*4;
        src.y = (stats.graphic/2)*sz*8;
        src.w = sz;
        src.h = sz;

        int dd = u->d-u->pd;
        if(dd < -4)
            dd += 8;
        if(dd > 4)
            dd -= 8;

        int d = u->pd;
        if(stats.infantry) {
            float m = u->progress*1.5;
            if(m > 1)
                m = 1;
            d = (u->pd + (int)(dd*m)) % 8;
            if(d < 0)
                d += 8;

            src.y += d*sz;
        }

        if(!stats.infantry && u->progress) {
            float m = u->progress*1.2;
            if(m > 1)
                m = 1;
            int t = (d*2+(int)(dd*2*m))%16;
            if(t < 0)
                t += 16;
            src.x += (t%2) * sz;
            src.y += (t/2) * sz;
        }
        else if(!stats.infantry)
            src.y += d*sz;

        if(stats.infantry && u->progress) {
            int step = (int)(u->progress*8)%4;
            if(step/2)
                step = 0;
            else
                step = step%2 + 1;
            src.x += sz*step;
        }
        
        src.x += 32*(!stats.infantry+1) * u->team;

        SDL_Rect dst = {0,0,sz,sz};
        getUnitXY(u, &dst.x, &dst.y);
        dst.x += x;
        dst.y += y;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }
}

void drawUnitUI(struct unit *u, int x, int y) {
    struct unit_stats stats = getUnitStats(u->type);
    int sz = 4 + !stats.infantry*4;

    SDL_Rect dst = {0,0,2,2};
    getUnitXY(u, &dst.x, &dst.y);
    dst.x += x-1;
    dst.y += y-1;
    SDL_Rect src = {8, 0, 3, 3};

    SDL_RenderCopy(renderer, uiTex, &src, &dst);
    src.x += 1;
    dst.x += sz;
    SDL_RenderCopy(renderer, uiTex, &src, &dst);
    src.y += 1;
    dst.y += sz;
    SDL_RenderCopy(renderer, uiTex, &src, &dst);
    src.x -= 1;
    dst.x -= sz;
    SDL_RenderCopy(renderer, uiTex, &src, &dst);
    dst.y -= sz;

    if(u->targetMode != TARGET_NONE) {
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x40);
        SDL_Rect dst2;
        if(u->targetMode == TARGET_TILE)
            dst2 = (SDL_Rect){u->targetX*4+x, u->targetY*4+y, 4, 4};
        else if(u->targetMode == TARGET_VEHICLE) {
            getUnitXY(u->target, &dst2.x, &dst2.y);
            dst2.x += x;
            dst2.y += y;
            dst2.w = 4;
            dst2.h = 4;
        }

        if(!stats.infantry) {
            dst2.x += 2;
            dst2.y += 2;
        }
        src = (SDL_Rect){4, 0, 4, 4};
        SDL_RenderCopy(renderer, uiTex, &src, &dst2);
        drawLine(dst.x+sz/2, dst.y+sz/2, dst2.x+sz/4, dst2.y+sz/4);
    }

    if(stats.capacity) {
        char text[15];
        int num = 0;
        for(int i = 0; i < stats.capacity; i++)
            if(u->cargo[i])
                num++;
        sprintf(text, "%d/%d", num, stats.capacity);
        drawText(dst.x, dst.y+12, text);
    }
}

void drawMinimapUnits() {
    for(int i = 0; i < numUnits; i++) {
        struct unit *u = units[i];

        if(getTile(fov, u->x/2, u->y/2))
            return;
            
        switch(u->team) {
        case TEAM_RED:
            SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
            break;
        case TEAM_BLUE:
            SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
            break;
        }

        for(int j = 0; j < 4; j++)
            SDL_RenderDrawPoint(renderer, WIDTH-40+4+(((u->x+0.5)/2)/(float)fov.w)*32, 4+(((u->y+0.5)/2)/(float)fov.h)*32);
    }
}

void unitUnload(struct unit *u) {
    if(tileBlocks(u->x/2, u->y/2, HEIGHTLEVEL_SHALLOW) == 1)
    return;

    struct unit_stats stats = getUnitStats(u->type);
    for(int i = 0; i < stats.capacity; i++)
        if(u->cargo[i]) {
            u->cargo[i]->x = u->x;
            u->cargo[i]->y = u->y;
            u->cargo[i]->px = u->x;
            u->cargo[i]->py = u->y;

            if(u->targetMode == TARGET_TILE) {
                u->cargo[i]->targetMode = TARGET_TILE;
                u->cargo[i]->targetX = u->targetX;
                u->cargo[i]->targetY = u->targetY;
            }

            addUnit(u->cargo[i]);
            u->cargo[i] = 0;
        }
}

void unitRevealFov(struct unit *u) {
    revealFov(u->x/2, u->y/2, 10);
}
