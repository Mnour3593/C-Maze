#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

// Maze Display Characters (ASCII-friendly)
#define WALL_CHAR  '#'
#define PATH_CHAR  ' '
#define PLAYER_CHAR 'P'
#define EXIT_CHAR  'E'

// Default Settings
#define DEFAULT_SIZE 21  // Default maze size
#define MIN_SIZE 5
#define MAX_SIZE 51

// Function Prototypes
void welcomeScreen();
void getMazeSize();
unsigned int getSeed();
void generateMaze();
void digPath(int x, int y);
void printMaze();
int isValidMove(int x, int y);
void playerMovement();
void gameLoop();
void endGame(int offerRestart);
void restartGame(int skipWelcome);
void freeMaze();
void displayTime();

// Global Variables
char **maze;
int playerX, playerY;
unsigned int seed;
int mazeSize = DEFAULT_SIZE;
time_t startTime;
int moveCounter = 0;  // Initialize move counter

int main() {
    restartGame(0);
    return 0;
}

// Welcome screen with ASCII art
void welcomeScreen() {
    system("cls");
    printf("=====================================\n");
    printf(" C-MAZE - A terminal based maze game \n");
    printf("=====================================\n");
    printf("Use WASD to move. Reach 'E' to win!\n");
    printf("Press any key to continue...\n");
    _getch();
}

// Function to get user-defined maze size
void getMazeSize() {
    char input[20];
    int tempSize = 0;

    while (1) {
        printf("Enter maze size (%d-%d, odd numbers only, default is %d): ", MIN_SIZE, MAX_SIZE, DEFAULT_SIZE);
        fgets(input, sizeof(input), stdin);

        if (input[0] == '\n') {  // If user just presses Enter, use default
            tempSize = DEFAULT_SIZE;
            break;
        }

        tempSize = atoi(input);

        if (tempSize >= MIN_SIZE && tempSize <= MAX_SIZE) {
            if (tempSize % 2 == 0) {
                tempSize++;  // Convert even numbers to the next odd number
                printf("Adjusted to next odd number: %d\n", tempSize);
            }
            break;  // Valid input, exit loop
        }

        printf("Invalid size! Please enter an odd number between %d and %d.\n", MIN_SIZE, MAX_SIZE);
        Sleep(1000);
    }

    mazeSize = tempSize;
}


// Function to get user-defined seed
unsigned int getSeed() {
    char input[20];

    printf("\nEnter a seed (or press Enter for random): ");
    fgets(input, sizeof(input), stdin);

    if (input[0] == '\n') {
        return time(NULL);
    } else {
        return (unsigned int) atoi(input);
    }
}

// Function to generate the maze
void generateMaze() {
    // Allocate memory for the maze
    maze = (char **)malloc(mazeSize * sizeof(char *));
    for (int i = 0; i < mazeSize; i++) {
        maze[i] = (char *)malloc(mazeSize * sizeof(char));
    }

    // Fill maze with walls
    for (int i = 0; i < mazeSize; i++) {
        for (int j = 0; j < mazeSize; j++) {
            maze[i][j] = WALL_CHAR;
        }
    }

    // Start carving paths from (1,1)
    digPath(1, 1);

    // Set player start and exit point
    maze[1][1] = PATH_CHAR;
    maze[mazeSize - 2][mazeSize - 3] = EXIT_CHAR;

    // Set player position
    playerX = 1;
    playerY = 1;

    // Start timer
    startTime = time(NULL);
}

// Recursive function to dig a path
void digPath(int x, int y) {
    maze[x][y] = PATH_CHAR;  // Create path

    int dx[] = {0, 0, -2, 2};
    int dy[] = {-2, 2, 0, 0};
    int order[] = {0, 1, 2, 3};

    // Fisher-Yates shuffle for random order
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

    // Create random branches
    int branchCount = rand() % 3 + 2;
    for (int i = 0; i < branchCount; i++) {
        int nx = x + dx[order[i]];
        int ny = y + dy[order[i]];

        if (nx >= 0 && nx < mazeSize && ny >= 0 && ny < mazeSize && maze[nx][ny] == WALL_CHAR) {
            maze[(x + nx) / 2][(y + ny) / 2] = PATH_CHAR;
            digPath(nx, ny);
        }
    }
}

// Function to print the maze
void printMaze() {
    system("cls");  // Clear screen

    for (int i = 0; i < mazeSize; i++) {
        for (int j = 0; j < mazeSize; j++) {
            if (i == playerX && j == playerY)
                printf("%c ", PLAYER_CHAR);
            else
                printf("%c ", maze[i][j]);
        }
        printf("\n");
    }
    
    displayTime();
    printf("\nMoves: %d\n", moveCounter);  // Display move count
    printf("\nControls: W A S D = Move | Q = Quit | R = Restart\n");
}

// Function to check valid movement
int isValidMove(int x, int y) {
    return (x >= 0 && x < mazeSize && y >= 0 && y < mazeSize && maze[x][y] != WALL_CHAR);
}

// Function to handle player movement
void playerMovement() {
    char input = _getch();
    int newX = playerX, newY = playerY;

    if (input == 'w') newX--;
    else if (input == 'a') newY--;
    else if (input == 's') newX++;
    else if (input == 'd') newY++;
    else if (input == 'q') endGame(0);
    else if (input == 'r') restartGame(1);
    else return;

    if (isValidMove(newX, newY)) {
        playerX = newX;
        playerY = newY;
        moveCounter++;  // Increment move counter

        if (maze[playerX][playerY] == EXIT_CHAR) {
            time_t endTime = time(NULL);
            int elapsedTime = (int)difftime(endTime, startTime);
            printf("\nCongratulations! You won in %d seconds and %d moves!\n", elapsedTime, moveCounter);
            endGame(1);
        }
    }
}

// Main game loop
void gameLoop() {
    while (1) {
        printMaze();
        playerMovement();
    }
}

// Function to end the game
void endGame(int offerRestart) {
    printf("\nGame Over! Seed: %u\n", seed);
    Sleep(1000);
    if (offerRestart) {
        printf("Press R to restart or any other key to exit...\n");
        char choice = _getch();
        if (choice == 'r') restartGame(1);
    }
    freeMaze();
    exit(0);
}

// Function to restart the game
void restartGame(int skipWelcome) {
    system("cls");

    if (!skipWelcome) {
        welcomeScreen();
    }

    getMazeSize();
    seed = getSeed();
    srand(seed);

    generateMaze();
    moveCounter = 0;  // Reset move counter when restarting
    gameLoop();
}

// Function to free memory
void freeMaze() {
    for (int i = 0; i < mazeSize; i++) {
        free(maze[i]);
    }
    free(maze);
}

// Function to display elapsed time
void displayTime() {
    time_t currentTime = time(NULL);
    int elapsedTime = (int)difftime(currentTime, startTime);
    printf("\nTime elapsed: %d seconds\n", elapsedTime);
}
