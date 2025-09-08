# Maze to Savor

## Overview

**Maze to Savor!** is a complex maze runner game developed in C where three players compete to capture a flag placed randomly in a multi-floor maze. The game features intricate movement mechanics, environmental interactions, and special gameplay elements that create an engaging competitive experience.

## Project Structure

```
maze-runner/
├── main.c              # Main program entry point
├── gameLogic.c         # Core game logic implementation
├── headers.h           # Header file with type definitions and function declarations
├── inputs/             # Directory containing game configuration files
│   ├── stairs.txt      # Stair definitions
│   ├── poles.txt       # Pole definitions
│   ├── walls.txt       # Wall definitions
│   ├── flag.txt        # Flag position
│   └── seed.txt        # Random number generator seed
└── README.md           # This file
```

## Compilation and Execution

### Prerequisites
- GCC compiler
- Standard C library
- Math library support

### Compilation
```bash
gcc -o maze-runner main.c gameLogic.c -lm
```

### Execution
```bash
./maze-runner
```

## Game Rules

### Maze Structure
The game takes place in a three-floor maze with unique layouts:

- **Floor 1**: 856 sq ft with a starting area
- **Floor 2**: 784 sq ft with two rectangular areas connected by a bridge  
- **Floor 3**: 360 sq ft rectangular area
- Each floor is **10 blocks wide** and **25 blocks long**

### Player Movement
Players navigate the maze using a dice-based movement system:

- **Movement Dice (1-6)**: Determines how many cells to move
- **Direction Dice**: Determines movement direction (North, East, South, West) or maintains current direction
- Players start in designated positions with initial directions

### Special Elements

#### Stairs
- Connect different floors
- Can be bidirectional or unidirectional
- Allow vertical movement between levels

#### Poles  
- Provide downward movement between floors
- One-way transportation method

#### Walls
- Block player movement
- Define maze boundaries and obstacles

#### Bawana (Special Area)
A special area with various effects when players land on it:

- **Food Poisoning**: Player misses next three turns
- **Disoriented**: Player moves randomly for four turns  
- **Triggered**: Player moves twice as fast
- **Happy**: Player receives 200 movement points

## Game Mechanics

### Movement Points System
- Players consume movement points when moving through cells
- Each cell has consumable values that deduct from movement points
- Some cells provide bonuses to movement points
- When movement points reach zero, player is transported to Bawana

### Player Interactions
- Players can capture each other by landing on the same cell
- Captured players face consequences based on game rules

### Victory Condition
- First player to capture the randomly placed flag wins the game

## Input Files

### stairs.txt
**Format**: `[startFloor,startX,startY,endFloor,endX,endY]`
**Example**: `[0,4,5,2,0,10]`

### poles.txt  
**Format**: `[startFloor,endFloor,x,y]`
**Example**: `[0,2,5,24]`

### walls.txt
**Format**: `[floor,startX,startY,endX,endY]`
**Example**: `[1,0,2,8,2]`

### flag.txt
**Format**: `[floor,x,y]`
**Example**: `[2,5,12]`

### seed.txt
Contains a single number used to seed the random number generator for consistent gameplay testing.

## Output

The game produces detailed output messages including:

- Player movements and direction changes
- Interactions with special elements (stairs, poles, Bawana)
- Movement point updates and transactions
- Game state changes and player status
- Victory announcements

## Implementation Details

### Key Data Structures

- **`Block`**: Represents individual cells in the maze with properties like consumable values
- **`Player`**: Tracks player state including position, direction, and movement points
- **`Stairs`**: Defines stair connections between floors
- **`Poles`**: Defines pole positions for downward movement
- **`Walls`**: Defines wall positions that block movement

### Core Functions

- **`loadSeed()`**: Loads the random seed from seed.txt file
- **`initializeFloors()`**: Sets up the maze structure and floor layouts
- **`loadStairs()`**, **`loadPoles()`**, **`loadWalls()`**: Parse respective input files
- **`gamePlay()`**: Main game loop handling player turns and game logic
- **`sendToBawana()`**: Handles Bawana special effects and consequences

## Development Notes

This implementation follows the specified game rules with special attention to:

- **Movement Mechanics**: Dice-based movement with direction handling
- **Vertical Navigation**: Proper stair and pole interaction systems
- **Special Effects**: Complete Bawana effects implementation
- **Resource Management**: Movement point tracking and management
- **Player Interactions**: Capture mechanics and collision detection
- **Modular Design**: Clear separation between game logic, data structures, and I/O operations

## Future Enhancements

Potential improvements for future versions:

- **Graphical User Interface**: Visual representation of the maze and players
- **Additional Special Areas**: More interactive elements and effects
- **Custom Maze Configurations**: User-defined maze layouts and rules
- **Multi-threaded Player Movement**: Simultaneous player actions
- **Network Multiplayer Support**: Online competitive gameplay
- **Save/Load Game States**: Ability to pause and resume games
- **Statistics Tracking**: Player performance analytics
- **Sound Effects**: Audio feedback for game events

## Installation and Setup

1. Clone or download the project files
2. Ensure all input files are properly configured in the `inputs/` directory
3. Compile using the provided compilation command
4. Run the executable to start the game

## Troubleshooting

### Common Issues

- **Compilation Errors**: Ensure GCC is properly installed and math library is linked
- **File Not Found**: Verify all input files exist in the `inputs/` directory
- **Segmentation Faults**: Check input file formats match expected structure
- **Unexpected Behavior**: Verify seed.txt contains a valid integer

## Contributing

This project is part of the UCSC curriculum. Contributions should follow academic integrity guidelines and course requirements.

## License

This project is created for educational purposes as part of the UCSC curriculum. All rights reserved for educational use.

---

**Developed for UCSC Computer Science Program**  
*A challenging maze navigation game featuring strategic movement and environmental interactions*