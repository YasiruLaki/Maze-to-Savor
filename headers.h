#ifndef HEADERS_H
#define HEADERS_H

typedef struct
{
    int startFloor;
    int startX;
    int startY;
    int endFloor;
    int endX;
    int endY;
    int direction;
} Stairs;

typedef struct
{
    int startFloor;
    int endFloor;
    int x;
    int y;
} Poles;

typedef struct
{
    int floor;
    int startX;
    int startY;
    int endX;
    int endY;
} Walls;

typedef struct
{
    int floor;
    int x;
    int y;
    int isActive;
    int movPoints;  
    char blockType; 
    int consumable; 
    int bonus;      
} Block;

typedef struct
{
    int floor;
    int width;
    int length;
} Floor;

typedef struct
{
    int floor;
    int PosX;
    int PosY;
    int direction; 
    int movPoints;
    int speed;
    char playerName;
    char isStarted;
    int missTurns;
    int disorientedTurns;
    int triggeredTurns;
    int speedMultiplier;
} Player;

typedef struct
{
    int y;
    int x;
    int type; 
} BawanaCell;

void loadStairs(const char *filename, Stairs *stairs, Block blocks[], int stairsCount, int floorWidth, int floorLength);
void loadPoles(const char *filename, Poles *poles, Block blocks[], int polesCount, int floorWidth, int floorLength);
void loadWalls(const char *filename, Walls *walls, Block blocks[], int wallsCount, int floorWidth, int floorLength);
void initializeFloors(Floor floors[], Block blocks[], int width, int length, int flagIndex);
void handleDeactivation(const char *filename, Block blocks[], int width, int length, int flagIndex);
int readFlagPosition(int floorWidth, int floorLength);

void rollMoveDice(int *moveDice);
void rollDirectionDice(int *directionDice);

void randomizeStairDirections(Stairs *stairs, int stairsCount);
int findBestStair(int x, int y, int floor, Stairs stairs[], int stairsCount, int flagIndex, int floorWidth, int floorLength, int canGoUp);
double calculateDistanceToFlag(int x, int y, int floor, int flagIndex, int floorWidth, int floorLength);



unsigned int loadSeed(const char *path);

int canMove(Player *p, int moveDice, Block blocks[], int floorWidth, int floorLength,
            Stairs stairs[], int stairsCount, Poles poles[], int polesCount, int flagIndex);
int movePlayerStep(Player *p, Block blocks[], int floorWidth, int floorLength,
                   Stairs stairs[], int stairsCount, Poles poles[], int polesCount, int flagIndex);

void resetPlayerToStart(Player *p);

void gamePlay(Block blocks[], Player players[], int floorWidth, int floorLength,
              Stairs stairs[], int stairsCount, Poles poles[], int polesCount);

void deactivateBlock(Block blocks[], int index);
void handleStairMidpoint(Stairs *stairs, Block blocks[], int stairIndex, int floorWidth, int floorLength);
int checkObstructingWalls(Walls *walls, int wallIndex, int floorWidth, int floorLength);

#endif