#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>

#define EDGEPAN_SPEED 0.1

extern struct unit **selectedUnits;
extern int numSelectedUnits;
extern float cameraX, cameraY, cameraXV, cameraYV;
extern Uint32 oldBtn;
extern bool dragging, clicking;
extern int playerTeam;

void leftClickMap(int mx, int my);
void rightClickMap(int mx, int my);
void updateLevel(int diff);
void initLevel();
void endLevel();
void startDrag();
void endDrag();
void getDragXY(float *x, float *y);
void drawSelectRect(int x, int y);

#endif
