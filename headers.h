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

typedef struct{
    int floor;
    int x;
    int y;
    int isActive;
    int movPoints;
} Block;

typedef struct {
    int floor;
    int width;
    int length;
} Floor;

int loadStairs(const char *filename, Stairs *stairs, int stairsCount);
int loadPoles(const char *filename, Poles *poles, int polesCount);
int loadWalls(const char *filename, Walls *walls, int wallsCount);
int initializeFloors(Floor floors[], Block blocks[], int width, int length);
int handleDeactivation(const char *filename, Block blocks[], Walls *walls, int wallCount, int blockCount, int width, int length);
int printFloorBlocks(Floor floors[], Block blocks[], int width, int length);

#endif

