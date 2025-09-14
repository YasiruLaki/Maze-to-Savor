#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "headers.h"

#define FLOOR_WIDTH 10
#define FLOOR_LENGTH 25
#define FLOORS_COUNT 3

#define A_START_X 12
#define A_START_Y 6
#define A_START_DIR 0

#define B_START_X 7
#define B_START_Y 9
#define B_START_DIR 3

#define C_START_X 17
#define C_START_Y 9
#define C_START_DIR 1

Block *blocks;
Walls *walls;
Floor *floors;
Stairs *stairs;
Poles *poles;


int globalRoundCounter = 0;
unsigned int globalSeed = 0;

const char *directionNames[] = {"North", "East", "South", "West"};


BawanaCell bawanaCells[12];

void deactivateBawanaWalls()
{
    FILE *fp = fopen("bawanaWalls.txt", "r");
    if (!fp)
    {
        printf("Error opening file bawanaWalls.txt\n");
        return;
    }

    int floor, x, y;
    while (fscanf(fp, "[%d,%d,%d] ", &floor, &y, &x) == 3)
    {
        if (floor >= 0 && floor < 3 && x >= 0 && x < FLOOR_LENGTH && y >= 0 && y < FLOOR_WIDTH)
        {
            int index = floor * (FLOOR_WIDTH * FLOOR_LENGTH) + y * FLOOR_LENGTH + x;
            deactivateBlock(blocks, index);
        }
    }
    fclose(fp);
}

void initializeBawanaCells()
{
    deactivateBawanaWalls();

    int indices[12];
    for (int i = 0; i < 12; i++)
        indices[i] = i;

    
    for (int i = 11; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }

    
    for (int t = 0; t < 4; t++)
    {
        for (int k = 0; k < 2; k++)
        {
            int idx = indices[t * 2 + k];
            bawanaCells[idx].x = 6 + idx % 4;
            bawanaCells[idx].y = 20 + idx / 4;
            bawanaCells[idx].type = t;
        }
    }
    
    for (int i = 8; i < 12; i++)
    {
        int idx = indices[i];
        bawanaCells[idx].x = 6 + idx % 4;
        bawanaCells[idx].y = 20 + idx / 4;
        bawanaCells[idx].type = 4;
    }
}


unsigned int loadSeed(const char *path)
{
    unsigned int seed = 0;
    FILE *fp = fopen(path, "r");
    if (fp)
    {
        if (fscanf(fp, "%u", &seed) != 1)
            seed = (unsigned int)time(NULL);
        fclose(fp);
    }
    else
    {
        seed = (unsigned int)time(NULL);
    }
    globalSeed = seed;
    return seed;
}


int countLines(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return 0;
    int count = 0, ch;
    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
            count++;
    }
    fclose(fp);
    return count;
}


void deactivateBlock(Block blocks[], int index)
{
    blocks[index].isActive = 0;
    printf("Deactivated block: (%d,%d,%d)\n",
           blocks[index].floor,
           blocks[index].y,
           blocks[index].x);
}


void handleStairMidpoint(Stairs *stairs, Block blocks[], int stairIndex, int floorWidth, int floorLength)
{

    float mindexf = (stairs[stairIndex].startX + stairs[stairIndex].endX) / 2.0f;
    float midYf = (stairs[stairIndex].startY + stairs[stairIndex].endY) / 2.0f;
    int midFloor = stairs[stairIndex].endFloor - 1;

    int mindex1 = (int)mindexf;
    int midY1 = (int)midYf;

    if ((mindexf - mindex1 != 0.0f) || (midYf - midY1 != 0.0f))
    {

        if (midFloor >= 0 && midFloor < 3 && mindex1 >= 0 && mindex1 < floorWidth && midY1 >= 0 && midY1 < floorLength)
        {
            int index = midFloor * (floorWidth * floorLength) + midY1 * floorLength + mindex1;
            deactivateBlock(blocks, index);
        }
    }
}


void randomizeStairDirections(Stairs *stairs, int stairsCount)
{
    for (int i = 0; i < stairsCount; i++)
    {
        int randDir = rand() % 3;
        stairs[i].direction = randDir;
    }
}


void loadStairs(const char *filename, Stairs *stairs, Block blocks[], int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    int count = 0;
    printf("Loading stairs from %s...\n", filename);

    while (fscanf(fp, "[%d,%d,%d,%d,%d,%d] ",
                  &stairs[count].startFloor,
                  &stairs[count].startY,
                  &stairs[count].startX,
                  &stairs[count].endFloor,
                  &stairs[count].endY,
                  &stairs[count].endX) == 6)
    {
        stairs[count].direction = 0;

        
        if ((stairs[count].endFloor - stairs[count].startFloor) != 1)
            handleStairMidpoint(stairs, blocks, count, floorWidth, floorLength);

        int startIndex = stairs[count].startFloor * (floorWidth * floorLength) +
                         stairs[count].startY * floorLength + stairs[count].startX;

        int endIndex = stairs[count].endFloor * (floorWidth * floorLength) +
                       stairs[count].endY * floorLength + stairs[count].endX;

        
        blocks[startIndex].blockType = 's';
        blocks[endIndex].blockType = 's';

        count++;
    }

    fclose(fp);
    printf("Total stairs loaded: %d\n", count);
}


void loadPoles(const char *filename, Poles *poles, Block blocks[], int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }
    int count = 0;

    while (fscanf(fp, "[%d,%d,%d,%d] ",
                  &poles[count].startFloor,
                  &poles[count].endFloor,
                  &poles[count].y,
                  &poles[count].x) == 4)
    {
        
        int minFloor = (poles[count].startFloor < poles[count].endFloor) ? poles[count].startFloor : poles[count].endFloor;
        int maxFloor = (poles[count].startFloor > poles[count].endFloor) ? poles[count].startFloor : poles[count].endFloor;

        for (int f = minFloor; f <= maxFloor; f++)
        {
            int index = f * (floorWidth * floorLength) +
                        poles[count].y * floorLength + poles[count].x;
            blocks[index].blockType = 'p';
        }
        count++;
    }

    fclose(fp);
}


int checkObstructingWalls(Walls *walls, int wallIndex, int floorWidth, int floorLength)
{
    FILE *fp = fopen("restrictedAreas.txt", "r");
    if (!fp)
        return 0;

    int f, x, y;

    while (fscanf(fp, "[%d,%d,%d] ", &f, &y, &x) == 3)
    {
        int index = f * (floorWidth * floorLength) + y * floorLength + x;
        if (wallIndex == index)
        {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}


void loadWalls(const char *filename, Walls *walls, Block blocks[], int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    int count = 0;

    while (fscanf(fp, "[%d,%d,%d,%d,%d] ",
                  &walls[count].floor,
                  &walls[count].startY,
                  &walls[count].startX,
                  &walls[count].endY,
                  &walls[count].endX) == 5)
    {
        if (walls[count].endX - walls[count].startX != 0)
        {
            
            for (int x = walls[count].startX; x <= walls[count].endX; x++)
            {
                int index = walls[count].floor * (floorWidth * floorLength) +
                            walls[count].startY * floorLength + x;
                if (!checkObstructingWalls(walls, index, floorWidth, floorLength))
                {
                    deactivateBlock(blocks, index);

                    
                    blocks[index].blockType = 'w';
                }
            }
        }

        else
        {
            
            for (int y = walls[count].startY; y <= walls[count].endY; y++)
            {
                int index = walls[count].floor * (floorWidth * floorLength) +
                            y * floorLength + walls[count].startX;
                if (!checkObstructingWalls(walls, index, floorWidth, floorLength))
                {
                    deactivateBlock(blocks, index);

                    
                    blocks[index].blockType = 'w';
                }
            }
        }
        count++;
    }

    fclose(fp);
}


void handleDeactivation(const char *filename, Block blocks[], int width, int length, int flagIndex)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return;

    int f, x, y;

    while (fscanf(fp, "[%d,%d,%d] ", &f, &y, &x) == 3)
    {
        int index = f * (width * length) + y * length + x;

        if (index == flagIndex)
            continue;
        deactivateBlock(blocks, index);
    }

    fclose(fp);
}


void initializeFloors(Floor floors[], Block blocks[], int width, int length, int flagIndex)
{

    for (int i = 0; i < 3; i++)
    {
        floors[i].floor = i;
        floors[i].width = width;
        floors[i].length = length;

        
        for (int y = 0; y < width; y++)
        {
            
            for (int x = 0; x < length; x++)
            {
                int index = i * (width * length) + y * length + x;
                blocks[index].floor = i;
                blocks[index].x = x;
                blocks[index].y = y;
                blocks[index].isActive = 1;
                blocks[index].blockType = '\0'; 
                blocks[index].value = 0;
                blocks[index].valueType = '\0'; 
            }
        }
    }

    
    handleDeactivation("deactiveBlocks.txt", blocks, width, length, flagIndex);
}


void assignValuesToActiveBlocks(Block blocks[], int width, int length)
{
    int totalCells = 3 * width * length;

    
    int activeCount = 0;
    for (int i = 0; i < totalCells; i++)
    {
        if (blocks[i].isActive)
            activeCount++;
    }

    int consumableZero = activeCount * 25 / 100;
    int consumableSmall = activeCount * 35 / 100;
    int bonusSmall = activeCount * 25 / 100;
    int bonusLarge = activeCount * 10 / 100;
    int bonusMultiplier = activeCount - (consumableZero + consumableSmall + bonusSmall + bonusLarge); 

    
    char *types = (char *)malloc(activeCount);
    int idx = 0;

    
    for (int i = 0; i < consumableZero; i++)
        types[idx++] = 'z'; 
    for (int i = 0; i < consumableSmall; i++)
        types[idx++] = 'c'; 
    for (int i = 0; i < bonusSmall; i++)
        types[idx++] = 'b'; 
    for (int i = 0; i < bonusLarge; i++)
        types[idx++] = 'B'; 
    for (int i = 0; i < bonusMultiplier; i++)
        types[idx++] = 'm'; 

    
    for (int i = activeCount - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        char temp = types[i];
        types[i] = types[j];
        types[j] = temp;
    }

    idx = 0;
    for (int i = 0; i < totalCells; i++)
    {
        if (blocks[i].isActive)
        {
            char t = types[idx++];
            blocks[i].value = 0;
            blocks[i].valueType = '\0';

            if (t == 'z')
            {
                blocks[i].valueType = 'c';
                blocks[i].value = 0;
            }
            else if (t == 'c')
            {
                blocks[i].valueType = 'c';
                blocks[i].value = 1 + rand() % 4;
            }
            else if (t == 'b')
            {
                blocks[i].valueType = 'b';
                blocks[i].value = 1 + rand() % 2;
            }
            else if (t == 'B')
            {
                blocks[i].valueType = 'b';
                blocks[i].value = 3 + rand() % 3;
            }
            else if (t == 'm')
            {
                blocks[i].valueType = 'm';
                blocks[i].value = 2 + rand() % 4;
            }
        }
    }
    free(types);
}


int readFlagPosition(int floorWidth, int floorLength)
{
    FILE *fp = fopen("flag.txt", "r");
    if (!fp)
        return -1;

    int f, x, y;

    fscanf(fp, "[%d,%d,%d] ", &f, &y, &x);

    fclose(fp);

    return f * (floorWidth * floorLength) + y * floorLength + x;
}


void rollMoveDice(int *moveDice, Player *p)
{

    *moveDice = ((rand() % 6) + 1) * p->speed;
}


void rollDirectionDice(int *directionDice)
{

    *directionDice = (rand() % 6) + 1;
}


void sendToBawana(Player *p)
{

    printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->playerName);

    
    int cellIndex = rand() % 12;

    BawanaCell cell = bawanaCells[cellIndex];

    p->floor = 0;
    p->PosX = cell.x;
    p->PosY = cell.y;

    if (cell.type == 0)
    {
        printf("Player %c is place on a Poisonous cell and effects take place.\n", p->playerName);
    }
    else if (cell.type == 1)
    {
        printf("Player %c is place on a Disoriented cell and effects take place.\n", p->playerName);
    }
    else if (cell.type == 2)
    {
        printf("Player %c is place on a Triggered cell and effects take place.\n", p->playerName);
    }
    else if (cell.type == 3)
    {
        printf("Player %c is place on a Happy cell and effects take place.\n", p->playerName);
    }
    else
    {
        printf("Player %c is place on a Random cell and effects take place.\n", p->playerName);
    }

    
    switch (cell.type)
    {
    case 0: 
        p->missTurns = 3;
        p->isFoodPoisoned = 'y';
        printf("Player %c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", p->playerName);
        break;

    case 1: 
        p->movPoints += 50;
        p->disorientedTurns = 4;
        p->isDisoriented = 'y';
        printf("Player %c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", p->playerName);
        break;

    case 2: 
        p->movPoints += 50;
        if (p->speed < 2)
            p->speed *= 2;
        else
            p->speed = 2;
        printf("Player %c eats from Bawana and is triggered. Gets 50 MP.\n", p->playerName);
        break;

    case 3: 
        p->movPoints += 200;
        printf("Player %c eats from Bawana and is happy. Gets 200 MP.\n", p->playerName);
        break;

    case 4: 
    {
        int points = 10 + rand() % 91;
        p->movPoints += points;
        printf("Player %c eats from Bawana and earns %d MP.\n", p->playerName, points);
        break;
    }
    }

    
    p->PosX = 19;
    p->PosY = 9;
    p->direction = 0;
}


double calculateDistanceToFlag(int x, int y, int floor, int flagIndex, int floorWidth, int floorLength)
{
    int flagFloor = flagIndex / (floorWidth * floorLength);
    int temp = flagIndex % (floorWidth * floorLength);
    int flagY = temp / floorLength;
    int flagX = temp % floorLength;

    return abs(x - flagX) + abs(y - flagY) + abs(floor - flagFloor);
}






int findBestStair(int x, int y, int floor, Stairs stairsArr[], int stairsCount,
                  int flagIndex, int floorWidth, int floorLength)
{
    int candidates[2];
    double distances[2];
    int count = 0;

    for (int s = 0; s < stairsCount && count < 2; ++s)
    {
        
        if (floor == stairsArr[s].startFloor && x == stairsArr[s].startX && y == stairsArr[s].startY)
        {
            if (stairsArr[s].direction == 0 || stairsArr[s].direction == 1) 
            {
                candidates[count] = s;
                distances[count] = calculateDistanceToFlag(stairsArr[s].endX, stairsArr[s].endY, stairsArr[s].endFloor,
                                                           flagIndex, floorWidth, floorLength);
                count++;
            }
        }
        
        else if (floor == stairsArr[s].endFloor && x == stairsArr[s].endX && y == stairsArr[s].endY)
        {
            if (stairsArr[s].direction == 0 || stairsArr[s].direction == 2) 
            {
                candidates[count] = s;
                distances[count] = calculateDistanceToFlag(stairsArr[s].startX, stairsArr[s].startY, stairsArr[s].startFloor,
                                                           flagIndex, floorWidth, floorLength);
                count++;
            }
        }
    }

    if (count == 0)
        return -1;
    if (count == 1)
        return candidates[0];

    if (fabs(distances[0] - distances[1]) < 1e-6)
    {
        return candidates[rand() % 2];
    }
    else
    {
        return (distances[0] < distances[1]) ? candidates[0] : candidates[1];
    }
}


void resetPlayerToStart(Player *p)
{

    if (p->playerName == 'A')
    {
        p->PosX = 12;
        p->PosY = 5;
        p->direction = 0;
        p->isStarted = 'n';
    }
    else if (p->playerName == 'B')
    {
        p->PosX = 7;
        p->PosY = 9;
        p->direction = 3;
        p->isStarted = 'n';
    }
    else
    {
        p->PosX = 17;
        p->PosY = 9;
        p->direction = 1;
        p->isStarted = 'n';
    }
    p->floor = 0;
}


int checkIsPlayerOnStartingArea(Player *p)
{
    if (p->floor == 0 && p->PosY >= 5 && p->PosY <= 9 && p->PosX >= 6 && p->PosX <= 10)
        return 1;
    return 0;
}

int canMove(Player *p, int moveDice, Block blocksArr[], int floorWidth, int floorLength, Stairs stairsArr[], int stairsCount, Poles polesArr[], int polesCount, int flagIndex)
{
    if (!p)
        return 0;

    int x = p->PosX;
    int y = p->PosY;
    int floor = p->floor;
    int totalCells = floorWidth * floorLength * FLOORS_COUNT;

    
    int *visited = (int *)calloc(totalCells, sizeof(int));
    if (!visited)
        return 0;

    for (int step = 0; step < moveDice; ++step)
    {
        int newX = x;
        int newY = y;
        int newFloor = floor;

        
        if (p->direction == 0)
            newY--;
        else if (p->direction == 1)
            newX++;
        else if (p->direction == 2)
            newY++;
        else if (p->direction == 3)
            newX--;

        
        if (newX < 0 || newX >= floorLength || newY < 0 || newY >= floorWidth)
        {
            printf("Player %c rolls %d on the movement dice and cannot move; out of bounds at (%d,%d,%d)\n", p->playerName, moveDice, floor, newY, newX);
            free(visited);
            return 0;
        }

        int index = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;

        
        if (!blocksArr[index].isActive || blocksArr[index].blockType == 'w')
        {
            printf("Player %c rolls %d on the movement dice and cannot move; blocked at (%d,%d,%d)\n", p->playerName, moveDice, floor, newY, newX);
            free(visited);
            return 0;
        }

        
        if (visited[index])
        {
            printf("Move check alert: Player %c would enter a cycle at (%d,%d,%d).\n", p->playerName, newFloor, newY, newX);
            free(visited);
            return 0;
        }
        visited[index] = 1;

        
        if (blocksArr[index].blockType == 's')
        {
            int bestStair = findBestStair(newX, newY, newFloor, stairsArr, stairsCount, flagIndex, floorWidth, floorLength);
            if (bestStair >= 0)
            {
                if (newFloor == stairsArr[bestStair].startFloor && newX == stairsArr[bestStair].startX && newY == stairsArr[bestStair].startY)
                {
                    
                    if (stairsArr[bestStair].direction == 0 || stairsArr[bestStair].direction == 1)
                    {
                        newFloor = stairsArr[bestStair].endFloor;
                        newX = stairsArr[bestStair].endX;
                        newY = stairsArr[bestStair].endY;
                    }
                }
                else if (newFloor == stairsArr[bestStair].endFloor && newX == stairsArr[bestStair].endX && newY == stairsArr[bestStair].endY)
                {
                    
                    if (stairsArr[bestStair].direction == 0 || stairsArr[bestStair].direction == 2)
                    {
                        newFloor = stairsArr[bestStair].startFloor;
                        newX = stairsArr[bestStair].startX;
                        newY = stairsArr[bestStair].startY;
                    }
                }
            }
            
            if (newX < 0 || newX >= floorLength || newY < 0 || newY >= floorWidth || newFloor < 0 || newFloor >= FLOORS_COUNT)
            {
                printf("Player %c rolls and %d on the movement dice and cannot move in the %s direction. Player remains at (%d,%d,%d)\n",
                       p->playerName, moveDice, directionNames[p->direction], p->floor, p->PosY, p->PosX);
                free(visited);
                return 0;
            }
            int teleIndex = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;

            if (!blocksArr[teleIndex].isActive || blocksArr[teleIndex].blockType == 'w')
            {
                printf("Player %c rolls and %d on the movement dice and cannot move in the %s direction. Player remains at (%d,%d,%d)\n",
                       p->playerName, moveDice, directionNames[p->direction], p->floor, p->PosY, p->PosX);
                free(visited);
                return 0;
            }
            index = teleIndex;
        }

        
        if (blocksArr[index].blockType == 'p')
        {
            for (int pl = 0; pl < polesCount; ++pl)
            {
                if (newX == polesArr[pl].x && newY == polesArr[pl].y)
                {
                    int top = polesArr[pl].startFloor > polesArr[pl].endFloor ? polesArr[pl].startFloor : polesArr[pl].endFloor;
                    int bottom = (polesArr[pl].startFloor > polesArr[pl].endFloor) ? polesArr[pl].endFloor : polesArr[pl].startFloor;
                    if (newFloor == top)
                    {
                        newFloor = bottom;
                        
                        if (newFloor < 0 || newFloor >= FLOORS_COUNT)
                        {
                            free(visited);
                            return 0;
                        }
                        index = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;
                        if (!blocksArr[index].isActive || blocksArr[index].blockType == 'w')
                        {
                            free(visited);
                            return 0;
                        }
                    }
                    break;
                }
            }
        }

        
        x = newX;
        y = newY;
        floor = newFloor;
    }

    free(visited);

    
    return 1;
}


int movePlayerStep(Player *p, Player playersArr[], Block blocksArr[], int floorWidth, int floorLength,
                   Stairs stairsArr[], int stairsCount, Poles polesArr[], int polesCount, int flagIndex)
{
    if (!p)
        return 0;

    int newX = p->PosX;
    int newY = p->PosY;
    int newFloor = p->floor;

    if (p->direction == 0)
        newY--;
    else if (p->direction == 1)
        newX++;
    else if (p->direction == 2)
        newY++;
    else if (p->direction == 3)
        newX--;

    if (newX < 0 || newX >= floorLength || newY < 0 || newY >= floorWidth)
        return 0;

    int index = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;

    if (!blocksArr[index].isActive || blocksArr[index].blockType == 'w')
        return 0;

    
    for (int i = 0; i < 3; ++i)
    {
        if (&playersArr[i] != p && playersArr[i].isStarted == 'y' &&
            playersArr[i].floor == newFloor &&
            playersArr[i].PosX == newX &&
            playersArr[i].PosY == newY)
        {
            printf("Player %c captures Player %c at (%d,%d,%d)\n", p->playerName, playersArr[i].playerName, newFloor, newY, newX);
            resetPlayerToStart(&playersArr[i]);
            break;
        }
    }

    
    if (blocksArr[index].blockType == 's')
    {
        int bestStair = findBestStair(newX, newY, newFloor, stairsArr, stairsCount, flagIndex, floorWidth, floorLength);
        if (bestStair >= 0)
        {
            if (newFloor == stairsArr[bestStair].startFloor && newX == stairsArr[bestStair].startX && newY == stairsArr[bestStair].startY)
            {
                if (stairsArr[bestStair].direction == 0 || stairsArr[bestStair].direction == 1)
                {
                    printf("Player %c lands on (%d,%d,%d) which is a stair cell.\n", p->playerName, newFloor, newY, newX);
                    newFloor = stairsArr[bestStair].endFloor;
                    newX = stairsArr[bestStair].endX;
                    newY = stairsArr[bestStair].endY;
                    printf("Player %c takes the stairs and now placed at (%d,%d,%d) in floor %d.\n", p->playerName, newFloor, newY, newX, newFloor);
                }
            }
            else if (newFloor == stairsArr[bestStair].endFloor && newX == stairsArr[bestStair].endX && newY == stairsArr[bestStair].endY)
            {
                if (stairsArr[bestStair].direction == 0 || stairsArr[bestStair].direction == 2)
                {
                    printf("Player %c lands on (%d,%d,%d) which is a stair cell.\n", p->playerName, newFloor, newY, newX);
                    newFloor = stairsArr[bestStair].startFloor;
                    newX = stairsArr[bestStair].startX;
                    newY = stairsArr[bestStair].startY;
                    printf("Player %c takes the stairs and now placed at (%d,%d,%d) in floor %d.\n", p->playerName, newFloor, newY, newX, newFloor);
                }
            }
        }
    }

    
    if (blocksArr[index].blockType == 'p')
    {
        for (int pl = 0; pl < polesCount; ++pl)
        {
            if (newX == polesArr[pl].x && newY == polesArr[pl].y)
            {
                int top = polesArr[pl].startFloor > polesArr[pl].endFloor ? polesArr[pl].startFloor : polesArr[pl].endFloor;
                int bottom = (polesArr[pl].startFloor > polesArr[pl].endFloor) ? polesArr[pl].endFloor : polesArr[pl].startFloor;
                if (newFloor == top)
                {
                    printf("Player %c lands on (%d,%d,%d) which is a pole cell.\n", p->playerName, newFloor, newY, newX);
                    newFloor = bottom;
                    printf("Player %c slides down and now placed at (%d,%d,%d) in floor %d.\n", p->playerName, newFloor, newY, newX, newFloor);
                }
                break;
            }
        }
    }

    
    p->PosX = newX;
    p->PosY = newY;
    p->floor = newFloor;
    return 1;
}

void gamePlay(Block blocksArr[], Player playersArr[], int floorWidth, int floorLength, Stairs stairsArr[], int stairsCount, Poles polesArr[], int polesCount)
{
    

    int flagIndex = readFlagPosition(floorWidth, floorLength);
    if (flagIndex == -1)
    {
        fprintf(stderr, "Error reading flag position. Aborting gameplay.\n");
        return;
    }

    int gameOver = 0;
    int turnCounter[3] = {0, 0, 0};
    int wasDirectionChanged = 0;

    
    while (!gameOver)
    {
        globalRoundCounter++;
        if (globalRoundCounter % 5 == 0)
            randomizeStairDirections(stairsArr, stairsCount);

        printf("\n\n----------------------------------------------\n");
        printf("-------------- Round %d --------------\n", globalRoundCounter);
        printf("----------------------------------------------\n\n");

        for (int i = 0; i < 3 && !gameOver; ++i)
        {
            Player *p = &playersArr[i];
            turnCounter[i]++;

            printf("\n--------- Player %c's turn (Turn %d) ---------\n", p->playerName, turnCounter[i]);

            if (p->isStarted == 'n')
            {
                int d;
                rollMoveDice(&d, p);
                if ((d / p->speed) == 6)
                {
                    
                    p->isStarted = 'y';

                    if (p->playerName == 'A')
                    {
                        p->PosX = A_START_X;
                        p->PosY = A_START_Y;
                        p->floor = 0;
                        p->direction = A_START_DIR;
                    }
                    else if (p->playerName == 'B')
                    {
                        p->PosX = B_START_X;
                        p->PosY = B_START_Y;
                        p->floor = 0;
                        p->direction = B_START_DIR;
                    }
                    else
                    {
                        p->PosX = C_START_X;
                        p->PosY = C_START_Y;
                        p->floor = 0;
                        p->direction = C_START_DIR;
                    }
                    printf("Player %c is at the starting area and rolls 6 on the movement dice and is placed on (%d,%d,%d) of the maze.\n", p->playerName, p->floor, p->PosY, p->PosX);
                }
                else
                {
                    printf("Player %c is at the starting area and rolls %d on the movement dice and cannot enter the maze.\n", p->playerName, d);
                }
                continue;
            }

            
            if (p->movPoints <= 0)
            {
                sendToBawana(p);
                continue;
            }
            else if (p->missTurns > 0)
            {
                p->missTurns--;
                printf("Player %c is skipping this turn. Remaining miss turns: %d\n", p->playerName, p->missTurns);
                continue;
            }
            else if (p->isFoodPoisoned == 'y')
            {
                
                p->isFoodPoisoned = 'n';
                printf("Player %c is recovering from food poisoning and will be placed in Bawana.\n", p->playerName);
                sendToBawana(p);
                continue;
            }
            else if (checkIsPlayerOnStartingArea(p) && p->isStarted == 'y')
            {
                printf("Player %c has returned to the starting area and is reset to start position.\n", p->playerName);
                resetPlayerToStart(p);
                p->isStarted = 'n';
                continue;
            }
            else
            {
                
                if (p->disorientedTurns > 0)
                {
                    p->disorientedTurns--;
                    int dirRoll;
                    rollDirectionDice(&dirRoll);
                    p->direction = dirRoll % 4;
                    wasDirectionChanged = 1;
                    printf("Player %c is disoriented and randomly changes direction to %d. Remaining disoriented turns: %d\n", p->playerName, p->direction, p->disorientedTurns);
                }
                else if (p->isDisoriented == 'y' && p->disorientedTurns == 0)
                {
                    p->isDisoriented = 'n';
                    printf("Player %c has recovered from disorientation.\n", p->playerName);
                }

                
                int moveDice;
                rollMoveDice(&moveDice, p);

                
                if (turnCounter[i] % 4 == 0 && p->isDisoriented == 'n')
                {
                    int dirRoll;
                    rollDirectionDice(&dirRoll);
                    wasDirectionChanged = 1;
                    if (dirRoll >= 2 && dirRoll <= 5)
                    {
                        p->direction = dirRoll - 2;
                    }
                }

                
                int success = canMove(p, moveDice, blocksArr, floorWidth, floorLength, stairsArr, stairsCount, polesArr, polesCount, flagIndex);
                if (!success)
                {
                    p->movPoints -= 2;
                    printf("Player %c lost 2 MP for being unable to move. Remaining MP: %d\n", p->playerName, p->movPoints);
                    if (p->movPoints <= 0)
                        sendToBawana(p);
                    continue;
                }

                if (wasDirectionChanged)
                {
                    printf("Player %c rolls %d on the movement dice and is moving in the %s direction after a direction change. Current MP: %d\n", p->playerName, moveDice, directionNames[p->direction], p->movPoints);
                    wasDirectionChanged = 0;
                }
                else
                {
                    printf("Player %c rolls %d on the movement dice and is moving in the %s direction. Current MP: %d\n", p->playerName, moveDice, directionNames[p->direction], p->movPoints);
                }

                
                for (int step = 0; step < moveDice; ++step)
                {
                    if (!movePlayerStep(p, playersArr, blocksArr, floorWidth, floorLength, stairsArr, stairsCount, polesArr, polesCount, flagIndex))
                        break;

                    int index = p->floor * (floorWidth * floorLength) + p->PosY * floorLength + p->PosX;

                    if (blocksArr[index].valueType != '\0')
                    {
                        if (blocksArr[index].valueType == 'c')
                        {
                            p->movPoints -= blocksArr[index].value;
                            printf("Player %c lands on (%d,%d,%d) with consumable %d and is moving in the %s direction. New MP: %d\n", p->playerName, p->floor, p->PosY, p->PosX, blocksArr[index].value, directionNames[p->direction], p->movPoints);
                        }
                        else if (blocksArr[index].valueType == 'b')
                        {
                            p->movPoints += blocksArr[index].value;
                            printf("Player %c lands on (%d,%d,%d) with bonus %d and is moving in the %s direction. New MP: %d\n", p->playerName, p->floor, p->PosY, p->PosX, blocksArr[index].value, directionNames[p->direction], p->movPoints);
                        }
                        else if (blocksArr[index].valueType == 'm')
                        {
                            int maxMovPoints = 250;
                            p->movPoints *= blocksArr[index].value;
                            if (p->movPoints > maxMovPoints)
                                p->movPoints = maxMovPoints;
                            printf("Player %c lands on (%d,%d,%d) with multiplier %d and is moving in the %s direction. New MP: %d\n", p->playerName, p->floor, p->PosY, p->PosX, blocksArr[index].value, directionNames[p->direction], p->movPoints);
                        }
                    }

                    if (p->movPoints <= 0)
                    {
                        sendToBawana(p);
                        break;
                    }

                    
                    int pIndex = p->floor * (floorWidth * floorLength) + p->PosY * floorLength + p->PosX;
                    if (pIndex == flagIndex)
                    {
                        printf("Player %c captured the flag at (%d,%d,%d) — Game Over BABYYY!!!.\n", p->playerName, p->floor, p->PosY, p->PosX);
                        gameOver = 1;
                        break;
                    }
                }
            }
            printf("-----------------------------------------------\n");
        } 
    } 
}

void startGame()
{
    printf("Starting UCSC Maze\n");

    
    unsigned int seed = loadSeed("seed.txt");
    srand(seed);

    int stairsCount = countLines("stairs.txt");
    int polesCount = countLines("poles.txt");
    int wallsCount = countLines("walls.txt");

    if (stairsCount <= 0)
        stairsCount = 0;
    if (polesCount <= 0)
        polesCount = 0;
    if (wallsCount <= 0)
        wallsCount = 0;

    stairs = (Stairs *)calloc((size_t)(stairsCount > 0 ? stairsCount : 1), sizeof(Stairs));
    if (!stairs)
    {
        fprintf(stderr, "Error allocating stairs\n");
        return;
    }

    poles = (Poles *)calloc((size_t)(polesCount > 0 ? polesCount : 1), sizeof(Poles));
    if (!poles)
    {
        fprintf(stderr, "Error allocating poles\n");
        free(stairs);
        return;
    }

    walls = (Walls *)calloc((size_t)(wallsCount > 0 ? wallsCount : 1), sizeof(Walls));
    if (!walls)
    {
        fprintf(stderr, "Error allocating walls\n");
        free(stairs);
        free(poles);
        return;
    }

    floors = (Floor *)calloc(FLOORS_COUNT, sizeof(Floor));
    if (!floors)
    {
        fprintf(stderr, "Error allocating floors\n");
        free(stairs);
        free(poles);
        free(walls);
        return;
    }

    blocks = (Block *)calloc(FLOOR_WIDTH * FLOOR_LENGTH * FLOORS_COUNT, sizeof(Block));
    if (!blocks)
    {
        fprintf(stderr, "Error allocating blocks\n");
        free(stairs);
        free(poles);
        free(walls);
        free(floors);
        return;
    }

    printf("\nInitializing the game board...\n");

    int flagIndex = readFlagPosition(FLOOR_WIDTH, FLOOR_LENGTH);
    if (flagIndex == -1)
    {
        fprintf(stderr, "Warning: flag position not found or invalid. Gameplay will abort if not set.\n");
    }

    initializeFloors(floors, blocks, FLOOR_WIDTH, FLOOR_LENGTH, flagIndex);

    printf("\nLoading game inputs...\n");
    loadStairs("stairs.txt", stairs, blocks, FLOOR_WIDTH, FLOOR_LENGTH);
    loadPoles("poles.txt", poles, blocks, FLOOR_WIDTH, FLOOR_LENGTH);
    loadWalls("walls.txt", walls, blocks, FLOOR_WIDTH, FLOOR_LENGTH);

    initializeBawanaCells();

    
    Player *players = (Player *)malloc(3 * sizeof(Player));
    if (!players)
    {
        fprintf(stderr, "Error allocating players\n");
        free(stairs);
        free(poles);
        free(walls);
        free(floors);
        free(blocks);
        return;
    }
    memset(players, 0, 3 * sizeof(Player));

    
    players[0].playerName = 'A';
    players[0].isStarted = 'n';
    players[0].PosX = A_START_X;
    players[0].PosY = A_START_Y;
    players[0].floor = 0;
    players[0].direction = A_START_DIR;
    players[0].movPoints = 100;
    players[0].speed = 1;
    players[0].missTurns = 0;
    players[0].isFoodPoisoned = 'n';
    players[0].isDisoriented = 'n';
    players[0].disorientedTurns = 0;

    players[1].playerName = 'B';
    players[1].isStarted = 'n';
    players[1].PosX = B_START_X;
    players[1].PosY = B_START_Y;
    players[1].floor = 0;
    players[1].direction = B_START_DIR;
    players[1].movPoints = 100;
    players[1].speed = 1;
    players[1].missTurns = 0;
    players[1].isFoodPoisoned = 'n';
    players[1].isDisoriented = 'n';
    players[1].disorientedTurns = 0;

    players[2].playerName = 'C';
    players[2].isStarted = 'n';
    players[2].PosX = C_START_X;
    players[2].PosY = C_START_Y;
    players[2].floor = 0;
    players[2].direction = C_START_DIR;
    players[2].movPoints = 100;
    players[2].speed = 1;
    players[2].missTurns = 0;
    players[2].isFoodPoisoned = 'n';
    players[2].isDisoriented = 'n';
    players[2].disorientedTurns = 0;

    printf("\nPlayer starting positions:\n");
    printf("Player A at (%d,%d) facing %d\n", players[0].PosX, players[0].PosY, players[0].direction);
    printf("Player B at (%d,%d) facing %d\n", players[1].PosX, players[1].PosY, players[1].direction);
    printf("Player C at (%d,%d) facing %d\n", players[2].PosX, players[2].PosY, players[2].direction);

    printf("\nAssigning values to cells...\n");
    assignValuesToActiveBlocks(blocks, FLOOR_WIDTH, FLOOR_LENGTH);

    printf("\nStarting Gameplay...\n\n");

    if (flagIndex == -1)
    {
        fprintf(stderr, "Cannot start gameplay: flag position invalid.\n");
    }
    else
    {
        gamePlay(blocks, players, FLOOR_WIDTH, FLOOR_LENGTH, stairs, (stairsCount > 0 ? stairsCount : 0), poles, (polesCount > 0 ? polesCount : 0));
    }

    printf("\nGameplay Stopped!\n");

    printf("Cleaning up memory...\n");
    free(stairs);
    free(poles);
    free(walls);
    free(floors);
    free(blocks);
    free(players);

    printf("Game logic finished.\n");
    return;
}

#endif
