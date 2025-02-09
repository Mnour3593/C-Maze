#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

// Define maze display characters
#define WALL_CHAR  '#'
#define PATH_CHAR  ' '
#define PLAYER_CHAR 'P'
#define EXIT_CHAR  'E'

// Default maze size settings
#define DEFAULT_SIZE 21  
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

// Global variables
char **maze;
int playerX, playerY;
unsigned int seed;
int mazeSize = DEFAULT_SIZE; 
time_t startTime;
int moveCounter = 0;  // Tracks player moves

int main() {
    restartGame(0);  // Start the game
    return 0;
}

// Display the welcome screen with game instructions
void welcomeScreen() {
    system("cls");
    printf("=====================================\n");
    printf(" C-MAZE - A terminal based maze game \n");
    printf("=====================================\n");
    printf("Use WASD to move. Reach 'E' to win!\n");
    printf("Press any key to continue...\n");
    _getch();  // Wait for user to press a key
}

// Prompt user to choose maze size
void getMazeSize() {
    char input[20];
    int tempSize = 0;

    while (1) {
        printf("Enter maze size (%d-%d, odd numbers only, default is %d): ", MIN_SIZE, MAX_SIZE, DEFAULT_SIZE);
        fgets(input, sizeof(input), stdin);  // Get input from user

        if (input[0] == '\n') {  // Default size if Enter is pressed
            tempSize = DEFAULT_SIZE;
            break;
        }

        tempSize = atoi(input);  // Convert input to integer

        if (tempSize >= MIN_SIZE && tempSize <= MAX_SIZE) {
            if (tempSize % 2 == 0) {
                tempSize++;  // Convert even number to the next odd number
                printf("Adjusted to next odd number: %d\n", tempSize);
            }
            break;  // Exit loop if valid size entered
        }

        printf("Invalid size! Please enter an odd number between %d and %d.\n", MIN_SIZE, MAX_SIZE);
        Sleep(1000);  // Wait before retrying
    }

    mazeSize = tempSize;  // Set the maze size
}

// Get the seed from the user or use random
unsigned int getSeed() {
    char input[20];

    printf("\nEnter a seed (or press Enter for random): ");
    fgets(input, sizeof(input), stdin);  // Get seed input

    if (input[0] == '\n') {
        return time(NULL);  // Use current time as seed if Enter is pressed
    } else {
        return (unsigned int) atoi(input);  // Use provided seed
    }
}

// Generate the maze and set start and exit
void generateMaze() {
    maze = (char **)malloc(mazeSize * sizeof(char *));  // Allocate memory for maze
    for (int i = 0; i < mazeSize; i++) {
        maze[i] = (char *)malloc(mazeSize * sizeof(char));  // Allocate each row
    }

    // Fill the maze with walls
    for (int i = 0; i < mazeSize; i++) {
        for (int j = 0; j < mazeSize; j++) {
            maze[i][j] = WALL_CHAR;  // Walls everywhere
        }
    }

    digPath(1, 1);  // Start digging paths from position (1, 1)

    // Set the start and exit points
    maze[1][1] = PATH_CHAR;
    maze[mazeSize - 2][mazeSize - 3] = EXIT_CHAR;

    playerX = 1;  // Set player initial position
    playerY = 1;

    startTime = time(NULL);  // Start timer for game duration
}

// Recursive function to dig paths for the maze
void digPath(int x, int y) {
    maze[x][y] = PATH_CHAR;  // Mark current position as path

    int dx[] = {0, 0, -2, 2};
    int dy[] = {-2, 2, 0, 0};
    int order[] = {0, 1, 2, 3};

    // Shuffle the directions to randomize path digging
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

    // Create random branches of paths
    int branchCount = rand() % 3 + 2;
    for (int i = 0; i < branchCount; i++) {
        int nx = x + dx[order[i]];
        int ny = y + dy[order[i]];

        if (nx >= 0 && nx < mazeSize && ny >= 0 && ny < mazeSize && maze[nx][ny] == WALL_CHAR) {
            maze[(x + nx) / 2][(y + ny) / 2] = PATH_CHAR;  // Carve path
            digPath(nx, ny);  // Recurse to dig next path
        }
    }
}

// Print the maze to the terminal
void printMaze() {
    system("cls");  // Clear the screen

    // Display the maze grid
    for (int i = 0; i < mazeSize; i++) {
        for (int j = 0; j < mazeSize; j++) {
            if (i == playerX && j == playerY) 
                printf("%c ", PLAYER_CHAR);  // Display player position
            else
                printf("%c ", maze[i][j]);  // Display walls and paths
        }
        printf("\n");
    }

    displayTime();  // Show elapsed time
    printf("\nMoves: %d\n", moveCounter);  // Show move counter
    printf("\nControls: W A S D = Move | Q = Quit | R = Restart\n");
}

// Check if move is valid (i.e. not out of bounds or hitting walls)
int isValidMove(int x, int y) {
    return (x >= 0 && x < mazeSize && y >= 0 && y < mazeSize && maze[x][y] != WALL_CHAR);
}

// Handle player input for movement and actions
void playerMovement() {
    char input = _getch();  // Get user input
    int newX = playerX, newY = playerY;

    // Process movement input
    if (input == 'w') newX--;
    else if (input == 'a') newY--;
    else if (input == 's') newX++;
    else if (input == 'd') newY++;
    else if (input == 'q') endGame(0);  // Quit the game
    else if (input == 'r') restartGame(1);  // Restart the game
    else return;  // Invalid input, do nothing

    if (isValidMove(newX, newY)) {
        playerX = newX;  // Update player position
        playerY = newY;
        moveCounter++;  // Increment move counter

        if (maze[playerX][playerY] == EXIT_CHAR) {
            time_t endTime = time(NULL);
            int elapsedTime = (int)difftime(endTime, startTime);  // Calculate time taken
            printf("\nCongratulations! You won in %d seconds and %d moves!\n", elapsedTime, moveCounter);
            endGame(1);  // End game upon reaching exit
        }
    }
}

// Main game loop that continues until the game ends
void gameLoop() {
    while (1) {
        printMaze();  // Display current maze
        playerMovement();  // Handle player movement
    }
}

// End the game and offer restart option
void endGame(int offerRestart) {
    printf("\nGame Over! Seed: %u\n", seed);
    Sleep(1000);
    if (offerRestart) {
        printf("Press R to restart or any other key to exit...\n");
        char choice = _getch();
        if (choice == 'r') restartGame(1);  // Restart the game
    }
    freeMaze();  // Free allocated memory
    exit(0);  // Exit the game
}

// Restart the game, optionally skipping the welcome screen
void restartGame(int skipWelcome) {
    system("cls");

    if (!skipWelcome) {
        welcomeScreen();  // Show welcome screen
    }

    getMazeSize();  // Get maze size from user
    seed = getSeed();  // Get seed for randomness
    srand(seed);  // Set random seed

    generateMaze();  // Generate new maze
    moveCounter = 0;  // Reset move counter
    gameLoop();  // Start the main game loop
}

// Free the dynamically allocated maze memory
void freeMaze() {
    for (int i = 0; i < mazeSize; i++) {
        free(maze[i]);  // Free each row
    }
    free(maze);  // Free the main maze structure
}

// Display the time elapsed since game start
void displayTime() {
    time_t currentTime = time(NULL);
    int elapsedTime = (int)difftime(currentTime, startTime);  // Calculate elapsed time
    printf("\nTime elapsed: %d seconds\n", elapsedTime);  // Display elapsed time
}
