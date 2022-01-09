#ifndef DRAWTEXT_H
#define DRAWTEXT_H

extern SDL_Texture *fontTex;
extern SDL_Texture *uiTex;
extern SDL_Texture *sidebarTex;

void drawText(int x, int y, const char *text);
void drawSidebar();
void leftClickMinimap(int mx, int my);
void drawCursor();

#endif
