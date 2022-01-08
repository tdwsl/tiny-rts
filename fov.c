#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "fov.h"
#include "map.h"
#include "initSDL.h"

SDL_Texture *fovTex = NULL;
struct map fov;

void initFov() {
    fov.w = map.w;
    fov.h = map.h;
    fov.arr = malloc(sizeof(int)*fov.w*fov.h);
    for(int i = 0; i < fov.w*fov.h; i++)
        fov.arr[i] = 1;
}

void freeFov() {
    free(fov.arr);
}

void revealFov(int x, int y, float r) {
    for(float a = 0; a < 3.14159*2; a += 0.01)
        for(float m = 0; m <= r; m++)
            setTile(fov, x+cosf(a)*m+0.5, y+sinf(a)*m+0.5, 0);
}

void drawFov(int x, int y) {
    for(int i = 0; i < fov.w*fov.h; i++) {
        if(!fov.arr[i])
            continue;

        int t = 0;
        static const int pwrs[] = {1, 2, 4, 8};
        for(int d = 0; d < 4; d++)
            if(getTile(fov, i%fov.w+dirs[d*2], i/fov.w+dirs[d*2+1]))
                t |= pwrs[d];

        SDL_Rect src = {(t%8)*8, (t/8)*8, 8, 8};
        SDL_Rect dst = {(i%fov.w)*8+x, (i/fov.w)*8+y, 8, 8};
        SDL_RenderCopy(renderer, fovTex, &src, &dst);
    }
}

void drawMinimapFov() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    for(int i = 0; i < 32*32; i++) {
        int x = i%32, y = i/32;
        if(!getTile(fov, (x/32.0)*fov.w, (y/32.0)*fov.h))
            continue;
        SDL_RenderDrawPoint(renderer, WIDTH-40+4+x, 0+4+y);
    }
}
