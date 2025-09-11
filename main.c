#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "headers.h"

#define FLOOR_WIDTH 10
#define FLOOR_LENGTH 25

Block *blocks;
Walls *walls;
Floor *floors;
Stairs *stairs;
Poles *poles;

int main()
{
    int stairsCount = countLines("inputs/stairs.txt");
    int polesCount = countLines("inputs/poles.txt");
    int wallsCount = countLines("inputs/walls.txt");

    Stairs *stairs = malloc(stairsCount * sizeof(Stairs));
    Poles *poles = malloc(polesCount * sizeof(Poles));
    Walls *walls = malloc(wallsCount * sizeof(Walls));

    printf("Starting UCSC Maze\n");

    stairs = (Stairs *)calloc(stairsCount, sizeof(Stairs));
    if (!stairs)
    {
        fprintf(stderr, "Error in allocating memory\n");
        return 1;
    }

    poles = (Poles *)calloc(polesCount, sizeof(Poles));
    if (!poles)
    {
        fprintf(stderr, "Error in allocating memory\n");
        free(stairs);
        return 1;
    }

    walls = (Walls *)calloc(wallsCount, sizeof(Walls));
    if (!walls)
    {
        fprintf(stderr, "Error in allocating memory\n");
        free(stairs);
        free(poles);
        return 1;
    }

    floors = (Floor *)calloc(3, sizeof(Floor));
    if (!floors)
    {
        fprintf(stderr, "Error in allocating memory\n");
        free(stairs);
        free(poles);
        free(walls);
        return 1;
    }

    blocks = (Block *)calloc(FLOOR_WIDTH * FLOOR_LENGTH * 3, sizeof(Block));
    if (!blocks)
    {
        fprintf(stderr, "Error in allocating memory\n");
        free(stairs);
        free(poles);
        free(walls);
        free(floors);
        return 1;
    }

    printf("\nInitializing the game...\n");

    int flagIndex = readFlagPosition(FLOOR_WIDTH, FLOOR_LENGTH);
    if (flagIndex == -1)
    {
        fprintf(stderr, "Error reading flag position\n");
        flagIndex = -1;
    }

    initializeFloors(floors, blocks, FLOOR_WIDTH, FLOOR_LENGTH, flagIndex);

    printf("\nLoading game inputs...\n");
    loadStairs("inputs/stairs.txt", stairs, blocks, stairsCount, FLOOR_WIDTH);
    loadPoles("inputs/poles.txt", poles, blocks, polesCount, FLOOR_WIDTH);
    loadWalls("inputs/walls.txt", walls, blocks, wallsCount, FLOOR_WIDTH);

    Player *players = (Player *)malloc(3 * sizeof(Player));
    if (!players)
    {
        fprintf(stderr, "Error in allocating memory\n");
        free(stairs);
        free(poles);
        free(walls);
        free(floors);
        free(blocks);
        return 1;
    }

    players[0].playerName = 'A';
    players[0].isStarted = 'n';
    players[0].PosX = 12;
    players[0].PosY = 6;
    players[0].floor = 0;
    players[0].direction = 0;
    players[0].movPoints = 0;
    players[0].speed = 1;

    players[1].playerName = 'B';
    players[1].isStarted = 'n';
    players[1].PosX = 7;
    players[1].PosY = 9;
    players[1].floor = 0;
    players[1].direction = 3;
    players[1].movPoints = 0;
    players[1].speed = 1;

    players[2].playerName = 'C';
    players[2].isStarted = 'n';
    players[2].PosX = 17;
    players[2].PosY = 9;
    players[2].floor = 0;
    players[2].direction = 2;
    players[2].movPoints = 0;
    players[2].speed = 1;

    printf("\nPlayer starting positions:\n");
    printf("Player A is in the Starting area (6,12) & first maze cell is (5,12). Direction: North\n");
    printf("Player B is in the Starting area (9,8) & first maze cell is (9,7). Direction: West\n");
    printf("Player C is in the Starting area (9,16) & first maze cell is (9,17). Direction: East\n");

    printf("\nStarting Gameplay...\n\n");

    gamePlay(blocks, players, FLOOR_WIDTH, FLOOR_LENGTH, stairs, stairsCount, poles, polesCount);

    printf("\nGameplay Stopped!\n");

    printf("Cleaning up memory...\n");
    free(stairs);
    free(poles);
    free(walls);
    free(floors);
    free(blocks);
    free(players);

    printf("Game logic finished.\n");

    return 0;
}