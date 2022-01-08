#ifndef FOV_H

#include <SDL2/SDL.h>
#include "map.h"

extern SDL_Texture *fovTex;
extern struct map fov;

void initFov();
void freeFov();
void drawFov(int x, int y);
void revealFov(int x, int y, float r);
void drawMinimapFov();

#endif
