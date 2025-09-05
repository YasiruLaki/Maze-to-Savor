#include <stdio.h>
#include "headers.h"

#define MAX_STAIRS 20
#define MAX_POLES 20
#define MAX_WALLS 50

Stairs stairs[MAX_STAIRS];
Poles poles[MAX_POLES];
Walls walls[MAX_WALLS];

int main()
{
    printf("Starting game logic...\n\n");

    loadStairs("inputs/stairs.txt",stairs, MAX_STAIRS);
    loadPoles("inputs/poles.txt",poles, MAX_POLES);
    loadWalls("inputs/walls.txt",walls, MAX_WALLS);

    printf("\nGame logic finished.\n");
    return 0;
}

