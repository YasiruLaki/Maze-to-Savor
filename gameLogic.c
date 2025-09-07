#include <stdio.h>
#include "headers.h"


int deactivateBlock(Block blocks[], int index)
{
    blocks[index].isActive = 0;
    printf("Deactivated block at index %d (%d,%d,%d)\n",
           index,
           blocks[index].floor,
           blocks[index].x,
           blocks[index].y);
    return 0;
}

int handleStairMidpoint(Stairs *stairs, Block blocks[], int stairIndex, int floorWidth, int floorLength)
{
    float midXf = (stairs[stairIndex].startX + stairs[stairIndex].endX) / 2.0f;
    float midYf = (stairs[stairIndex].startY + stairs[stairIndex].endY) / 2.0f;
    int midFloor = stairs[stairIndex].endFloor - 1;

    int midX1 = (int)midXf;
    int midY1 = (int)midYf;

    if ((midXf - midX1 == 0.0f) || (midYf - midY1 == 0.0f)) {
        int index = midFloor * (floorWidth * floorLength) + midY1 * floorWidth + midX1;
        deactivateBlock(blocks, index);
    }
    return 0;
}

int loadStairs(const char *filename, Stairs *stairs, Block blocks[], int stairsCount, int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int count = 0;

    printf("Loading stairs from %s...\n", filename);

    while (fscanf(fp, "[%d,%d,%d,%d,%d,%d] ",
                                         &stairs[count].startFloor,
                                         &stairs[count].startX,
                                         &stairs[count].startY,
                                         &stairs[count].endFloor,
                                         &stairs[count].endX,
                                         &stairs[count].endY) == 6)
    {   
        stairs[count].direction = 0;

        if ((stairs[count].endFloor - stairs[count].startFloor)!=1) {
            handleStairMidpoint(stairs, blocks, count, floorWidth, floorLength);
        }
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

int loadWalls(const char *filename, Walls *walls, Block blocks[], int wallsCount, int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int count = 0;

    printf("\nLoading walls from %s...\n", filename);

    while (fscanf(fp, "[%d,%d,%d,%d,%d] ",
                                        &walls[count].floor,
                                        &walls[count].startX,
                                        &walls[count].startY,
                                        &walls[count].endX,
                                        &walls[count].endY) == 5)
    {
        if (walls[count].endX - walls[count].startX != 0)
        {
            for (int x = walls[count].startX; x <= walls[count].endX; x++)
            {
                int index = walls[count].floor * (floorWidth * floorLength) + walls[count].startY * floorWidth + x;
                deactivateBlock(blocks, index);
            }
        }
        else if (walls[count].endY - walls[count].startY != 0)
        {
            for (int y = walls[count].startY; y <= walls[count].endY; y++)
            {
                int index = walls[count].floor * (floorWidth * floorLength) + y * floorWidth + walls[count].startX;
                deactivateBlock(blocks, index);

                printf("Deactivated block at index %d (%d,%d,%d)\n",
                       index,
                       walls[count].floor,
                       walls[count].startX,
                       y);  
            }
        }

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

    printf("Finished loading walls.\n");
    return 0;
}

int handleDeactivation(const char *filename, Block blocks[], int width, int length)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Error opening file");
        return 1;
    }

    int blockFloor, blockX, blockY;

    while (fscanf(fp, "[%d,%d,%d] ",
                  &blockFloor,
                  &blockX,
                  &blockY) == 3)
    {
        int index = blockFloor * (width * length) + blockY * width + blockX;
        deactivateBlock(blocks, index);
    }

    fclose(fp);

    printf("Finished deactivating blocks.\n");
    return 0;
}

int initializeFloors(Floor floors[], Block blocks[], int width, int length)
{
    for (int i = 0; i < 3; i++)
    {
        floors[i].floor = i;
        floors[i].width = width;
        floors[i].length = length;

        for (int x = 0; x < length; x++)
        {
            for (int y = 0; y < width; y++)
            {
                int index = i * (width * length) + (x * width) + y;
                blocks[index].floor = i;
                blocks[index].x = y;
                blocks[index].y = x;
                blocks[index].isActive = 1;
                blocks[index].movPoints = 0;
            }
        }
        printf("Initialized floor %d with dimensions (%d, %d)\n", floors[i].floor + 1, floors[i].width, floors[i].length);
    }

    handleDeactivation("deactiveBlocks.txt", blocks, width, length);

    for (int i = 0; i < 3; i++)
    {
        printf("Blocks for floor %d:\n", floors[i].floor + 1);
        for (int x = 0; x < length; x++)
        {
            for (int y = 0; y < width; y++)
            {
                int index = i * (width * length) + x * width + y;
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