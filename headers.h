#ifndef GAMELOGIC_H
#define GAMELOGIC_H

typedef struct
{
    int startFloor;
    int startX;
    int startY;
    int endFloor;
    int endX;
    int endY;
    int direction;
} Stairs;

typedef struct
{
    int startFloor;
    int endFloor;
    int x;
    int y;
} Poles;

typedef struct
{
    int floor;
    int startX;
    int startY;
    int endX;
    int endY;
} Walls;

typedef struct
{
    int floor;
    int x;
    int y;
    int isActive;
    int movPoints;
} Block;

typedef struct
{
    int floor;
    int width;
    int length;
} Floor;

int loadStairs(const char *filename, Stairs *stairs, Block blocks[], int stairsCount, int floorWidth, int floorLength);
int loadPoles(const char *filename, Poles *poles, int polesCount);
int loadWalls(const char *filename, Walls *walls, Block blocks[], int wallsCount, int floorWidth, int floorLength);
int initializeFloors(Floor floors[], Block blocks[], int width, int length);
#endif
