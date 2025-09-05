#include <stdio.h>
#include "headers.h"

int loadStairs(const char *filename, Stairs *stairs, int stairsCount)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int count = 0;

    printf("Loading stairs from %s...\n", filename);

    while (count < stairsCount && fscanf(fp, "[%d,%d,%d,%d,%d,%d] ",
                                         &stairs[count].startFloor,
                                         &stairs[count].startX,
                                         &stairs[count].startY,
                                         &stairs[count].endFloor,
                                         &stairs[count].endX,
                                         &stairs[count].endY) == 6)
    {
        count++;
    }
    for (int i = 0; i < count; i++)
    {
        printf("Loaded stair %d: Starts from (%d, %d, %d) and ends from (%d, %d, %d)\n",
               i + 1,
               stairs[i].startFloor,
               stairs[i].startX,
               stairs[i].startY,
               stairs[i].endFloor,
               stairs[i].endX,
               stairs[i].endY);
    }
    fclose(fp);

    printf("Total stairs loaded: %d\n", count);
    printf("Memory Size of Stairs struct: %zu bytes\n", sizeof(Stairs));
    printf("Memory Size of stairs array: %zu bytes\n", sizeof(Poles) * stairsCount);

    printf("Finished loading stairs.\n");

    return 0;
}

int loadPoles(const char *filename, Poles *poles, int polesCount)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int count = 0;

    printf("\nLoading poles from %s...\n", filename);

    while (count < polesCount && fscanf(fp, "[%d,%d,%d,%d] ",
                                        &poles[count].startFloor,
                                        &poles[count].endFloor,
                                        &poles[count].x,
                                        &poles[count].y) == 4)
    {
        count++;
    }
    for (int i = 0; i < count; i++)
    {
        printf("Loaded pole %d: Starts from floor %d and ends at floor %d at position (%d, %d)\n",
               i + 1,
               poles[i].startFloor,
               poles[i].endFloor,
               poles[i].x,
               poles[i].y);
    }
    fclose(fp);
    printf("Total poles loaded: %d\n", count);
    printf("Memory Size of Poles struct: %zu bytes\n", sizeof(Poles));
    printf("Memory Size of poles array: %zu bytes\n", sizeof(Poles) * polesCount);

    printf("Finished loading poles.\n");
    return 0;
}

int loadWalls(const char *filename, Walls *walls, int wallsCount)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int count = 0;

    printf("\nLoading walls from %s...\n", filename);

    while (count < wallsCount && fscanf(fp, "[%d,%d,%d,%d,%d] ",
                                        &walls[count].floor,
                                        &walls[count].startX,
                                        &walls[count].startY,
                                        &walls[count].endX,
                                        &walls[count].endY) == 5)
    {
        count++;
    }
    for (int i = 0; i < count; i++)
    {
        printf("Loaded wall %d on floor %d from (%d, %d) to (%d, %d)\n",
               i + 1,
               walls[i].floor,
               walls[i].startX,
               walls[i].startY,
               walls[i].endX,
               walls[i].endY);
    }
    fclose(fp);
    printf("Total walls loaded: %d\n", count);
    printf("Memory Size of Walls struct: %zu bytes\n", sizeof(Walls));
    printf("Memory Size of walls array: %zu bytes\n", sizeof(Walls) * wallsCount);

    printf("Finished loading walls.\n");
    return 0;
}


int initializeFloors(Floor floors[], Block blocks[], int width, int length)
{
    for (int i = 0; i < 3; i++)
    {
        floors[i].floor = i;
        floors[i].width = width;
        floors[i].length = length;

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < length; y++)
            {
                int index = i * (width * length) + (y * width) + x;
                blocks[index].floor = i;
                blocks[index].x = x;
                blocks[index].y = y;
                blocks[index].isActive = 1;
                blocks[index].movPoints = 0;
            }
        }
        printf("Initialized floor %d with dimensions (%d, %d)\n", floors[i].floor + 1, floors[i].width, floors[i].length);
    }

    return 0;
}


int deactivateBlock(Block blocks[], int floor, int x, int y, int width, int length) {
    int index = floor * (width * length) + y * width + x;
    if (blocks[index].isActive) {
        blocks[index].isActive = 0;
        printf("Deactivated block at (%d, %d, %d)\n", floor, x, y);
    } else {
        printf("Block at (%d, %d, %d) is already deactivated\n", floor, x, y);
    }
    return 0;
}

int deactivateWalls(Walls *walls, Block blocks[], int width, int length, int wallCount) {
    if (walls == NULL) {
        printf("No walls to deactivate.\n");
        return 1;
    }

    for (int i = 0; i < wallCount; i++) {
        int floor = walls[i].floor;
        if (walls[i].startX == walls[i].endX) {
            int x = walls[i].startX;
            int yStart = walls[i].startY < walls[i].endY ? walls[i].startY : walls[i].endY;
            int yEnd = walls[i].startY > walls[i].endY ? walls[i].startY : walls[i].endY;
            for (int y = yStart; y <= yEnd; y++) {
                deactivateBlock(blocks, floor, x, y, width, length);
            }
        } else if (walls[i].startY == walls[i].endY) {
            int y = walls[i].startY;
            int xStart = walls[i].startX < walls[i].endX ? walls[i].startX : walls[i].endX;
            int xEnd = walls[i].startX > walls[i].endX ? walls[i].startX : walls[i].endX;
            for (int x = xStart; x <= xEnd; x++) {
                deactivateBlock(blocks, floor, x, y, width, length);
            }
        } else {
            printf("Wall %d is not axis-aligned, skipping deactivation.\n", i + 1);
        }
    }
    return 0;
}

int handleDeactivation(const char *filename, Block blocks[], Walls *walls, int wallCount, int blockCount, int width, int length)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int floor, x, y;
    while (fscanf(fp, "[%d,%d,%d] ", &floor, &x, &y) == 3)
    {
        deactivateBlock(blocks, floor, x, y, width, length);
    }
    fclose(fp);

    deactivateWalls(walls, blocks, width, length, wallCount);

    return 0;
}

int printFloorBlocks(Floor floors[], Block blocks[], int width, int length) {
    for (int i = 0; i < 3; i++) {
        printf("Blocks for floor %d:\n", floors[i].floor + 1);
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < length; y++) {
                int index = i * (width * length) + y * width + x;
                printf("  Block at (%d, %d, %d): isActive=%d, movPoints=%d\n",
                       blocks[index].floor,
                       blocks[index].x,
                       blocks[index].y,
                       blocks[index].isActive,
                       blocks[index].movPoints);
            }
        }
    }
    return 0;
}