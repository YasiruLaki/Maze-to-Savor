#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "headers.h"

// global variables
int globalRoundCounter = 0;
unsigned int globalSeed = 0;

// Configuring Bawana Cells
BawanaCell bawanaCells[12] = {
    {6, 20, 0},
    {7, 20, 0},
    {8, 20, 1},
    {9, 20, 1},
    {6, 21, 2},
    {7, 21, 2},
    {8, 21, 3},
    {9, 21, 3},
    {6, 22, 4},
    {7, 22, 4},
    {8, 22, 4},
    {9, 22, 4}};

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
int countLines(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    int count = 0, ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') count++;
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
        int index = poles[count].startFloor * (floorWidth * floorLength) +
                    poles[count].y * floorLength + poles[count].x;

        // Mark pole block in the blocks array
        blocks[index].blockType = 'p';
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
                blocks[index].consumable = 0;
                blocks[index].bonus = 0;

                // Randomly assign consumables and bonuses, avoiding the flag position
                int roll = rand() % 100;
                if (roll < 10)
                    blocks[index].consumable = (rand() % 2 == 0) ? 5 : -3;
                else if (roll < 15)
                    blocks[index].bonus = (rand() % 2 == 0) ? 3 : 1;
            }
        }
    }

    // Deactivate blocks as per deactivation file
    handleDeactivation("deactiveBlocks.txt", blocks, width, length, flagIndex);
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
void rollMoveDice(int *moveDice)
{

    *moveDice = (rand() % 6) + 1;
}

// Dice rolling functions
void rollDirectionDice(int *directionDice)
{

    *directionDice = (rand() % 6) + 1;
}

// Send player to Bawana and apply effects
void sendToBawana(Player *p)
{
    p->floor = 0;
    p->PosX = 19;
    p->PosY = 9;
    p->direction = 0;

    printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->playerName);

    // Randomly select a Bawana cell
    int cellIndex = rand() % 12;

    BawanaCell cell = bawanaCells[cellIndex];

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
        printf("Player %c eats from Bawana and has food poisoning. Will miss 3 turns.\n", p->playerName);
        break;

    case 1: // Disoriented
        p->movPoints = 50;
        p->disorientedTurns = 4;
        printf("Player %c eats from Bawana and is disoriented. Gets 50 MP.\n", p->playerName);
        break;

    case 2: // Triggered
        p->movPoints = 50;
        p->triggeredTurns = 4;
        printf("Player %c eats from Bawana and is triggered. Gets 50 MP.\n", p->playerName);
        break;

    case 3: // Happy
        p->movPoints = 200;
        printf("Player %c eats from Bawana and is happy. Gets 200 MP.\n", p->playerName);
        break;

    case 4: // Random
    {
        int points = 10 + rand() % 91;
        p->movPoints = points;
        printf("Player %c eats from Bawana and earns %d MP.\n", p->playerName, points);
        break;
    }
    }
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

int findBestStair(int x, int y, int floor, Stairs stairs[], int stairsCount, int flagIndex, int floorWidth, int floorLength, int isAtStart)
{
    int bestStair = -1;
    double minDistance = 1e9;
    int validStairs[16];
    int validCount = 0;

    for (int s = 0; s < stairsCount; s++)
    {
        // Check if the player is at the start or end of the stair at this cell
        if (floor == stairs[s].startFloor && x == stairs[s].startX && y == stairs[s].startY)
        {
            // Can go up if stair is bidirectional or up only
            if (stairs[s].direction == 0 || stairs[s].direction == 1)
            {
                double dist = calculateDistanceToFlag(stairs[s].endX, stairs[s].endY, stairs[s].endFloor, flagIndex, floorWidth, floorLength);
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
            // Can go down if stair is bidirectional or down only
            if (stairs[s].direction == 0 || stairs[s].direction == 2)
            {
                double dist = calculateDistanceToFlag(stairs[s].startX, stairs[s].startY, stairs[s].startFloor, flagIndex, floorWidth, floorLength);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    bestStair = s;
                }
                validStairs[validCount++] = s;
            }
        }
    }

    // If multiple stairs have the same minimal distance, pick randomly among them
    if (validCount > 1)
    {
        int tieStairs[16];
        int tieCount = 0;

        for (int i = 0; i < validCount; i++)
        {
            int s = validStairs[i];
            double dist;

            if (floor == stairs[s].startFloor && x == stairs[s].startX && y == stairs[s].startY)
                dist = calculateDistanceToFlag(stairs[s].endX, stairs[s].endY, stairs[s].endFloor, flagIndex, floorWidth, floorLength);
            else
                dist = calculateDistanceToFlag(stairs[s].startX, stairs[s].startY, stairs[s].startFloor, flagIndex, floorWidth, floorLength);

            if (fabs(dist - minDistance) < 0.01)
                tieStairs[tieCount++] = s;
        }

        // If there's a tie, pick randomly among the best
        if (tieCount > 1)
            bestStair = tieStairs[rand() % tieCount];
    }

    return bestStair;
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
                rollMoveDice(&d);
                if (d == 6)
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

            // Check for missed turns
            if (p->missTurns > 0)
            {
                p->missTurns--;
                printf("Player %c is missing this turn. Remaining missed turns: %d\n", p->playerName, p->missTurns);
                continue;
            }

            // Check for disoriented state
            if (p->disorientedTurns > 0)
            {
                p->disorientedTurns--;
                p->direction = rand() % 4;
                printf("Player %c is disoriented and changes direction to %d. Remaining disoriented turns: %d\n", p->playerName, p->direction, p->disorientedTurns);
            }

            // triggered logic

            // Roll movement dice
            int moveDice;
            rollMoveDice(&moveDice);

            if (turnCounter[i] % 4 == 0)
            {
                int dirRoll;
                rollDirectionDice(&dirRoll);
                wasDirectionChanged = 1;

                if (dirRoll >= 2 && dirRoll <= 5)
                {
                    p->direction = dirRoll - 2;
                }
            }

            int success = canMove(p, moveDice, blocks, floorWidth, floorLength, stairs, stairsCount, poles, polesCount, flagIndex);
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

            if (wasDirectionChanged)
            {
                printf("Player %c rolls %d on the movement dice and moves to (%d,%d,%d) in direction %d after changing direction.\n", p->playerName, moveDice, p->floor, p->PosY, p->PosX, p->direction);
                wasDirectionChanged = 0;
            }
            else
            {
                printf("Player %c rolls %d on the movement dice and moves to (%d,%d,%d) in direction %d.\n", p->playerName, moveDice, p->floor, p->PosY, p->PosX, p->direction);
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
                printf("Player %c has reached the flag at (%d,%d,%d) and wins the game!\n", p->playerName, p->floor, p->PosY, p->PosX);
                gameOver = 1;
            }

            if (p->movPoints <= 0)
            {
                sendToBawana(p);
            }

            printf("-----------------------------------------------\n");
        }
    }
}