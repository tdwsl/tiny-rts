#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include "map.h"
#include "initSDL.h"
#include "unit.h"

SDL_Texture *tileset = NULL;

struct map map;

void initMap(int w, int h, int t) {
	map.arr = malloc(sizeof(int)*w*h);
	map.w = w;
	map.h = h;
	for(int i = 0; i < w*h; i++)
		map.arr[i] = t;
}

void printMap(struct map map) {
    for(int y = 0; y < map.h; y++) {
        for(int x = 0; x < map.w; x++)
            printf("%2d", map.arr[y*map.w+x]);
        printf("\n");
    }
}

int getTile(struct map map, int x, int y) {
	if(x < 0 || y < 0 || x >= map.w || y >= map.h)
		return -1;
	return map.arr[y*map.w+x];
}

void setTile(struct map map, int x, int y, int t) {
	if(x >= 0 && y >= 0 && x < map.w && y < map.h)
		map.arr[y*map.w+x] = t;
}

void loadMap(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if(!fp) {
		printf("Failed to open map '%s'\n", filename);
		endSDL();
		exit(1);
	}
	fscanf(fp, "%d%d", &map.w, &map.h);
	map.arr = malloc(sizeof(int)*map.w*map.h);
	for(int i = 0; i < map.w*map.h; i++)
		fscanf(fp, "%d", &map.arr[i]);
	fclose(fp);
}

int blocks(int t, int hlevel) {
    if(hlevel == HEIGHTLEVEL_SKY)
        return 0;
    switch(t) {
    case 2:
        if(hlevel == HEIGHTLEVEL_SEA)
            return 0;
        break;
    case 1:
        if(hlevel != HEIGHTLEVEL_LAND)
            return 2;
        break;
    case 0:
        if(hlevel == HEIGHTLEVEL_SHALLOW)
            return 0;
        if(hlevel == HEIGHTLEVEL_LAND)
            return 2;
        break;
    case 3:
        if(hlevel != HEIGHTLEVEL_SEA)
            return 0;
        break;
    }
    return 1;
}

int tileBlocks(int x, int y, int hlevel) {
    return blocks(getTile(map, x, y), hlevel);
}

/*struct map generatePathmap(struct unit *u, int x2, int y2) {
    struct unit_stats stats = getUnitStats(u->type);
    int x1 = u->x, y1 = u->y;
    int sz = 1 + !stats.infantry;
    printf("generating\n");

    struct map pmap;
    pmap.w = map.w*2;
    pmap.h = map.h*2;

    pmap.arr = malloc(sizeof(int)*pmap.w*pmap.h);
    for(int i = 0; i < pmap.w*pmap.h; i++) {
        int mx = (i%pmap.w)/2;
        int my = (i/pmap.w)/2;

        pmap.arr[i] = blocks(map.arr[my*map.w+mx], stats.heightLevel)*-1;

        for(int x = 0; x < sz; x++)
            for(int y = 0; y < sz; y++) {
                int mx = (i%pmap.w + x)/2;
                int my = (i/pmap.w + y)/2;

                int t = tileBlocks(mx, my, stats.heightLevel)*-1;
                if(t == -1)
                    pmap.arr[i] = t;

                struct unit *ua = unitAt(u->x+x, u->y+y);
                if(ua && ua != u)
                    if(ua->x == ua->px && ua->y == ua->py)
                        pmap.arr[i] = -1;
            }
    }
    printf("pathmap yay\n");
    printMap(pmap);

    if(pmap.arr[y2*pmap.w+x2] == -1) {
        free(pmap.arr);
        pmap.arr = 0;
        return pmap;
    }

    setTile(pmap, x2, y2, 1);
    setTile(pmap, x2+1, y2, 1);
    setTile(pmap, x2, y2+1, 1);
    setTile(pmap, x2+1, y2+1, 1);

    if(pmap.arr[y1*pmap.w+x1] == -1) {
        free(pmap.arr);
        pmap.arr = 0;
        return pmap;
    }
    pmap.arr[y1*pmap.w+x1] = 0;

    printf("doing\n");

    int stuckDuration = 0;
    for(int i = 1; !pmap.arr[y1*pmap.w+x1]; i++) {
        printf("%d\n", i);
        bool stuck = true;

        for(int j = 0; j < pmap.w*pmap.h; j++) {
            if(pmap.arr[j] != i)
                continue;

            stuck = false;

            int x = j%pmap.w, y = j/pmap.w;
            for(int d = 0; d < 8; d++) {
                int t = getTile(pmap, x+ddirs[d*2], y+ddirs[d*2+1]);
                if(t == 0 || t == -2)
                    setTile(pmap, x+ddirs[d*2], y+ddirs[d*2+1], i+1);
            }
        }

        for(int j = 0; j < pmap.w*pmap.h && !stuck; j++) {
            if(pmap.arr[j] != i+1)
                continue;

            int x = j%pmap.w, y = j/pmap.w;
            for(int d = 0; d < 8; d++) {
                int t = getTile(pmap, x+ddirs[d*2], y+ddirs[d*2+1]);
                if(t == 0)
                    setTile(pmap, x+ddirs[d*2], y+ddirs[d*2+1], i+2);
            }
        }

        if(stuck) {
            stuckDuration++;
            if(stuckDuration < 5)
                continue;

            free(pmap.arr);
            pmap.arr = 0;
            return pmap;
        }
        else
            stuckDuration = 0;
    }

    return pmap;
}*/

struct map generatePathmap(struct unit *u, int x2, int y2) {
    struct map pmap;
    pmap.w = map.w*2;
    pmap.h = map.h*2;
    pmap.arr = malloc(sizeof(int)*pmap.w*pmap.h);

    struct unit_stats stats = getUnitStats(u->type);

    for(int i = 0; i < pmap.w*pmap.h; i++) {
        int tx = (i%pmap.w);
        int ty = (i/pmap.w);
        pmap.arr[i] = tileBlocks(tx/2, ty/2, stats.heightLevel)*-1;

        if(tx >= x2 && ty >= y2 && tx <= x2+stats.infantry && ty <= y2+stats.infantry)
            continue;

        struct unit *ua = unitAt(tx, ty);
        if(ua && ua != u)
            if(u->x == u->px && u->y == u->py) {
                setTile(pmap, tx, ty, -1);
                setTile(pmap, tx+!stats.infantry, ty, -1);
                setTile(pmap, tx, ty+!stats.infantry, -1);
                setTile(pmap, tx+!stats.infantry, ty+!stats.infantry, -1);
            }
    }

    if(!stats.infantry)
        for(int i = 0; i < pmap.w*pmap.h-pmap.w-1; i++)
            if(pmap.arr[i+1] == -1 || pmap.arr[i+pmap.w] == -1 || pmap.arr[i+pmap.w+1] == -1)
                pmap.arr[i] = -1;

    if(getTile(pmap, u->x, u->y) == -1 || getTile(pmap, x2, y2) == -1) {
        free(pmap.arr);
        pmap.arr = 0;
        return pmap;
    }

    setTile(pmap, x2, y2, 1);
    setTile(pmap, u->x, u->y, 0);

    int stuckDuration = 0;

    for(int i = 1; getTile(pmap, u->x, u->y) == 0; i++) {
        bool stuck = true;

        for(int j = 0; j < pmap.w*pmap.h; j++) {
            if(pmap.arr[j] != i)
                continue;

            int tx = j%pmap.w, ty = j/pmap.w;

            stuck = false;

            for(int d = 0; d < 8; d++) {
                int t = getTile(pmap, tx+ddirs[d*2], ty+ddirs[d*2+1]);
                if(t == 0 || t == -2)
                    setTile(pmap, tx+ddirs[d*2], ty+ddirs[d*2+1], i+1);
            }
        }

        if(stuck) {
            stuckDuration++;
            if(stuckDuration < 4)
                continue;

            free(pmap.arr);
            pmap.arr = 0;
            return pmap;
        }
        else
            stuckDuration = 0;

        for(int k = 1; k < 12; k++)
            for(int j = 0; j < pmap.w*pmap.h; j++) {
                if(pmap.arr[j] != i+k)
                    continue;

                int tx = j%pmap.w, ty = j/pmap.w;

                for(int d = 0; d < 8; d++) {
                    int t = getTile(pmap, tx+ddirs[d*2], ty+ddirs[d*2+1]);
                    if(t == 0)
                        setTile(pmap, tx+ddirs[d*2], ty+ddirs[d*2+1], i+k+1);
                }
            }
    }

    return pmap;
}

void drawMap(int xo, int yo) {
	for(int i = 0; i < map.w*map.h; i++) {
		int t = 0;
		int x = i%map.w, y = i/map.w;
		static const int pwrs[] = {1, 2, 4, 8};
		int st=-1;
		int mt = map.arr[i];

		if(mt == 3) {
            st = mt;
            mt = 0;
		}

		SDL_Rect dst = {x*8+xo, y*8+yo, 8, 8};

		for(int d = 0; d < 4; d++) {
			int at = getTile(map, x+dirs[d*2], y+dirs[d*2+1]);
			if(at != mt && at != -1 && at != 2 && !(at == 3 && mt == 0))
				t |= pwrs[d];

            if(d == 3) {
                SDL_Rect src = {(t%8)*8, (t/8)*8+mt*16, 8, 8};
                SDL_RenderCopy(renderer, tileset, &src, &dst);

                if(st != -1) {
                    mt = st;
                    d = -1;
                    st = -1;
                    t = 0;
                }
            }
		}
	}
}
