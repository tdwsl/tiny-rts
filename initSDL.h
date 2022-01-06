#ifndef INITSDL_H
#define INITSDL_H

#include <SDL2/SDL.h>

extern SDL_Renderer *renderer;
extern SDL_Window *win;

void initSDL();
void endSDL();
SDL_Texture *loadTexture(const char *filename);
void updateDisplay();
Uint32 getMouseState(int *x, int *y);

#endif
