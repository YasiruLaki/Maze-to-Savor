#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "headers.h"

BawanaCell bawanaCells[12] = {
    {6, 20, 0}, {7, 20, 0}, {8, 20, 1}, {9, 20, 1}, {6, 21, 2}, {7, 21, 2}, {8, 21, 3}, {9, 21, 3}, {6, 22, 4}, {7, 22, 4}, {8, 22, 4}, {9, 22, 4}};

int globalRoundCounter = 0;

unsigned int globalSeed = 0;

void deactivateBlock(Block blocks[], int index)
{
    blocks[index].isActive = 0;
    printf("Deactivated block: (%d,%d,%d)\n",
           blocks[index].floor,
           blocks[index].y,
           blocks[index].x);
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
        printf("Stair %d direction set to %s\n", i,
               (randDir == 0) ? "bidirectional" : (randDir == 1) ? "up only"
                                                                 : "down only");
    }
}

void loadStairs(const char *filename, Stairs *stairs, Block blocks[], int stairsCount, int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    int count = 0;
    printf("Loading stairs from %s...\n", filename);

    while (count < stairsCount &&
           fscanf(fp, "[%d,%d,%d,%d,%d,%d] ",
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

void loadPoles(const char *filename, Poles *poles, Block blocks[], int polesCount, int floorWidth, int floorLength)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }
    int count = 0;

    while (count < polesCount &&
           fscanf(fp, "[%d,%d,%d,%d] ",
                  &poles[count].startFloor,
                  &poles[count].endFloor,
                  &poles[count].y,
                  &poles[count].x) == 4)
    {
        int index = poles[count].startFloor * (floorWidth * floorLength) +
                    poles[count].y * floorLength + poles[count].x;
        blocks[index].blockType = 'p';
        count++;
    }
    fclose(fp);
}

int checkObstructingWalls(Walls *walls, int wallIndex, int floorWidth, int floorLength)
{
    FILE *fp = fopen("startingArea.txt", "r");
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

void loadWalls(const char *filename, Walls *walls, Block blocks[], int wallsCount, int floorWidth, int floorLength)
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
                blocks[index].consumable = 0;
                blocks[index].bonus = 0;

                int roll = rand() % 100;
                if (roll < 10)
                    blocks[index].consumable = (rand() % 2 == 0) ? 5 : -3;
                else if (roll < 15)
                    blocks[index].bonus = (rand() % 2 == 0) ? 3 : 1;
            }
        }
    }
    handleDeactivation("deactiveBlocks.txt", blocks, width, length, flagIndex);
}

int readFlagPosition(int floorWidth, int floorLength)
{
    FILE *fp = fopen("inputs/flag.txt", "r");
    if (!fp)
        return -1;
    int f, x, y;
    fscanf(fp, "[%d,%d,%d] ", &f, &y, &x);
    fclose(fp);
    return f * (floorWidth * floorLength) + y * floorLength + x;
}

void rollMoveDice(int *moveDice) { *moveDice = (rand() % 6) + 1; }
void rollDirectionDice(int *directionDice) { *directionDice = (rand() % 6) + 1; }

void sendToBawana(Player *p)
{
    p->floor = 0;
    p->PosX = 19;
    p->PosY = 9;
    p->direction = 0;
    printf("Player %c movement points depleted. Transporting to Bawana.\n", p->playerName);

    int cellIndex = rand() % 12;
    BawanaCell cell = bawanaCells[cellIndex];

    p->PosX = cell.x;
    p->PosY = cell.y;

    switch (cell.type)
    {
    case 0:
        p->missTurns = 3;
        printf("Player %c eats from Bawana and has food poisoning. Will miss 3 turns.\n", p->playerName);
        break;
    case 1:
        p->movPoints = 50;
        p->disorientedTurns = 4;
        printf("Player %c eats from Bawana and is disoriented. Gets 50 MP.\n", p->playerName);
        break;
    case 2:
        p->movPoints = 50;
        p->triggeredTurns = 4;
        printf("Player %c eats from Bawana and is triggered. Gets 50 MP.\n", p->playerName);
        break;
    case 3:
        p->movPoints = 200;
        printf("Player %c eats from Bawana and is happy. Gets 200 MP.\n", p->playerName);
        break;
    case 4:
    {
        int points = 10 + rand() % 91;
        p->movPoints = points;
        printf("Player %c eats from Bawana and earns %d MP.\n", p->playerName, points);
        break;
    }
    }
}

double calculateDistanceToFlag(int x, int y, int floor, int flagIndex, int floorWidth, int floorLength)
{
    int flagFloor = flagIndex / (floorWidth * floorLength);
    int temp = flagIndex % (floorWidth * floorLength);
    int flagY = temp / floorLength;
    int flagX = temp % floorLength;

    return sqrt(pow(x - flagX, 2) + pow(y - flagY, 2) + pow(floor - flagFloor, 2));
}

int findBestStair(int x, int y, int floor, Stairs stairs[], int stairsCount, int flagIndex, int floorWidth, int floorLength, int canGoUp)
{
    int bestStair = -1;
    double minDistance = 999999.0;
    int validStairs[10];
    int validCount = 0;

    for (int s = 0; s < stairsCount; s++)
    {
        if (floor == stairs[s].startFloor && x == stairs[s].startX && y == stairs[s].startY)
        {
            if (canGoUp && (stairs[s].direction == 0 || stairs[s].direction == 1))
            {
                double dist = calculateDistanceToFlag(stairs[s].endX, stairs[s].endY, stairs[s].endFloor,
                                                      flagIndex, floorWidth, floorLength);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    bestStair = s;
                }
                validStairs[validCount++] = s;
            }
        }
        else if (floor == stairs[s].endFloor && x == stairs[s].endX && y == stairs[s].endY)
        {
            if (!canGoUp && (stairs[s].direction == 0 || stairs[s].direction == 2))
            {
                double dist = calculateDistanceToFlag(stairs[s].startX, stairs[s].startY, stairs[s].startFloor,
                                                      flagIndex, floorWidth, floorLength);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    bestStair = s;
                }
                validStairs[validCount++] = s;
            }
        }
    }

    if (validCount > 1)
    {
        int tieCount = 0;
        for (int i = 0; i < validCount; i++)
        {
            double dist;
            int s = validStairs[i];
            if (floor == stairs[s].startFloor && x == stairs[s].startX && y == stairs[s].startY)
            {
                dist = calculateDistanceToFlag(stairs[s].endX, stairs[s].endY, stairs[s].endFloor,
                                               flagIndex, floorWidth, floorLength);
            }
            else
            {
                dist = calculateDistanceToFlag(stairs[s].startX, stairs[s].startY, stairs[s].startFloor,
                                               flagIndex, floorWidth, floorLength);
            }

            if (fabs(dist - minDistance) < 0.01)
            {
                tieCount++;
            }
        }

        if (tieCount > 1)
        {

            bestStair = validStairs[rand() % validCount];
        }
    }

    return bestStair;
}

int canMove(Player *p, int moveDice, Block blocks[], int floorWidth, int floorLength,
            Stairs stairs[], int stairsCount, Poles poles[], int polesCount, int flagIndex)
{
    int x = p->PosX;
    int y = p->PosY;
    int floor = p->floor;

    int maxSize = floorWidth * floorLength * 3;

    int *visited = calloc(maxSize, sizeof(int));

    if (!visited)
        return 0;

    for (int step = 0; step < moveDice; step++)
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
            printf("canMove: Player %c out of bounds at (%d,%d,%d)\n", p->playerName, newFloor, newY, newX);
            free(visited);
            return 0;
        }

        int index = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;

        if (!blocks[index].isActive || blocks[index].blockType == 'w')
        {
            printf("canMove: Player %c blocked at (%d,%d,%d) index %d - isActive=%d blockType=%c\n",
                   p->playerName, newFloor, newY, newX, index, blocks[index].isActive, blocks[index].blockType ? blocks[index].blockType : '\0');
            free(visited);
            return 0;
        }

        if (visited[index])
        {
            printf("canMove: Player %c would loop at index %d (pos %d,%d,%d)\n", p->playerName, index, newFloor, newY, newX);
            free(visited);
            return 0;
        }
        visited[index] = 1;

        if (blocks[index].blockType == 's')
        {

            int isAtStart = 0;
            for (int s2 = 0; s2 < stairsCount; s2++)
            {
                if (newFloor == stairs[s2].startFloor && newX == stairs[s2].startX && newY == stairs[s2].startY)
                {
                    isAtStart = 1;
                    break;
                }
            }

            int bestStair = findBestStair(newX, newY, newFloor, stairs, stairsCount, flagIndex, floorWidth, floorLength, isAtStart);
            if (bestStair >= 0)
            {
                if (newFloor == stairs[bestStair].startFloor && newX == stairs[bestStair].startX && newY == stairs[bestStair].startY)
                {
                    newFloor = stairs[bestStair].endFloor;
                    newX = stairs[bestStair].endX;
                    newY = stairs[bestStair].endY;
                }
                else if (newFloor == stairs[bestStair].endFloor && newX == stairs[bestStair].endX && newY == stairs[bestStair].endY)
                {
                    newFloor = stairs[bestStair].startFloor;
                    newX = stairs[bestStair].startX;
                    newY = stairs[bestStair].startY;
                }
            }
        }

        if (blocks[index].blockType == 'p')
        {
            for (int pl = 0; pl < polesCount; pl++)
            {
                if (newFloor == poles[pl].startFloor && newX == poles[pl].x && newY == poles[pl].y)
                {
                    newFloor = poles[pl].endFloor;
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

int movePlayerStep(Player *p, Block blocks[], int floorWidth, int floorLength,
                   Stairs stairs[], int stairsCount, Poles poles[], int polesCount, int flagIndex)
{
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

    if (!blocks[index].isActive || blocks[index].blockType == 'w')
        return 0;

    if (blocks[index].blockType == 's')
    {

        int isAtStart = 0;
        for (int s2 = 0; s2 < stairsCount; s2++)
        {
            if (newFloor == stairs[s2].startFloor && newX == stairs[s2].startX && newY == stairs[s2].startY)
            {
                isAtStart = 1;
                break;
            }
        }

        int bestStair = findBestStair(newX, newY, newFloor, stairs, stairsCount, flagIndex, floorWidth, floorLength, isAtStart);
        if (bestStair >= 0)
        {
            if (newFloor == stairs[bestStair].startFloor && newX == stairs[bestStair].startX && newY == stairs[bestStair].startY)
            {
                if (stairs[bestStair].direction == 0 || stairs[bestStair].direction == 1)
                {
                    newFloor = stairs[bestStair].endFloor;
                    newX = stairs[bestStair].endX;
                    newY = stairs[bestStair].endY;
                    printf("Player %c took stairs UP to (%d,%d,%d)\n", p->playerName, newFloor, newY, newX);
                }
            }
            else if (newFloor == stairs[bestStair].endFloor && newX == stairs[bestStair].endX && newY == stairs[bestStair].endY)
            {
                if (stairs[bestStair].direction == 0 || stairs[bestStair].direction == 2)
                {
                    newFloor = stairs[bestStair].startFloor;
                    newX = stairs[bestStair].startX;
                    newY = stairs[bestStair].startY;
                    printf("Player %c took stairs DOWN to (%d,%d,%d)\n", p->playerName, newFloor, newY, newX);
                }
            }
        }
    }

    if (blocks[index].blockType == 'p')
    {
        for (int pl = 0; pl < polesCount; pl++)
        {
            if (newFloor == poles[pl].startFloor && newX == poles[pl].x && newY == poles[pl].y)
            {
                newFloor = poles[pl].endFloor;
                printf("Player %c took pole to floor %d\n", p->playerName, newFloor);
                break;
            }
        }
    }

    p->PosX = newX;
    p->PosY = newY;
    p->floor = newFloor;

    return 1;
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

void gamePlay(Block blocks[], Player players[], int floorWidth, int floorLength,
              Stairs stairs[], int stairsCount, Poles poles[], int polesCount)
{
    srand(globalSeed);

    int flagIndex = readFlagPosition(floorWidth, floorLength);
    if (flagIndex == -1)
        return;

    int gameOver = 0;
    int turnCounter[3] = {0};

    while (!gameOver)
    {
        globalRoundCounter++;
        if (globalRoundCounter % 5 == 0)
            randomizeStairDirections(stairs, stairsCount);

        for (int i = 0; i < 3 && !gameOver; i++)
        {
            Player *p = &players[i];
            turnCounter[i]++;

            if (p->isStarted == 'n')
            {
                int d;
                rollMoveDice(&d);
                if (d == 6)
                    resetPlayerToStart(p), p->isStarted = 'y';
                continue;
            }

            int moveDice;
            rollMoveDice(&moveDice);
            if (turnCounter[i] % 4 == 0)
            {
                int dirRoll;
                rollDirectionDice(&dirRoll);
                if (dirRoll >= 2 && dirRoll <= 5)
                    p->direction = dirRoll - 2;
            }

            int success = canMove(p, moveDice, blocks, floorWidth, floorLength,
                                  stairs, stairsCount, poles, polesCount, flagIndex);
            if (!success)
            {
                p->movPoints -= 2;
                continue;
            }

            for (int step = 0; step < moveDice; step++)
            {
                if (!movePlayerStep(p, blocks, floorWidth, floorLength,
                                    stairs, stairsCount, poles, polesCount, flagIndex))
                    break;

                p->movPoints--;
                int index = p->floor * (floorWidth * floorLength) + p->PosY * floorLength + p->PosX;

                if (blocks[index].consumable != 0)
                {
                    p->movPoints += blocks[index].consumable;
                    blocks[index].consumable = 0;
                }
                if (blocks[index].bonus != 0)
                {
                    if (blocks[index].bonus > 1)
                        p->movPoints += blocks[index].bonus;
                    else
                        p->speed++;
                    blocks[index].bonus = 0;
                }
                if (p->movPoints <= 0)
                {
                    sendToBawana(p);
                    break;
                }
            }

            for (int j = 0; j < 3; j++)
                if (i != j)
                {
                    Player *q = &players[j];
                    if (q->isStarted == 'y' && p->floor == q->floor &&
                        p->PosX == q->PosX && p->PosY == q->PosY)
                        resetPlayerToStart(q);
                }

            int pindex = p->floor * (floorWidth * floorLength) + p->PosY * floorLength + p->PosX;
            if (pindex == flagIndex)
            {
                printf("Player %c wins!\n", p->playerName);
                gameOver = 1;
            }
        }
    }
}