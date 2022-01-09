#ifndef UNIT_H
#define UNIT_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define MAX_CARGOCAPACITY 10

extern SDL_Texture *infantryTex;
extern SDL_Texture *vehicleTex;

enum {
    UNIT_RIFLEMAN,
    UNIT_ROCKETLAUNCHER,
    UNIT_TANK,
    UNIT_BUGGY,
    UNIT_GUNBOAT,
    UNIT_CARGOSHIP,
};

enum {
    TARGET_NONE,
    TARGET_TILE,
    TARGET_VEHICLE,
};

struct unit {
    int x, y, px, py;
    float progress;
    int type;
    int d, pd;
    int targetMode;
    int targetX, targetY;
    struct unit *target;
    struct unit *cargo[MAX_CARGOCAPACITY];
    struct unit *transport;
    bool mapBlocked;
};

struct unit_stats {
    bool infantry;
    int hp;
    int graphic;
    float speed;
    int heightLevel;
    int capacity;
};

extern struct unit **units;
extern int numUnits;

struct unit *newUnit(int x, int y, int type);
void addUnit(struct unit *u);
void drawUnits(int x, int y);
void updateUnits(int diff);
struct unit_stats getUnitStats(int type);
void unitTarget(struct unit *u, int tx, int ty);
void unitTargetVehicle(struct unit *u, struct unit *v);
void freeUnits();
struct unit *unitAt(int x, int y);
struct unit_stats getUnitStats(int type);
void drawUnitUI(struct unit *u, int x, int y);
void drawMinimapUnits();
void unitUnload(struct unit *u);
void unitRevealFov(struct unit *u);
bool unitIsAt(struct unit *u, int x, int y);

#endif
