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