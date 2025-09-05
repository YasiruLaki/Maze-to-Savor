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

    loadStairs("inputs/stairs.txt",stairs, MAX_STAIRS);
    loadPoles("inputs/poles.txt",poles, MAX_POLES);
    loadWalls("inputs/walls.txt",walls, MAX_WALLS);

    initializeFloors(floors, blocks, FLOOR_WIDTH, FLOOR_LENGTH);
    handleDeactivation("inputs/walls.txt", blocks, walls, MAX_WALLS, FLOOR_WIDTH * FLOOR_LENGTH * 3, FLOOR_WIDTH, FLOOR_LENGTH);
    printFloorBlocks(floors, blocks, FLOOR_WIDTH, FLOOR_LENGTH);

    printf("\nGame logic finished.\n");
    return 0;
}

