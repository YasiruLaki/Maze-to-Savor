#ifndef GAMELOGIC_H
#define GAMELOGIC_H

typedef struct {
    int startFloor;
    int startX;
    int startY;
    int endFloor;
    int endX;
    int endY;
} Stairs;

typedef struct {
    int startFloor;
    int endFloor;
    int x;
    int y;
} Poles;

typedef struct {
    int floor;
    int startX;
    int startY;
    int endX;
    int endY;
} Walls;


int loadStairs(const char *filename, Stairs *stairs, int stairsCount);
int loadPoles(const char *filename, Poles *poles, int polesCount);
int loadWalls(const char *filename, Walls *walls, int wallsCount);

#endif

