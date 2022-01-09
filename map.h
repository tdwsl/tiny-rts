#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include "unit.h"

struct map {
    int w, h;
    int *arr;
};

enum {
    HEIGHTLEVEL_LAND,
    HEIGHTLEVEL_SEA,
    HEIGHTLEVEL_SKY,
    HEIGHTLEVEL_SHALLOW,
};

static const int dirs[] = {
    0, -1,
    1, 0,
    0, 1,
    -1, 0,
};

static const int ddirs[] = {
    0, -1,
    1, -1,
    1, 0,
    1, 1,
    0, 1,
    -1, 1,
    -1, 0,
    -1, -1,
};

extern SDL_Texture *tileset;
extern struct map map;

int getTile(struct map map, int x, int y);
void setTile(struct map map, int x, int y, int t);
void loadMap(const char *filename);
void drawMap(int xo, int yo);
void initMap(int w, int h, int t);
int tileBlocks(int x, int y, int hlevel);
struct map generatePathmap(struct unit *u, int x2, int y2);
SDL_Texture *initMinimap();
void drawMinimap();
void freeMinimap();
void leftClickMinimap(int mx, int my);
void rightClickMinimap(int mx, int my);

#endif
