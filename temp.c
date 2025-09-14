#ifndef GAMELOGIC_H
#define GAMELOGIC_H


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "headers.h"

#define FLOOR_WIDTH 10
#define FLOOR_LENGTH 25

Block *blocks;
Walls *walls;
Floor *floors;
Stairs *stairs;
Poles *poles;

// global variables
int globalRoundCounter = 0;
unsigned int globalSeed = 0;

// Configuring Bawana Cells
BawanaCell bawanaCells[12];

void initializeBawanaCells()
{
    int indices[12];
    for (int i = 0; i < 12; i++)
        indices[i] = i;

    // Shuffle indices
    for (int i = 11; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }

    // Assign two of each type (0-3)
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
    // Remaining 4 cells: type 4 (random MP)
    for (int i = 8; i < 12; i++)
    {
        int idx = indices[i];
        bawanaCells[idx].x = 6 + idx % 4;
        bawanaCells[idx].y = 20 + idx / 4;
        bawanaCells[idx].type = 4;
    }
}

// Load seed from the file and assign to globalSeed
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

// Returns the number of lines in a file (or 0 on error)
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

// common function to deactivate a block
void deactivateBlock(Block blocks[], int index)
{
    blocks[index].isActive = 0;
    printf("Deactivated block: (%d,%d,%d)\n",
           blocks[index].floor,
           blocks[index].y,
           blocks[index].x);
}

// Handle midpoints for stairs that span multiple floors
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

// Randomize stair directions every 5 rounds
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

// Load stairs from file and update blocks
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

        // Handle midpoints for stairs spanning multiple floors
        if ((stairs[count].endFloor - stairs[count].startFloor) != 1)
            handleStairMidpoint(stairs, blocks, count, floorWidth, floorLength);

        int startIndex = stairs[count].startFloor * (floorWidth * floorLength) +
                         stairs[count].startY * floorLength + stairs[count].startX;

        int endIndex = stairs[count].endFloor * (floorWidth * floorLength) +
                       stairs[count].endY * floorLength + stairs[count].endX;

        // Mark stair blocks in the blocks array
        blocks[startIndex].blockType = 's';
        blocks[endIndex].blockType = 's';

        count++;
    }

    fclose(fp);
    printf("Total stairs loaded: %d\n", count);
}

// Load poles from file and update blocks
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
        // Mark all floors between startFloor and endFloor (exclusive) as pole access points
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

// Check if a wall obstructs a block
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

// Load walls from file and update blocks
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
            // Horizontal wall
            for (int x = walls[count].startX; x <= walls[count].endX; x++)
            {
                int index = walls[count].floor * (floorWidth * floorLength) +
                            walls[count].startY * floorLength + x;
                if (!checkObstructingWalls(walls, index, floorWidth, floorLength))
                {
                    deactivateBlock(blocks, index);

                    // Mark wall block in the blocks array
                    blocks[index].blockType = 'w';
                }
            }
        }

        else
        {
            // Vertical wall
            for (int y = walls[count].startY; y <= walls[count].endY; y++)
            {
                int index = walls[count].floor * (floorWidth * floorLength) +
                            y * floorLength + walls[count].startX;
                if (!checkObstructingWalls(walls, index, floorWidth, floorLength))
                {
                    deactivateBlock(blocks, index);

                    // Mark wall block in the blocks array
                    blocks[index].blockType = 'w';
                }
            }
        }
        count++;
    }

    fclose(fp);
}

// Handle deactivation of blocks from file
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

// Initialize floors and blocks
void initializeFloors(Floor floors[], Block blocks[], int width, int length, int flagIndex)
{

    for (int i = 0; i < 3; i++)
    {
        floors[i].floor = i;
        floors[i].width = width;
        floors[i].length = length;

        // Initialize blocks for this floor
        for (int y = 0; y < width; y++)
        {
            // Initialize each block
            for (int x = 0; x < length; x++)
            {
                int index = i * (width * length) + y * length + x;
                blocks[index].floor = i;
                blocks[index].x = x;
                blocks[index].y = y;
                blocks[index].isActive = 1;
                blocks[index].blockType = '\0'; // No special type initially
                blocks[index].value = 0;
                blocks[index].valueType = '\0'; // No consumable or bonus initially
            }
        }
    }

    // Deactivate blocks as per deactivation file
    handleDeactivation("deactiveBlocks.txt", blocks, width, length, flagIndex);
}

// function to assign values to only active blocks
void assignValuesToActiveBlocks(Block blocks[], int width, int length)
{
    int totalCells = 3 * width * length;

    // Count only active blocks
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
    int bonusMultiplier = activeCount - (consumableZero + consumableSmall + bonusSmall + bonusLarge); // 5%

    // Create an array to store the type for each active cell
    char *types = malloc(activeCount);
    int idx = 0;

    // Fill types array according to distribution
    for (int i = 0; i < consumableZero; i++)
        types[idx++] = 'z'; // zero consumable
    for (int i = 0; i < consumableSmall; i++)
        types[idx++] = 'c'; // small consumable
    for (int i = 0; i < bonusSmall; i++)
        types[idx++] = 'b'; // small bonus
    for (int i = 0; i < bonusLarge; i++)
        types[idx++] = 'B'; // large bonus
    for (int i = 0; i < bonusMultiplier; i++)
        types[idx++] = 'm'; // multiplier

    // Shuffle the types array
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

// Read flag position from file
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

// Dice rolling functions
void rollMoveDice(int *moveDice, Player *p)
{

    *moveDice = ((rand() % 6) + 1) * p->speed;
}

// Dice rolling functions
void rollDirectionDice(int *directionDice)
{

    *directionDice = (rand() % 6) + 1;
}

// Send player to Bawana and apply effects
void sendToBawana(Player *p)
{

    printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->playerName);

    // Randomly select a Bawana cell
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

    // Apply effects based on cell type
    switch (cell.type)
    {
    case 0: // Poisonous
        p->missTurns = 3;
        p->isFoodPoisoned = 'y';
        printf("Player %c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", p->playerName);
        break;

    case 1: // Disoriented
        p->movPoints += 50;
        p->disorientedTurns = 4;
        p->isDisoriented = 'y';
        printf("Player %c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", p->playerName);
        break;

    case 2: // Triggered
        p->movPoints += 50;
        p->speed *= 2;
        printf("Player %c eats from Bawana and is triggered. Gets 50 MP.\n", p->playerName);
        break;

    case 3: // Happy
        p->movPoints += 200;
        printf("Player %c eats from Bawana and is happy. Gets 200 MP.\n", p->playerName);
        break;

    case 4: // Random
    {
        int points = 10 + rand() % 91;
        p->movPoints += points;
        printf("Player %c eats from Bawana and earns %d MP.\n", p->playerName, points);
        break;
    }
    }

    // set player to the entrance cell
    p->PosX = 19;
    p->PosY = 9;
    p->direction = 0;
}

// Calculate manhattan distance to flag
double calculateDistanceToFlag(int x, int y, int floor, int flagIndex, int floorWidth, int floorLength)
{
    int flagFloor = flagIndex / (floorWidth * floorLength);
    int temp = flagIndex % (floorWidth * floorLength);
    int flagY = temp / floorLength;
    int flagX = temp % floorLength;

    return abs(x - flagX) + abs(y - flagY) + abs(floor - flagFloor);
}

/*
 * Find the best stair to take based on distance to flag.
 * If there are multiple stairs at the current cell, consider all of them and pick the one that gets closest to the flag.
 * If there is a tie, pick randomly among the best.
 */

int findBestStair(int x, int y, int floor, Stairs stairs[], int stairsCount,
                  int flagIndex, int floorWidth, int floorLength, int isAtStart)
{
    int candidates[2]; // at most two stairs
    double distances[2];
    int count = 0;

    // collect usable stairs from this cell
    for (int s = 0; s < stairsCount && count < 2; s++)
    {
        if (floor == stairs[s].startFloor && x == stairs[s].startX && y == stairs[s].startY)
        {
            if (stairs[s].direction == 0 || stairs[s].direction == 1) // bidirectional or up
            {
                candidates[count] = s;
                distances[count] = calculateDistanceToFlag(stairs[s].endX, stairs[s].endY, stairs[s].endFloor,
                                                           flagIndex, floorWidth, floorLength);
                count++;
            }
        }
        else if (floor == stairs[s].endFloor && x == stairs[s].endX && y == stairs[s].endY)
        {
            if (stairs[s].direction == 0 || stairs[s].direction == 2) // bidirectional or down
            {
                candidates[count] = s;
                distances[count] = calculateDistanceToFlag(stairs[s].startX, stairs[s].startY, stairs[s].startFloor,
                                                           flagIndex, floorWidth, floorLength);
                count++;
            }
        }
    }

    if (count == 0)
        return -1; // no usable stairs
    if (count == 1)
        return candidates[0]; // only one option

    // if two stairs exist, compare distances
    if (fabs(distances[0] - distances[1]) < 1e-6)
    {
        // tie → pick randomly
        return candidates[rand() % 2];
    }
    else
    {
        // pick closer
        return (distances[0] < distances[1]) ? candidates[0] : candidates[1];
    }
}

// Check if player can move the given number of steps
int canMove(Player *p, int moveDice, Block blocks[], int floorWidth, int floorLength, Stairs stairs[], int stairsCount, Poles poles[], int polesCount, int flagIndex)
{
    int x = p->PosX;
    int y = p->PosY;
    int floor = p->floor;

    // To avoid cycles, keep track of visited blocks
    // Calculate maximum size for visited array
    int maxSize = floorWidth * floorLength * 3;

    // Allocate visited array
    int *visited = calloc(maxSize, sizeof(int));

    if (!visited)
        return 0;

    for (int step = 0; step < moveDice; step++)
    {
        // Calculate new position based on direction
        int newX = x;
        int newY = y;
        int newFloor = floor;

        // Move in the current direction
        if (p->direction == 0)
            newY--;
        else if (p->direction == 1)
            newX++;
        else if (p->direction == 2)
            newY++;
        else if (p->direction == 3)
            newX--;

        // Check bounds
        if (newX < 0 || newX >= floorLength || newY < 0 || newY >= floorWidth)
        {
            printf("Move check alert: Player %c out of bounds at (%d,%d,%d). Player remains at current position (%d,%d,%d)\n", p->playerName, newFloor, newY, newX, floor, y, x);

            free(visited);
            return 0;
        }

        int index = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;

        // Check if block is active and not a wall
        if (!blocks[index].isActive || blocks[index].blockType == 'w')
        {
            printf("Player %c rolls %d on the movement dice and cannot move in the %d direction. Player remains at cell (%d,%d,%d)\n", p->playerName, moveDice, p->direction, floor, y, x);

            free(visited);
            return 0;
        }

        // Check for cycles
        if (visited[index])
        {
            printf("Move check alert: Player %c would enter a cycle at (%d,%d,%d). Player remains at current position (%d,%d,%d)\n", p->playerName, newFloor, newY, newX, floor, y, x);
            free(visited);
            return 0;
        }
        visited[index] = 1;

        // Handle stairs
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

        // Handle poles
        if (blocks[index].blockType == 'p')
        {
            for (int pl = 0; pl < polesCount; pl++)
            {
                // Handle both start and end floor as pole entry points (mid floor poles as well)
                if ((newFloor == poles[pl].startFloor && newX == poles[pl].x && newY == poles[pl].y) ||
                    (newFloor == poles[pl].endFloor && newX == poles[pl].x && newY == poles[pl].y))
                {
                    // Slide to the other end of the pole
                    if (newFloor == poles[pl].startFloor)
                        newFloor = poles[pl].endFloor;
                    else
                        newFloor = poles[pl].startFloor;
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

int movePlayerStep(Player *p, Player players[], Block blocks[], int floorWidth, int floorLength,
                   Stairs stairs[], int stairsCount, Poles poles[], int polesCount, int flagIndex)
{
    int newX = p->PosX;
    int newY = p->PosY;
    int newFloor = p->floor;

    // Move in the current direction
    if (p->direction == 0)
        newY--;
    else if (p->direction == 1)
        newX++;
    else if (p->direction == 2)
        newY++;
    else if (p->direction == 3)
        newX--;

    // Check bounds
    if (newX < 0 || newX >= floorLength || newY < 0 || newY >= floorWidth)
        return 0;

    int index = newFloor * (floorWidth * floorLength) + newY * floorLength + newX;

    // Check if block is active and not a wall
    if (!blocks[index].isActive || blocks[index].blockType == 'w')
        return 0;

    // Check for collisions with other players
    for (int i = 0; i < 3; i++)
    {
        if (&players[i] != p && players[i].isStarted == 'y' &&
            players[i].floor == newFloor &&
            players[i].PosX == newX &&
            players[i].PosY == newY)
        {
            // Collision detected
            printf("Move collision alert: Player %c would collide with Player %c at (%d,%d,%d). Player remains at current position (%d,%d,%d)\n",
                   p->playerName, players[i].playerName, newFloor, newY, newX, p->floor, p->PosY, p->PosX);
            return 0;
        }
    }

    // Handle stairs
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

        // Find the best stair to take
        int bestStair = findBestStair(newX, newY, newFloor, stairs, stairsCount, flagIndex, floorWidth, floorLength, isAtStart);
        if (bestStair >= 0)
        {

            if (newFloor == stairs[bestStair].startFloor && newX == stairs[bestStair].startX && newY == stairs[bestStair].startY)
            {
                // Can go up if stair is bidirectional or up only
                if (stairs[bestStair].direction == 0 || stairs[bestStair].direction == 1)
                {
                    printf("Player %c lands on (%d,%d,%d) which is a stair cell.\n", p->playerName, newFloor, newY, newX);
                    newFloor = stairs[bestStair].endFloor;
                    newX = stairs[bestStair].endX;
                    newY = stairs[bestStair].endY;
                    printf("Player %c takes the stairs and now placed at (%d,%d,%d) in floor %d.\n", p->playerName, newFloor, newY, newX, newFloor);
                }
            }
            else if (newFloor == stairs[bestStair].endFloor && newX == stairs[bestStair].endX && newY == stairs[bestStair].endY)
            {
                // Can go down if stair is bidirectional or down only
                if (stairs[bestStair].direction == 0 || stairs[bestStair].direction == 2)
                {
                    printf("Player %c lands on (%d,%d,%d) which is a stair cell.\n", p->playerName, newFloor, newY, newX);
                    newFloor = stairs[bestStair].startFloor;
                    newX = stairs[bestStair].startX;
                    newY = stairs[bestStair].startY;
                    printf("Player %c takes the stairs and now placed at (%d,%d,%d) in floor %d.\n", p->playerName, newFloor, newY, newX, newFloor);
                }
            }
        }
    }

    // Handle poles
    if (blocks[index].blockType == 'p')
    {
        for (int pl = 0; pl < polesCount; pl++)
        {
            if (newFloor == poles[pl].startFloor && newX == poles[pl].x && newY == poles[pl].y)
            {
                printf("Player %c lands on (%d,%d,%d) which is a pole cell.\n", p->playerName, newFloor, newY, newX);
                newFloor = poles[pl].endFloor;
                printf("Player %c slides down and now placed at (%d,%d,%d) in floor %d.\n", p->playerName, newFloor, newY, newX, newFloor);
                break;
            }
        }
    }

    p->PosX = newX;
    p->PosY = newY;
    p->floor = newFloor;

    return 1;
}

// Reset player to starting position
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

// Main game loop
void gamePlay(Block blocks[], Player players[], int floorWidth, int floorLength, Stairs stairs[], int stairsCount, Poles poles[], int polesCount)
{

    // Seed the random number generator
    srand(globalSeed);

    // Read flag position
    int flagIndex = readFlagPosition(floorWidth, floorLength);

    if (flagIndex == -1)
        return;

    // check if the game is over
    int gameOver = 0;

    // Turn counters for each player
    int turnCounter[3] = {0};

    int wasDirectionChanged = 0;

    while (!gameOver) // Each iteration is a round. check if game is over
    {
        // Increment global round counter and randomize stair directions every 5 rounds
        globalRoundCounter++;
        if (globalRoundCounter % 5 == 0)
            randomizeStairDirections(stairs, stairsCount);

        printf("\n\n----------------------------------------------\n");
        printf("-------------- Round %d --------------\n", globalRoundCounter);
        printf("----------------------------------------------\n\n");

        // Each player takes a turn
        for (int i = 0; i < 3 && !gameOver; i++)
        {
            Player *p = &players[i];
            turnCounter[i]++;

            printf("\n--------- Player %c's turn (Turn %d) ---------\n", p->playerName, turnCounter[i]);

            // check if the game is started for the player
            if (p->isStarted == 'n')
            {
                int d;
                rollMoveDice(&d, p);
                if ((d / p->speed) == 6)
                {
                    resetPlayerToStart(p);
                    p->isStarted = 'y';
                    printf("Player %c is at the starting area and rolls 6 on the movement dice and is placed on (%d,%d,%d) of the maze.\n", p->playerName, p->floor, p->PosY, p->PosX);
                }
                else
                {
                    printf("Player %c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", p->playerName, d);
                }
                continue;
            }
            else
            {
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
                    printf("Player %c is now fit to proceed from the food poisoning episode.\n", p->playerName);
                    sendToBawana(p);
                    continue;
                }
                else
                {
                    // Handle disorientation
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

                    // Roll movement dice
                    int moveDice;
                    rollMoveDice(&moveDice, p);

                    // Check for automatic direction change every 4 turns if not disoriented
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

                    // Check if the player can move the rolled number of steps
                    int success = canMove(p, moveDice, blocks, floorWidth, floorLength, stairs, stairsCount, poles, polesCount, flagIndex);

                    // If not, lose 2 MP and skip turn
                    if (!success)
                    {
                        p->movPoints -= 2;
                        printf("Player %c lost 2 MP for being unable to move. Remaining MP: %d\n", p->playerName, p->movPoints);
                        continue;
                    }

                    // Execute movement step by step
                    for (int step = 0; step < moveDice; step++)
                    {
                        if (!movePlayerStep(p, players, blocks, floorWidth, floorLength,
                                            stairs, stairsCount, poles, polesCount, flagIndex))
                            break;

                        int index = p->floor * (floorWidth * floorLength) + p->PosY * floorLength + p->PosX;

                        if (blocks[index].valueType != '\0')
                        {
                            if (blocks[index].valueType == 'c')
                            {
                                p->movPoints -= blocks[index].value;
                                printf("Player %c lands on (%d,%d,%d) which has a consumable of value %d. New MP: %d\n", p->playerName, p->floor, p->PosY, p->PosX, blocks[index].value, p->movPoints);
                            }
                            else if (blocks[index].valueType == 'b')
                            {
                                p->movPoints += blocks[index].value;
                                printf("Player %c lands on (%d,%d,%d) which has a bonus of value %d. New MP: %d\n", p->playerName, p->floor, p->PosY, p->PosX, blocks[index].value, p->movPoints);
                            }
                            else if (blocks[index].valueType == 'm')
                            {
                                int maxMovPoints = 1000;
                                p->movPoints *= blocks[index].value;
                                if (p->movPoints > maxMovPoints)
                                {
                                    p->movPoints = maxMovPoints;
                                }
                                printf("Player %c lands on (%d,%d,%d) which has a multiplier of value %d. New MP: %d\n", p->playerName, p->floor, p->PosY, p->PosX, blocks[index].value, p->movPoints);
                            }
                        }

                        if (p->movPoints <= 0)
                        {
                            sendToBawana(p);
                            break;
                        }
                    }

                    if (wasDirectionChanged)
                    {
                        printf("Player %c rolls %d on the movement dice and moves to (%d,%d,%d) in direction %d after changing direction.\n", p->playerName, moveDice, p->floor, p->PosY, p->PosX, p->direction);
                        wasDirectionChanged = 0;
                    }
                    else
                    {
                        printf("Player %c rolls %d on the movement dice and moves to (%d,%d,%d) in direction %d.\n", p->playerName, moveDice, p->floor, p->PosY, p->PosX, p->direction);
                    }

                    // Check for collisions with other players
                    for (int j = 0; j < 3; j++)
                    {
                        if (i != j)
                        {
                            Player *q = &players[j];
                            if (q->isStarted == 'y' && p->floor == q->floor &&
                                p->PosX == q->PosX && p->PosY == q->PosY)
                                resetPlayerToStart(q);
                        }
                    }
                }
            }
            printf("-----------------------------------------------\n");
        }
    }
}

void startGame()
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



#endif