#ifndef INITSDL_H
#define INITSDL_H

#include <SDL2/SDL.h>

#define WIDTH 160
#define HEIGHT 100

extern SDL_Renderer *renderer;
extern SDL_Window *win;
extern SDL_Texture *disp;

void initSDL();
void endSDL();
SDL_Texture *loadTexture(const char *filename);
void updateDisplay();
Uint32 getMouseState(int *x, int *y);
void drawLine(int x1, int y1, int x2, int y2);

#endif
