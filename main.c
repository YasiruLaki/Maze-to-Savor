#include <stdio.h>
#include "headers.h"

#define MAX_STAIRS 20
#define MAX_POLES 20
#define MAX_WALLS 50
#define FLOOR_WIDTH 25
#define FLOOR_LENGTH 10

Stairs stairs[MAX_STAIRS];
Poles poles[MAX_POLES];
Walls walls[MAX_WALLS];

Floor floors[3];
Block blocks[FLOOR_WIDTH * FLOOR_LENGTH * 3];

int main()
{
    printf("Starting game logic...\n\n");

    initializeFloors(floors, blocks, FLOOR_WIDTH, FLOOR_LENGTH);

    loadStairs("inputs/stairs.txt", stairs, blocks, MAX_STAIRS, FLOOR_WIDTH, FLOOR_LENGTH);
    loadPoles("inputs/poles.txt", poles, MAX_POLES);
    loadWalls("inputs/walls.txt", walls, blocks, MAX_WALLS, FLOOR_WIDTH, FLOOR_LENGTH);

    printf("\nGame logic finished.\n");
    return 0;
}
