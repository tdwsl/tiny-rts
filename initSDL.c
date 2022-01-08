#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "initSDL.h"

#define WPARMS "Tiny-RTS",\
	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,\
	640, 480,\
	SDL_WINDOW_RESIZABLE

SDL_Renderer *renderer = NULL;
SDL_Window *win = NULL;

SDL_Texture *disp = NULL;

SDL_Texture **textures = 0;
int numTextures = 0;

void f_sdlAssert(bool cond, const int line) {
	if(!cond) {
		printf("%d: %s\n", line, SDL_GetError());
		exit(1);
	}
}
#define sdlAssert(C) f_sdlAssert(C, __LINE__)

void addTexture(SDL_Texture *tex) {
	numTextures++;
	textures = realloc(textures, sizeof(SDL_Texture*)*numTextures);
	textures[numTextures-1] = tex;
}

void freeTextures() {
	for(int i = 0; i < numTextures; i++)
		SDL_DestroyTexture(textures[i]);
	free(textures);
	textures = 0;
	numTextures = 0;
}

void initSDL() {
    /* treat the touchpad like a mouse >:( */
    sdlAssert(SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0"));
    sdlAssert(SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1"));

	sdlAssert(SDL_Init(SDL_INIT_EVERYTHING) >= 0);
	sdlAssert(win = SDL_CreateWindow(WPARMS));
	sdlAssert(renderer = SDL_CreateRenderer(win, -1,
		SDL_RENDERER_ACCELERATED));

	sdlAssert(disp = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		WIDTH, HEIGHT));
	addTexture(disp);
	SDL_SetRenderTarget(renderer, disp);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(disp, SDL_BLENDMODE_BLEND);

	SDL_ShowCursor(SDL_DISABLE);
}

SDL_Rect getScreenRect() {
    int w, h;
	SDL_GetWindowSize(win, &w, &h);

	float xs = ((float)w/WIDTH), ys = ((float)h/HEIGHT);
	int scale = (xs > ys) ? ys : xs;

	int sw = WIDTH*scale, sh = HEIGHT*scale;

	SDL_Rect dst = {w/2-sw/2, h/2-sh/2, sw, sh};
	return dst;
}

void updateDisplay() {
    SDL_SetRenderTarget(renderer, NULL);
	SDL_SetRenderDrawColor(renderer, 30, 30, 30, 0xff);
	SDL_RenderClear(renderer);

	SDL_Rect dst = getScreenRect();
	SDL_RenderCopy(renderer, disp, NULL, &dst);

	SDL_RenderPresent(renderer);
	SDL_SetRenderTarget(renderer, disp);
}

Uint32 getMouseState(int *x, int *y) {
    Uint32 btn;
    int mx, my;

    if(SDL_GetKeyboardFocus()) {
        btn = SDL_GetGlobalMouseState(&mx, &my);
        int wx, wy;
        SDL_GetWindowPosition(win, &wx, &wy);
        mx -= wx;
        my -= wy;
    }
    else
        btn = SDL_GetMouseState(&mx, &my);

    SDL_Rect dst = getScreenRect();
    mx -= dst.x;
    my -= dst.y;
    mx /= (float)dst.w/(float)WIDTH;
    my /= (float)dst.h/(float)HEIGHT;

    if(mx < 0)
        mx = 0;
    if(my < 0)
        my = 0;
    if(mx >= WIDTH)
        mx = WIDTH-1;
    if(my >= HEIGHT)
        my = HEIGHT-1;

    if(x)
        *x = mx;
    if(y)
        *y = my;

    return btn;
}

SDL_Texture *loadTexture(const char *filename) {
	SDL_Surface *surf = SDL_LoadBMP(filename);
	sdlAssert(surf);
	SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format,
		0, 0xff, 0xff));

	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
	sdlAssert(tex);
	SDL_FreeSurface(surf);

	addTexture(tex);
	return tex;
}

void endSDL() {
	freeTextures();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

void drawLine(int x1, int y1, int x2, int y2) {
    int xd = x2-x1, yd = y2-y1;
    int sz = xd;
    if(sz < 0)
        sz *= -1;
    if(yd > sz)
        sz = yd;
    if(yd*-1 > sz)
        sz = yd*-1;

    for(int i = 0; i < sz; i++) {
        float m = (float)i/sz;
        SDL_RenderDrawPoint(renderer, x1+xd*m, y1+yd*m);
    }
}
