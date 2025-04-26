// C-MAZE - A terminal based maze game
// Note: This code uses Windows-specific functions and headers (windows.h, conio.h)
// and will not compile/run on other operating systems without modification.
#define _CRT_SECURE_NO_WARNINGS // Optional: Suppress warnings for standard fopen if not using fopen_s on MSVC

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>    // For _getch(), _kbhit() (Windows specific console I/O)
#include <windows.h>  // For Sleep(), system(), console handles, gotoxy, colors (Windows specific)
#include <time.h>     // For time(), srand(), difftime(), strftime(), localtime()
#include <string.h>   // For strcspn(), strcpy(), strcmp(), strlen()
#include <errno.h>    // For errno

// --- Game Modes ---
#define SINGLE_PLAYER 1
#define DUAL_PLAYER   2

// --- Maze Display Characters ---
#define WALL_CHAR    '`' // Using backtick for wall (can be invisible/customizable)
#define PATH_CHAR    ' '
#define EXIT_CHAR    'E'
#define BONUS_CHAR   '.'
#define OVERLAP_CHAR 'X' // Character displayed when players overlap

// --- Default Player Icons ---
#define DEFAULT_PLAYER1_CHAR 'M'
#define DEFAULT_PLAYER2_CHAR 'N'

// --- Points ---
#define BONUS_POINTS 10

// --- Movement Input Keys ---
// WASD for Player 1
// Arrow Keys for Player 2 (requires checking for prefix -32)
#define ARROW_KEY_PREFIX -32 // Special value indicating an arrow key sequence
#define UP_INPUT    72
#define DOWN_INPUT  80
#define LEFT_INPUT  75
#define RIGHT_INPUT 77

// --- Maze Size Settings ---
#define DEFAULT_SIZE 21  // Default dimension (must be odd)
#define MIN_SIZE 5
#define MAX_SIZE 51
#define DEFAULT_ALGORITHM 1 // Default algorithm for maze generation (Prim's)

// --- Console Colors (Windows specific FOREGROUND/BACKGROUND constants) ---
#define COLOR_WHITE         (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE    (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)

#define COLOR_DEFAULT       (COLOR_WHITE) 
#define COLOR_PATH_BLACK_BG (COLOR_DEFAULT)
#define COLOR_PATH_WHITE_BG (BACKGROUND_WHITE) 

// Player & Mode Colors
// _ALT is the same color but on a white background, used when the path is white
#define COLOR_SINGLE    (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_DUAL      (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_PLAYER1   (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COLOR_PLAYER2   (FOREGROUND_BLUE | FOREGROUND_INTENSITY) 
#define COLOR_PLAYER1_ALT (BACKGROUND_WHITE | FOREGROUND_RED) 
#define COLOR_PLAYER2_ALT (BACKGROUND_WHITE | FOREGROUND_BLUE)

// Maze Element Colors
#define COLOR_EXIT      (BACKGROUND_GREEN | BACKGROUND_INTENSITY) 
#define COLOR_BONUS     (FOREGROUND_RED | FOREGROUND_GREEN ) 
#define COLOR_BONUS_ALT (FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_WHITE) 
#define COLOR_OVERLAP   (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY) 
#define COLOR_OVERLAP_ALT (COLOR_OVERLAP | BACKGROUND_WHITE) 

// UI Element Colors
#define COLOR_INFO      (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_ERROR     (FOREGROUND_RED | FOREGROUND_INTENSITY) 
#define COLOR_SUCCESS   (FOREGROUND_GREEN | FOREGROUND_INTENSITY)

// Full Block Colors (for random wall colors)
#define COLOR_FULLRED     (FOREGROUND_RED | BACKGROUND_RED)
#define COLOR_FULLBLUE    (FOREGROUND_BLUE | BACKGROUND_BLUE)
#define COLOR_FULLWHITE   (COLOR_WHITE | BACKGROUND_WHITE)
#define COLOR_FULLGREEN   (FOREGROUND_GREEN | BACKGROUND_GREEN)
#define COLOR_FULLMAGENTA (FOREGROUND_RED | FOREGROUND_BLUE | BACKGROUND_RED | BACKGROUND_BLUE)
#define COLOR_FULLCYAN    (FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define COLOR_FULLYELLOW  (FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_GREEN)

// --- Constants ---
#define MAX_BONUS_PLACEMENT_ATTEMPTS_MULTIPLIER 2

// --- Structs ---
// Represents a potential wall segment between two cells in Kruskal's
typedef struct {
    int x1, y1; // Coords of the first cell
    int x2, y2; // Coords of the second cell
} WallSegment;

// Represents a cell coordinate (used in Prim's)
typedef struct {
    int x, y;
} Cell;

// --- Global Variables ---

// Maze state
char **maze = NULL;         // 2D array for the maze structure
int mazeSize = DEFAULT_SIZE; // Current maze dimensions (odd number)
unsigned int seed;           // Seed used for randomization
int exitX, exitY;            // Coordinates of the exit
int totalDots = 0;           // Count of bonus dots placed

// Player state
int playerX, playerY;        // Player 1 coordinates
int player1Score = 0;
int player1Moves = 0;
char player1Char = DEFAULT_PLAYER1_CHAR;

int player2X, player2Y;      // Player 2 coordinates (used only in DUAL_PLAYER mode)
int player2Score = 0;
int player2Moves = 0;
char player2Char = DEFAULT_PLAYER2_CHAR;

// Game flow state
int gameMode = SINGLE_PLAYER;  // Current game mode
int winner = 0;              // 0 = game running, 1 = P1 wins, 2 = P2 wins, -1 = quit
time_t startTime;            // Time when the current game started
int mazeRegenCount = 0;      // Counter for maze regeneration attempts
int generationAlgorithmChoice = DEFAULT_ALGORITHM; // '1' Prim's Algorithm by default

// Console handle (Windows specific)
HANDLE hConsole;

// Dynamic Color Scheme Variables (set by setMazeColorScheme)
WORD g_currentWallColor;
WORD g_currentPathColor;
WORD g_currentPlayer1Color;
WORD g_currentPlayer2Color;
WORD g_currentBonusColor;
WORD g_currentExitColor;
WORD g_currentOverlapColor;
WORD g_currentDefaultColor;

// --- Function Prototypes ---

// Console Utilities (Windows Specific)
void gotoxy(int x, int y);
void setConsoleColor(WORD color);
void showConsoleCursor(int show);
void clearConsoleLine(int row);
void clearInputBuffer();

// Game Setup & Initialization
void displayWelcomeScreen();    // Displays the title screen and instructions
void promptForGameMode();   // Prompts for game mode (Single/Dual Player)
void promptForPlayerIcons();    // Prompts for player icons (characters)
void getMazeSize(); // Prompts for maze size (odd number within limits)
unsigned int getSeed(); // Prompts for maze seed (positive number or random)
void promptForAlgorithm();  // Prompts for maze generation algorithm choice
void initializeMazeState(); // Initializes the maze state and player positions
void placeBonusDots();  // Places bonus dots randomly on the maze
int isExitReachable(); // BFS check used after generation
void freeMaze();    // Frees the allocated maze memory
void setMazeColorScheme();  // Sets the color scheme for maze elements based on the current mode

// Maze Generation Algorithms
void carveMazePathRecursive(int startX, int startY);    // Recursive backtracking algorithm
void carveMazePathRecursiveAlternative(int startX, int startY); // Iterative backtracking algorithm
void generatePrimMaze(int startX, int startY);  // Prim's algorithm for maze generation
void generateKruskalMaze(int startX, int startY);   // Kruskal's algorithm for maze generation (uses DSU helper functions internally)
void generateWilsonMaze(int startX, int startY);    // Wilson's algorithm for maze generation

// Game Loop & Logic
void gameLoop();    // Main game loop for handling player input and game state
void handlePlayerInput();   // Handles player input for movement and actions
int isValidMove(int x, int y);  // Checks if a given coordinate (x, y) is a valid move target

// Rendering
void renderMazeAndStatus(); // Renders the entire maze grid and status information
void displayTime();   // Displays the elapsed time since the game started

// Game End & Scoring
void saveScore(int winningPlayer, int score, int moves, int timeTaken); // Prompts to save score to file
void endGame(int offerRestart); // Ends the game, offering to restart or quit

// Main Game Controller
void initializeAndStartGame(int skipWelcome);   // Initializes a new game or restarts, handling setup prompts and maze generation


// === MAIN FUNCTION ===
int main() {
    // Get the handle to the standard output device (console)
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        perror("FATAL ERROR: Failed to get console handle");
        return 1; // Exit if console handle cannot be obtained
    }

    // Start the game initialization process
    initializeAndStartGame(0); // 0 means show the welcome screen and prompts

    // Ensure cursor is visible and color is default before exiting
    showConsoleCursor(1);
    setConsoleColor(COLOR_DEFAULT);
    gotoxy(0, mazeSize + 10); // Move cursor below game area

    return 0; // Indicate successful execution
}

// === CONSOLE UTILITY FUNCTIONS (Windows Specific) ===

// Moves the console cursor to the specified coordinates (X, Y).
void gotoxy(int x, int y) {
    COORD coord = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, coord); // Set the cursor position in the console
}

/*
 * Sets the text attributes (color) for subsequent console output.
 */
void setConsoleColor(WORD color) {
    SetConsoleTextAttribute(hConsole, color);   // Set the console text color
}

/*
 * Shows or hides the console cursor.
 */
void showConsoleCursor(int show) {
    CONSOLE_CURSOR_INFO cursorInfo; // Structure to hold cursor information
    GetConsoleCursorInfo(hConsole, &cursorInfo);    // Get current cursor info
    cursorInfo.bVisible = show;    // Set to TRUE (1) or FALSE (0)
    SetConsoleCursorInfo(hConsole, &cursorInfo);    // Update the console with the new cursor info
}

// Clears a specific line on the console by overwriting with spaces.
void clearConsoleLine(int row) {
    gotoxy(0, row);
    // Print spaces across the typical console width (e.g., 80)
    // Adjust width if your console is significantly different
    printf("                                                                                "); // 80 spaces
    gotoxy(0, row); // Move cursor back to the start of the cleared line
}

/*
 * Clears the keyboard input buffer by consuming any pending characters.
 * Useful before reading new input to avoid processing old key presses.
 */
void clearInputBuffer() {
    while (_kbhit()) {  // Check if a key has been pressed
        _getch();     // Consume the key press
    }
}

// === GAME SETUP & INITIALIZATION FUNCTIONS ===

/*
 * Displays the ASCII art title screen and basic instructions.
 */
void displayWelcomeScreen() {
    system("cls"); // Clear the console screen
    setConsoleColor(COLOR_BONUS); // Use a bright color for the title
    // ASCII Art for "C-MAZE"
    printf("   ______       .___  ___.      ___      ________   _______  \n");
    printf("  /      |      |   \\/   |     /   \\    |       /  |   ____| \n");
    printf(" |  ,----'      |  \\  /  |    /  ^  \\    ---/  /   |  |__    \n");
    printf(" |  |           |  |\\/|  |   /  /_\\  \\     /  /    |   __|   \n");
    printf(" |  ----.       |  |  |  |  /  _____  \\   /  /----.|  |____  \n");
    printf("  \\______|      |__|  |__| /__/     \\__\\ /________||_______| \n\n");

    setConsoleColor(COLOR_DEFAULT);
    printf(" C MAZE - A terminal based maze game \n");
    printf("=====================================\n");

    printf("Reach '");
    setConsoleColor(COLOR_EXIT); printf("%c", EXIT_CHAR); setConsoleColor(COLOR_INFO); 
    printf("' first to win! Collect '");
    setConsoleColor(COLOR_BONUS); printf("%c", BONUS_CHAR); setConsoleColor(COLOR_INFO);
    printf("' for points.\n");
    setConsoleColor(COLOR_ERROR);   // Error color for warnings
    printf("Please note: Due to cursor shifting, maximizing the console window is recommended. Press F11 for Fullscreen.\n");
    setConsoleColor(COLOR_INFO);    // Reset to info color for further prompts
}

/*
 * Prompts the user to select Single Player or Dual Player mode.
 * Uses _getch() for immediate input.
 */
void promptForGameMode() {
    char inputKey;
    setConsoleColor(COLOR_INFO);
    int promptRow = 13; // Row position for the prompt

    clearConsoleLine(promptRow);
    clearConsoleLine(promptRow + 1);

    gotoxy(0, promptRow);   // Position cursor at the prompt row
    showConsoleCursor(1); // Show cursor for input
    printf("Choose game mode; Press 2 for Dual Player, any other key for Single Player: ");
    clearInputBuffer(); // Ensure no prior key presses interfere
    inputKey = _getch(); // Read a single character directly

    if (inputKey == '2') {
        gameMode = DUAL_PLAYER;
        gotoxy(0, promptRow -1); // Position message slightly above prompt line
        setConsoleColor(COLOR_DUAL); printf("Head to Head!"); setConsoleColor(COLOR_INFO); printf(" Dual Player Selected. Reach the exit first!\n");
    } else {
        gameMode = SINGLE_PLAYER;
         gotoxy(0, promptRow -1);
        printf("Going "); setConsoleColor(COLOR_SINGLE); printf("Solo"); setConsoleColor(COLOR_INFO); printf(". Single Player Selected. Conquer the maze!\n");
    }
    showConsoleCursor(0);

    // Clear the prompt area after selection
    clearConsoleLine(promptRow);
    clearConsoleLine(promptRow + 1);
    clearConsoleLine(promptRow + 2);
}

/*
 * Prompts the user(s) to enter their desired player icons.
 * Uses fgets for input, allowing multi-character input but only using the first non-space character.
 */
void promptForPlayerIcons() {
    char inputBuffer[16]; // Small buffer for input
    setConsoleColor(COLOR_INFO);
    int promptRow = 14; // Start row for prompts

    // --- Player 1 Icon ---
    clearConsoleLine(promptRow);
    clearConsoleLine(promptRow + 1); // Clear area
    gotoxy(0, promptRow);
    printf("Enter "); setConsoleColor(COLOR_PLAYER1); printf("Player 1 "); setConsoleColor(COLOR_INFO);
    printf("character [Default = "); setConsoleColor(COLOR_PLAYER1); printf("%c", DEFAULT_PLAYER1_CHAR);
    setConsoleColor(COLOR_INFO); printf("]: ");

    clearInputBuffer();
    player1Char = DEFAULT_PLAYER1_CHAR; // Set default initially
    showConsoleCursor(1); // Show cursor for input
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        inputBuffer[strcspn(inputBuffer, "\n")] = 0; // Remove trailing newline
        if (strlen(inputBuffer) > 0 && inputBuffer[0] != ' ') { // Use first non-space character if provided
            player1Char = inputBuffer[0];
        }
    }
    showConsoleCursor(0); // Hide cursor after input
    clearConsoleLine(promptRow); // Clear the prompt line

// --- Player 2 Icon (only if Dual Player mode) ---
if (gameMode == DUAL_PLAYER) {
    while (1) {
        clearConsoleLine(promptRow);
        gotoxy(0, promptRow);

        printf("Enter "); setConsoleColor(COLOR_PLAYER2); printf("Player 2 "); setConsoleColor(COLOR_INFO);
        printf("character [Default = "); setConsoleColor(COLOR_PLAYER2); printf("%c", DEFAULT_PLAYER2_CHAR);
        setConsoleColor(COLOR_INFO); printf("], cannot be '%c': ", player1Char);

        clearInputBuffer();
        showConsoleCursor(1);
        player2Char = DEFAULT_PLAYER2_CHAR; // Set default initially

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {   
            inputBuffer[strcspn(inputBuffer, "\n")] = 0; // Remove newline

            if (strlen(inputBuffer) == 0) {
                // User pressed Enter → use default, but check for conflict
                if (player2Char == player1Char) {
                    gotoxy(0, promptRow + 1);
                    setConsoleColor(COLOR_ERROR);
                    printf("Default '%c' conflicts with Player 1. Choose another.", player2Char);
                    setConsoleColor(COLOR_INFO);
                    continue; // Reprompt
                }
                break; // Accept default
            }
            else if (inputBuffer[0] != ' ' && inputBuffer[0] != player1Char) {  // Check for space and conflict with P1 char
                // Valid input
                player2Char = inputBuffer[0];
                break;
            }
            else {
                gotoxy(0, promptRow + 1);
                setConsoleColor(COLOR_ERROR);
                if (inputBuffer[0] == player1Char)
                    printf("Character cannot match Player 1 ('%c'). Choose another.", player1Char);
                else
                    printf("Please enter a non-space character.");
                setConsoleColor(COLOR_INFO);
            }
        }
    }

    showConsoleCursor(0);
    clearConsoleLine(promptRow);
    clearConsoleLine(promptRow + 1);
}
 else {
        player2Char = DEFAULT_PLAYER2_CHAR; // Ensure P2 char is default even if not used
    }
    setConsoleColor(COLOR_DEFAULT); // Reset color
}

/*
 * Prompts the user to enter the desired maze size (odd number within limits).
 * Uses fgets for input.
 */
void getMazeSize() {
    char inputBuffer[20];
    int tempSize = 0;
    setConsoleColor(COLOR_INFO);
    int promptRow = 16; // Position below Icons prompt area

    while (1) { // Loop until valid input is received
        clearConsoleLine(promptRow);
        clearConsoleLine(promptRow + 1); // Clear prompt area
        gotoxy(0, promptRow);
        printf("Enter maze size (%d-%d, odd numbers only) [Default = %d]: ", MIN_SIZE, MAX_SIZE, DEFAULT_SIZE);

        clearInputBuffer();
        showConsoleCursor(1);
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
            inputBuffer[strcspn(inputBuffer, "\n")] = 0; // Remove newline

            if (inputBuffer[0] == '\0') { // User pressed Enter for default
                tempSize = DEFAULT_SIZE;
                break; // Exit loop with default size
            }

            // Attempt to convert input to integer
            char* endPtr;
            tempSize = strtol(inputBuffer, &endPtr, 10);

            // Check if conversion was successful and input is within range
            if (endPtr != inputBuffer && *endPtr == '\0' && tempSize >= MIN_SIZE && tempSize <= MAX_SIZE) {
                if (tempSize % 2 == 0) { // If even, adjust to the next odd number
                    tempSize++;
                    if (tempSize > MAX_SIZE) tempSize -= 2; // Adjust down if incrementing went out of bounds
                    gotoxy(0, promptRow + 1);
                    printf("Input was even, adjusted to odd number: %d", tempSize);
                    Sleep(1500);
                }
                 // Check again after potential adjustment
                if (tempSize >= MIN_SIZE && tempSize <= MAX_SIZE) {
                    break; // Valid odd number within range, exit loop
                }
            }
        }
        // If input was invalid
        gotoxy(0, promptRow + 1);
        setConsoleColor(COLOR_ERROR);
        printf("Invalid size! Please enter an odd number between %d and %d.", MIN_SIZE, MAX_SIZE);
        setConsoleColor(COLOR_INFO);
        Sleep(2000); // Pause to show error
        clearerr(stdin); // Clear potential error flags on stdin
    }

    // Warn if size is large
    if (tempSize > 32) {
        gotoxy(0, promptRow + 1);
        setConsoleColor(COLOR_ERROR);
        printf("Warning: Large maze size may affect performance/visibility. Use CTRL '-' to zoom out. Press any key to continue.");
        setConsoleColor(COLOR_INFO);
        showConsoleCursor(1);   // Show cursor for user input
        _getch(); // Wait for user to acknowledge
        clearConsoleLine(promptRow + 1); // Clear warning message
    }

    showConsoleCursor(0);
    mazeSize = tempSize; // Store the validated size globally
    // Clear prompt area
    clearConsoleLine(promptRow);
    clearConsoleLine(promptRow + 1);
    setConsoleColor(COLOR_DEFAULT);
}

// Prompts the user to enter a seed for maze generation or use a random one.
unsigned int getSeed() {
    char inputBuffer[20];
    unsigned int chosen_seed = 0; 
    setConsoleColor(COLOR_INFO);
    int promptRow = 17; // Position below maze size prompt area

    while (1) {
        clearConsoleLine(promptRow);
        clearConsoleLine(promptRow + 1);
        gotoxy(0, promptRow);
        printf("Enter maze seed (number > 0) [Press Enter for random]: ");

        clearInputBuffer();
        showConsoleCursor(1);
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
            inputBuffer[strcspn(inputBuffer, "\n")] = 0; // Remove newline

            if (inputBuffer[0] == '\0') { // User pressed Enter for random
                chosen_seed = (unsigned int)time(NULL); // Use current time as seed
                if (chosen_seed == 0) chosen_seed = 1; // Ensure seed is not 0
                break;
            } else {
                // Validate if input is a positive number
                int valid = 1;
                if (inputBuffer[0] == '0' && inputBuffer[1] == '\0') valid = 0; // Seed must be > 0
                for(int i = 0; inputBuffer[i] != '\0'; i++) {
                    if(inputBuffer[i] < '0' || inputBuffer[i] > '9') {
                        valid = 0;
                        break;
                    }
                }

                if (valid) {
                    chosen_seed = (unsigned int)atoi(inputBuffer);
                    if (chosen_seed > 0) { // Check again just to be sure atoi worked
                         break; // Valid seed entered
                    }
                }
            }
        }
        // If input was invalid
        gotoxy(0, promptRow + 1);
        setConsoleColor(COLOR_ERROR);
        printf("Invalid seed! Please enter a positive number or press Enter.");
        setConsoleColor(COLOR_INFO);
        Sleep(2000);
        clearerr(stdin);    // Clear potential error flags on stdin
    }

    showConsoleCursor(0);
    // Clear prompt area
    clearConsoleLine(promptRow);
    clearConsoleLine(promptRow + 1);
    setConsoleColor(COLOR_DEFAULT);
    return chosen_seed;
}

//Prompts the user to select the maze generation algorithm.
void promptForAlgorithm() {
    char inputBuffer[10];
    int choice = 0;
    int promptRow = 18; // Position overwriting maze size prompt area
    setConsoleColor(COLOR_INFO);

    const char* algoNames[] = {
        "Prim's Algorithm : Cellular Expansion",
        "Kruskal's Algorithm : Wall Bridging",
        "Wilson's Algorithm : Wandering Paths",
        "Recursive Backtracker : Deep First Search",
        "Recursive Backtracker (Alternative) : Iterative Loops"
    };
    int numAlgorithms = sizeof(algoNames) / sizeof(algoNames[0]);
    int defaultChoiceIndex = 0; // Index of the default algorithm (Prim's)

    while (1) { // Loop until valid input
        // Clear potential prompt area
        for (int i = 0; i < numAlgorithms + 4; ++i) clearConsoleLine(promptRow + i);
        gotoxy(0, promptRow);

        printf("Current maze settings: %d x %d, Seed: %u\n\n", mazeSize, mazeSize, seed);
        printf("Select Maze Generation Algorithm:\n");
        for (int i = 0; i < numAlgorithms; ++i) {
             printf("  %d. %s\n", i + 1, algoNames[i]);
        }

        printf("Enter choice (1-%d) [Default = %d ]: ", numAlgorithms, defaultChoiceIndex + 1); // Default is Prim's

        clearInputBuffer();
        showConsoleCursor(1);
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
            inputBuffer[strcspn(inputBuffer, "\n")] = 0; // Remove newline

            if (inputBuffer[0] == '\0') { // Default choice
                choice = defaultChoiceIndex + 1; // Use default algorithm number
                break;
            }

            char *endPtr;
            choice = strtol(inputBuffer, &endPtr, 10);

            // Check if conversion was successful and choice is within valid range
            if (endPtr != inputBuffer && *endPtr == '\0' && choice >= 1 && choice <= numAlgorithms) {
                break; // Valid choice entered
            }
        }
        // Invalid input
        gotoxy(0, promptRow + numAlgorithms + 2); // Position error below options
        setConsoleColor(COLOR_ERROR);
        printf("Invalid choice. Please enter a number between 1 and %d.", numAlgorithms);
        setConsoleColor(COLOR_INFO);
        Sleep(2000);
        clearerr(stdin);
    }

    showConsoleCursor(0);
    generationAlgorithmChoice = choice; // Store the chosen algorithm globally

     // Clear prompt area after selection
    for (int i = 0; i < numAlgorithms + 4; ++i) clearConsoleLine(promptRow + i);
    setConsoleColor(COLOR_DEFAULT);
}


/*
 * Allocates memory for the maze, initializes it with walls,
 * calls the selected generation algorithm, places the exit and bonus dots,
 * and sets initial player positions.
 */
void initializeMazeState() {
    // --- Allocate Memory for Maze Array ---
    maze = (char **)malloc(mazeSize * sizeof(char *));
    if (!maze) {
        perror("FATAL ERROR: Failed to allocate memory for maze rows");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < mazeSize; i++) {
        maze[i] = (char *)malloc(mazeSize * sizeof(char));
        if (!maze[i]) {
            perror("FATAL ERROR: Failed to allocate memory for maze columns");
            // Cleanup already allocated rows before exiting
            for(int j = 0; j < i; j++) {
                free(maze[j]);
            }
            free(maze);
            maze = NULL;
            exit(EXIT_FAILURE);
        }
        // Initialize all cells as walls initially
        for (int j = 0; j < mazeSize; j++) {
            maze[i][j] = WALL_CHAR;
        }
    }

    // --- Call Selected Maze Generation Algorithm ---
    int startGenX = 1;
    int startGenY = 1; // Standard starting cell for generation
    switch (generationAlgorithmChoice) {
        case 1: generatePrimMaze(startGenX, startGenY); break;
        case 2: generateKruskalMaze(startGenX, startGenY); break;
        case 3: generateWilsonMaze(startGenX, startGenY); break;
        case 4: carveMazePathRecursive(startGenX, startGenY); break;
        case 5: carveMazePathRecursiveAlternative(startGenX, startGenY); break;
        default: // Fallback
            fprintf(stderr, "Warning: Invalid algorithm choice %d, defaulting to Prim's.\n", generationAlgorithmChoice);
            generatePrimMaze(startGenX, startGenY);
            break;
    }
    // Ensure the starting cell itself is a path, regardless of algorithm
    maze[startGenX][startGenY] = PATH_CHAR;

    // --- Set Exit Point ---
    // Try placing the exit near the bottom-right corner path cell
    exitX = mazeSize - 2;
    exitY = mazeSize - 2;
    if (maze[exitX][exitY] == WALL_CHAR) { // If corner is wall, search nearby
        int foundExit = 0;
        // Check adjacent path cells preferred over searching far away
        if (exitX > 1 && maze[exitX - 1][exitY] == PATH_CHAR) { exitX--; foundExit = 1; } // Up
        else if (exitY > 1 && maze[exitX][exitY - 1] == PATH_CHAR) { exitY--; foundExit = 1; } // Left
        else if (exitX < mazeSize - 2 && maze[exitX + 1][exitY] == PATH_CHAR) { exitX++; foundExit = 1; } // Down (less common start)
        else if (exitY < mazeSize - 2 && maze[exitX][exitY + 1] == PATH_CHAR) { exitY++; foundExit = 1; } // Right (less common start)

        // If no adjacent path, search backwards from bottom right systematically
        if (!foundExit) {
            for (int r = mazeSize - 2; r > 0 && !foundExit; r--) {
                for (int c = mazeSize - 2; c > 0; c--) {
                    if (maze[r][c] == PATH_CHAR) {
                        exitX = r;
                        exitY = c;
                        foundExit = 1;
                        break;
                    }
                }
            }
        }
        // Absolute fallback if no path found (shouldn't happen with valid generation)
        if (!foundExit) {
            fprintf(stderr, "Warning: Could not find a valid exit position! Placing near start.\n");
            exitX = 1; exitY = 3; // Try (1,3)
            if(maze[exitX][exitY] == WALL_CHAR) { exitX = 3; exitY = 1; } // Try (3,1)
            if(maze[exitX][exitY] == WALL_CHAR) { exitX = 1; exitY = 1; } // Use start itself
        }
    }
    maze[exitX][exitY] = EXIT_CHAR; // Mark the chosen exit cell


    // --- Place Bonus Dots ---
    placeBonusDots();

    // --- Set Initial Player Positions ---
    playerX = 1;
    playerY = 1; // Player 1 always starts at (1, 1)
    player1Moves = 0; player1Score = 0;

    if (gameMode == DUAL_PLAYER) {
        player2Moves = 0;
        player2Score = 0;
        int p2placed = 0;
        // Try placing P2 adjacent to P1 on a path cell
        int offsets[][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}; // Check adjacent cells
        for (int i = 0; i < 8 && !p2placed; ++i) {
            int checkX = playerX + offsets[i][0];
            int checkY = playerY + offsets[i][1];
            // Check bounds and if the cell is a path
            if (checkX > 0 && checkX < mazeSize - 1 && checkY > 0 && checkY < mazeSize - 1 && maze[checkX][checkY] == PATH_CHAR) {
                player2X = checkX;
                player2Y = checkY;
                p2placed = 1;
            }
        }
        // Fallback if no adjacent path cell found (e.g., P1 starts in a 1x1 pocket)
        if (!p2placed) {
            player2X = 3; player2Y = 1; // Try (3,1)
            if (player2X >= mazeSize-1 || maze[player2X][player2Y] == WALL_CHAR || (player2X == playerX && player2Y == playerY)) {
                player2X = 1; player2Y = 3; // Try (1,3)
                if (player2Y >= mazeSize-1 || maze[player2X][player2Y] == WALL_CHAR || (player2X == playerX && player2Y == playerY)) {
                     player2X = 3; player2Y = 3; // Try (3,3)
                }
            }
             // Ensure fallback didn't land on P1 again or wall
             if ((player2X == playerX && player2Y == playerY) || maze[player2X][player2Y] == WALL_CHAR) {
                 player2X = -1; player2Y = -1; // Mark as invalid if truly stuck (should be rare)
                 fprintf(stderr, "Warning: Could not place Player 2 validly near start!\n");
             }
        }
    } else {
        // In single player mode, set Player 2 coords to invalid values
        player2X = -1;
        player2Y = -1;
    }

    // --- Reset Game State Variables ---
    winner = 0;
    mazeRegenCount = 0;
    // startTime is set later in gameLoop
}

/*
 * Places bonus dots randomly on PATH_CHAR cells within the maze.
 * Avoids placing dots on the start or exit cells.
 */
void placeBonusDots() {
    // Determine number of dots based on maze size
    int dotsToPlace = mazeSize / 2;
    if (dotsToPlace < 1) dotsToPlace = 1; // Ensure at least one dot
    totalDots = 0;

    int attempts = 0;
    // Limit attempts to prevent infinite loops on very sparse/small mazes
    const int maxAttempts = mazeSize * mazeSize * MAX_BONUS_PLACEMENT_ATTEMPTS_MULTIPLIER;

    while (totalDots < dotsToPlace && attempts < maxAttempts) {
        // Generate random coordinates within maze boundaries (excluding border)
        int randX = 1 + (rand() % (mazeSize - 2));
        int randY = 1 + (rand() % (mazeSize - 2));

        // Check if the random cell is a path and not the start, exit, or P2 start
        if (maze[randX][randY] == PATH_CHAR &&
            !(randX == exitX && randY == exitY) &&        // Not the exit
            !(randX == 1 && randY == 1) &&                // Not the P1 start
            !(gameMode == DUAL_PLAYER && randX == player2X && randY == player2Y) // Not P2 start
           )
        {
            maze[randX][randY] = BONUS_CHAR;
            totalDots++;
        }
        attempts++;
    }

    if (totalDots < dotsToPlace) {
        fprintf(stderr, "Warning: Could only place %d of %d bonus dots after %d attempts.\n", totalDots, dotsToPlace, maxAttempts);
    }
}

/*
 * Checks if the exit cell is reachable from the player's starting position (1, 1)
 * using Breadth-First Search (BFS). Used to validate maze generation.
 */
int isExitReachable() {
    if (playerX == exitX && playerY == exitY) return 1; // Edge case: start is exit

    // --- Allocate 'visited' array ---
    int **visited = (int **)malloc(mazeSize * sizeof(int *));
    if (!visited) {
        perror("Error allocating visited array rows in isExitReachable");
        return 0; // Cannot perform check if allocation fails
    }
    for (int i = 0; i < mazeSize; i++) {
        // Use calloc to initialize all cells to 0 (not visited)
        visited[i] = (int *)calloc(mazeSize, sizeof(int));
        if (!visited[i]) {
            perror("Error allocating visited array columns in isExitReachable");
            for(int j = 0; j < i; j++) free(visited[j]);
            free(visited);
            return 0; // Cannot perform check
        }
    }

    // --- Initialize BFS Queue ---
    // Simple array-based queue; size mazeSize*mazeSize is guaranteed large enough
    int queue[mazeSize * mazeSize][2]; // Stores {x, y} coordinates
    int front = 0, rear = 0;

    // Add the starting position (Player 1's start) to the queue
    int startX = 1, startY = 1;
    if (startX >= 0 && startX < mazeSize && startY >= 0 && startY < mazeSize && maze[startX][startY] != WALL_CHAR) {
        queue[rear][0] = startX;
        queue[rear][1] = startY;
        rear++;
        visited[startX][startY] = 1; // Mark start as visited
    } else {
        // Start position is invalid (a wall?), exit unreachable by definition
         fprintf(stderr,"Error: Start position (1,1) is invalid for BFS reachability check.\n");
         for (int i = 0; i < mazeSize; i++) free(visited[i]);
         free(visited);
         return 0;
    }

    int reachable = 0; // Flag

    // --- Perform BFS ---
    while (front < rear) { // While queue is not empty
        int x = queue[front][0];
        int y = queue[front][1];
        front++;

        if (x == exitX && y == exitY) {
            reachable = 1; // Exit found!
            break; // Stop search
        }

        // Explore neighbors (Up, Down, Left, Right)
        int dx[] = {-1, 1, 0, 0}; // Row offsets
        int dy[] = {0, 0, -1, 1}; // Column offsets
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            // Check if neighbor is within bounds, is not a wall, and hasn't been visited
            if (nx >= 0 && nx < mazeSize && ny >= 0 && ny < mazeSize &&
                maze[nx][ny] != WALL_CHAR && !visited[nx][ny])
            {
                visited[nx][ny] = 1; // Mark neighbor as visited
                // Enqueue the neighbor
                queue[rear][0] = nx;
                queue[rear][1] = ny;
                rear++;
            }
        }
    }

    // --- Cleanup ---
    for (int i = 0; i < mazeSize; i++) {
        free(visited[i]);
    }
    free(visited);

    return reachable;
}

/*
 * Frees the dynamically allocated memory used by the maze array.
 * Sets the global maze pointer to NULL afterwards.
 */
void freeMaze() {
    if (maze != NULL) {
        for (int i = 0; i < mazeSize; i++) {
            if (maze[i] != NULL) {
                free(maze[i]);
                maze[i] = NULL; // Good practice
            }
        }
        free(maze);
        maze = NULL;
    }
}

/*
 * Selects a randomized color scheme for walls, paths, and dependent elements.
 * Stores the chosen colors in global variables (g_current...).
 */
void setMazeColorScheme() {
    // --- Choose Wall Color ---
    static const WORD wallColors[] = {
        COLOR_FULLRED, COLOR_FULLBLUE, COLOR_FULLGREEN,
        COLOR_FULLMAGENTA, COLOR_FULLCYAN, COLOR_FULLYELLOW
    };
    static const int numWallColors = sizeof(wallColors) / sizeof(wallColors[0]);
    g_currentWallColor = wallColors[rand() % numWallColors];

    // --- Choose Path Background Color ---
    static const WORD pathOptions[] = {
        COLOR_PATH_BLACK_BG, // Standard white text on black background
        COLOR_PATH_WHITE_BG  // Black text on white background
    };
    g_currentPathColor = pathOptions[rand() % 2];

    // --- Determine Dependent Colors based on Path Choice ---
    // Use alternate (_ALT) colors if path background is white for visibility.
    if (g_currentPathColor == COLOR_PATH_WHITE_BG) {
        g_currentPlayer1Color = COLOR_PLAYER1_ALT; // Red on White BG
        g_currentPlayer2Color = COLOR_PLAYER2_ALT; // Blue on White BG
        g_currentBonusColor   = COLOR_BONUS_ALT;   // Yellow on White BG
        g_currentExitColor    = COLOR_EXIT;        // Exit color unchanged (Green BG)
        g_currentOverlapColor = COLOR_OVERLAP_ALT; // Magenta on White BG
        g_currentDefaultColor = g_currentPathColor;// Default is path color itself
    }
    else { // Path background is black, use standard colors.
        g_currentPlayer1Color = COLOR_PLAYER1;     // Bright Red on Black BG
        g_currentPlayer2Color = COLOR_PLAYER2;     // Bright Blue on Black BG
        g_currentBonusColor   = COLOR_BONUS;       // Bright Yellow on Black BG
        g_currentExitColor    = COLOR_EXIT;        // Exit color unchanged
        g_currentOverlapColor = COLOR_OVERLAP;     // Bright Magenta on Black BG
        g_currentDefaultColor = COLOR_DEFAULT;     // Standard default (White on Black BG)
    }
}

// === MAZE GENERATION ALGORITHMS ===

/*
 * Generates a maze using the Recursive Backtracking algorithm.
 * Creates a  maze with no loops but one path from start to exit.
 */
void carveMazePathRecursive(int x, int y) {
    maze[x][y] = PATH_CHAR; // Mark current cell as path

    // Define directions (move 2 cells at a time) this is by design to ensure walls are always 1 cell away from the path
    // dx and dy are defined such that they represent the direction of movement
    int dx[] = {0, 0, 2, -2}; // N, S, E, W (row changes)
    int dy[] = {2, -2, 0, 0}; // E, W, N, S (col changes) - indexes are shifted vs dx

    // Shuffle order to randomize direction choice
    int order[] = {0, 1, 2, 3};
    for (int i = 0; i < 4; ++i) {
        int j = rand() % 4;
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

    // Iterate through shuffled directions
    for (int i = 0; i < 4; ++i) {
        int dir_idx = order[i];
        // Calculate coordinates of neighbor cell (2 steps away) and wall cell (1 step away)
        int nx = x + dx[dir_idx];
        int ny = y + dy[dir_idx];
        int wallX = x + dx[dir_idx] / 2;
        int wallY = y + dy[dir_idx] / 2;

        // Check if neighbor is within bounds and is currently an unvisited wall
        if (nx > 0 && nx < mazeSize - 1 && ny > 0 && ny < mazeSize - 1 && maze[nx][ny] == WALL_CHAR) {
            maze[wallX][wallY] = PATH_CHAR; // Carve the wall
            carveMazePathRecursive(nx, ny); // Recurse into neighbor
        }
    }
}

/*
 * Generates a maze using a modified Recursive Backtracking algorithm
 * that introduces a chance of creating loops to add more pathways.
 */
void carveMazePathRecursiveAlternative(int x, int y) {
    maze[x][y] = PATH_CHAR; // Mark current cell as path

    int directions[4] = {0, 1, 2, 3}; // 0:Up, 1:Down, 2:Left, 3:Right
    // Shuffle directions
    for (int i = 0; i < 4; ++i) {
        int j = rand() % 4;
        int temp = directions[i]; directions[i] = directions[j]; directions[j] = temp;
    }

    for (int i = 0; i < 4; ++i) {
        int dx = 0, dy = 0;
        switch (directions[i]) {
            case 0: dx = -2; break; // Up
            case 1: dx =  2; break; // Down
            case 2: dy = -2; break; // Left
            case 3: dy =  2; break; // Right
        }

        int nx = x + dx;    // Neighbor coords
        int ny = y + dy;    // Neighbor coords
        int wx = x + dx / 2;    // Wall coords
        int wy = y + dy / 2;    // Wall coords

        // Check if neighbor is within maze bounds
        if (nx > 0 && nx < mazeSize - 1 && ny > 0 && ny < mazeSize - 1) {
            // If neighbor is an unvisited wall cell
            if (maze[nx][ny] == WALL_CHAR) {
                maze[wx][wy] = PATH_CHAR; // Carve the wall
                carveMazePathRecursiveAlternative(nx, ny); // Recurse into neighbor
            }
            // Loop Creation Logic: If the wall between current and neighbor still exists,
            // but the neighbor is already a path (visited via another route),
            // there's a chance (15%) to carve this wall anyway, creating a loop.
            else if (maze[wx][wy] == WALL_CHAR && (rand() % 100) < 15) { // 15% chance
                maze[wx][wy] = PATH_CHAR; // Carve wall, creating loop
            }
        }
    }
}

/*
 * Generates a maze using Prim's algorithm (randomized version).
 * Starts from a cell, adds its walls to a list, and iteratively carves
 * walls connecting to unvisited cells.
 */
void generatePrimMaze(int startX, int startY) {
    Cell *frontier = NULL; // Dynamic array of frontier cells
    int frontierSize = 0;
    int frontierCapacity = 0;

    int dirs[4][2] = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}}; // N, S, W, E

    maze[startX][startY] = PATH_CHAR; // Mark start as path

    // --- Add initial frontier cells (neighbors of the start cell) ---
    for (int i = 0; i < 4; i++) {
        int nx = startX + dirs[i][0];
        int ny = startY + dirs[i][1];
        if (nx > 0 && nx < mazeSize - 1 && ny > 0 && ny < mazeSize - 1) {
            // Expand frontier array if needed
            if (frontierSize >= frontierCapacity) {
                frontierCapacity = (frontierCapacity == 0) ? 4 : frontierCapacity * 2;
                Cell* temp = realloc(frontier, frontierCapacity * sizeof(Cell));
                if (!temp) { free(frontier); perror("Error reallocating frontier in Prim's (initial)"); return; }
                frontier = temp;
            }
            frontier[frontierSize++] = (Cell){nx, ny};
        }
    }

    // --- Main Prim's Loop ---
    while (frontierSize > 0) {
        // Randomly select a cell from the frontier
        int idx = rand() % frontierSize;
        Cell current = frontier[idx];

        // Remove selected cell from frontier (swap with last element)
        frontier[idx] = frontier[--frontierSize];

        // Find maze neighbors (already path) of the 'current' frontier cell
        Cell mazeNeighbors[4];
        int mazeNeighborCount = 0;
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dirs[i][0];
            int ny = current.y + dirs[i][1];
            if (nx > 0 && nx < mazeSize - 1 && ny > 0 && ny < mazeSize - 1 && maze[nx][ny] == PATH_CHAR) {
                mazeNeighbors[mazeNeighborCount++] = (Cell){nx, ny};
            }
        }

        // If the current frontier cell has neighbors already in the maze
        if (mazeNeighborCount > 0) {
            // Randomly choose one maze neighbor to connect to
            Cell connectTo = mazeNeighbors[rand() % mazeNeighborCount];
            // Carve wall between current and chosen neighbor
            int wallX = (current.x + connectTo.x) / 2;
            int wallY = (current.y + connectTo.y) / 2;
            maze[wallX][wallY] = PATH_CHAR;
            maze[current.x][current.y] = PATH_CHAR; // Add current cell to maze path

            // Add *new* frontier neighbors of the 'current' cell
            for (int i = 0; i < 4; i++) {
                int nx = current.x + dirs[i][0];
                int ny = current.y + dirs[i][1];
                // Check if neighbor is valid and still a wall
                if (nx > 0 && nx < mazeSize - 1 && ny > 0 && ny < mazeSize - 1 && maze[nx][ny] == WALL_CHAR) {
                    // Check if this neighbor is *already* in the frontier list
                    int alreadyInFrontier = 0;
                    for (int j = 0; j < frontierSize; j++) {
                        if (frontier[j].x == nx && frontier[j].y == ny) {
                            alreadyInFrontier = 1; break;
                        }
                    }
                    // If not already in frontier, add it
                    if (!alreadyInFrontier) {
                         if (frontierSize >= frontierCapacity) { // Expand if needed
                            frontierCapacity = (frontierCapacity == 0) ? 4 : frontierCapacity * 2; // Avoid initial *2 if capacity is 0
                            Cell* temp = realloc(frontier, frontierCapacity * sizeof(Cell));
                            if (!temp) { free(frontier); perror("Error reallocating frontier in Prim's (loop)"); return; }
                            frontier = temp;
                         }
                        frontier[frontierSize++] = (Cell){nx, ny};
                    }
                }
            }
        }
    }

    free(frontier); // Free the frontier array memory
}


// --- Kruskal's Algorithm Helper Functions (DSU) ---
// These need to be defined *before* generateKruskalMaze or declared via prototypes if placed after.
// For simplicity here, they are nested or require careful ordering.

// DSU: Find the root/representative of the set containing element i (with path compression)
// Assumes 'parent' array is accessible (passed as arg or global - here uses the one in generateKruskalMaze scope)
int find_set_kruskal(int i, int* parent) {
    if (parent[i] == i) return i;
    return parent[i] = find_set_kruskal(parent[i], parent); // Path compression
}

// DSU: Unite the sets containing elements i and j
void unite_sets_kruskal(int i, int j, int* parent) {
    int root_i = find_set_kruskal(i, parent);
    int root_j = find_set_kruskal(j, parent);
    if (root_i != root_j) {
        parent[root_j] = root_i; // Make root_i the parent of root_j
    }
}
// --- End Kruskal Helpers ---


/*
 * Generates a maze using Kruskal's algorithm (randomized version).
 * Treats potential walls as edges, shuffles them, and adds edges (removes walls)
 * if they connect two previously disconnected components (using DSU).
 */
void generateKruskalMaze(int startX, int startY) {
    // Dimensions of the cell grid (half the maze size)
    int numCellsX = (mazeSize - 1) / 2;
    int numCellsY = (mazeSize - 1) / 2;
    int totalCells = numCellsX * numCellsY;

    // --- Disjoint Set Union (DSU) Setup ---
    int *parent = (int*)malloc(totalCells * sizeof(int));
    if (!parent) { perror("Error allocating parent array in Kruskal's"); return; }
    for (int i = 0; i < totalCells; i++) { parent[i] = i; } // Each cell is its own set initially

    // --- Wall List Generation ---
    WallSegment *walls = NULL; // Dynamic array for potential walls
    int wallCount = 0;
    int wallCapacity = 0;

    // Iterate through cell locations (odd coordinates) to find potential walls
    for (int x = 1; x < mazeSize; x += 2) {
        for (int y = 1; y < mazeSize; y += 2) {
            // Check/add wall to the right (Horizontal)
            if (y + 2 < mazeSize) {
                 if (wallCount >= wallCapacity) { // Expand wall list
                    wallCapacity = (wallCapacity == 0) ? (numCellsX * numCellsY) : wallCapacity * 2; // Estimate initial
                    WallSegment* temp = realloc(walls, wallCapacity * sizeof(WallSegment));
                    if (!temp) { free(walls); free(parent); perror("Error reallocating walls in Kruskal's (H)"); return; }
                    walls = temp;
                 }
                walls[wallCount++] = (WallSegment){x, y, x, y + 2};
            }
            // Check/add wall below (Vertical)
            if (x + 2 < mazeSize) {
                 if (wallCount >= wallCapacity) { // Expand wall list
                    wallCapacity = (wallCapacity == 0) ? (numCellsX * numCellsY) : wallCapacity * 2;
                    WallSegment* temp = realloc(walls, wallCapacity * sizeof(WallSegment));
                    if (!temp) { free(walls); free(parent); perror("Error reallocating walls in Kruskal's (V)"); return; }
                    walls = temp;
                 }
                walls[wallCount++] = (WallSegment){x, y, x + 2, y};
            }
        }
    }

    // --- Shuffle the Wall List (Fisher-Yates) ---
    for (int i = wallCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        WallSegment temp = walls[i]; walls[i] = walls[j]; walls[j] = temp;
    }

    // --- Process Walls ---
    // Kruskal's starts with all walls and removes them. Ensure start is path.
    maze[startX][startY] = PATH_CHAR; // Mark start explicitly

    int edgesAdded = 0;
    for (int i = 0; i < wallCount && edgesAdded < totalCells - 1; i++) { // Stop when MST is formed
        int x1 = walls[i].x1, y1 = walls[i].y1;
        int x2 = walls[i].x2, y2 = walls[i].y2;

        // Convert cell coords (1,1), (1,3)... to DSU indices (0, 1...)
        int cell1_idx = (x1 / 2) * numCellsY + (y1 / 2);
        int cell2_idx = (x2 / 2) * numCellsY + (y2 / 2);

        // Check if the cells are already connected using DSU
        if (find_set_kruskal(cell1_idx, parent) != find_set_kruskal(cell2_idx, parent)) {
            // If not connected, unite them
            unite_sets_kruskal(cell1_idx, cell2_idx, parent);

            // Carve the wall between them
            int wallX = (x1 + x2) / 2;
            int wallY = (y1 + y2) / 2;
            maze[wallX][wallY] = PATH_CHAR;

            // Also ensure the cells themselves are paths
            maze[x1][y1] = PATH_CHAR;
            maze[x2][y2] = PATH_CHAR;
            edgesAdded++;
        }
    }

    // --- Cleanup ---
    free(parent);
    free(walls);
}


/*
 * Generates a maze using Wilson's algorithm.
 * Starts with one cell, then performs random walks from unvisited cells
 * until they hit the existing maze. The walk path is then added.
 * Handles loop erasure during walks.
 */
void generateWilsonMaze(int startX, int startY) {
    // Wilson's adds paths to an initially all-wall grid.
    // initializeMazeState already sets walls, but this ensures it.
    for (int i = 0; i < mazeSize; i++)
        for (int j = 0; j < mazeSize; j++)
            maze[i][j] = WALL_CHAR;

    maze[startX][startY] = PATH_CHAR; // Mark the first cell as part of the maze

    int numCellsX = (mazeSize - 1) / 2;
    int numCellsY = (mazeSize - 1) / 2;
    int totalCellsToVisit = numCellsX * numCellsY;
    int visitedCellCount = 1; // Start cell is visited

    // --- Data structures for random walk ---
    // path: Stores {x,y} coords of the current walk path. Max size needed.
    int (*path)[2] = malloc((size_t)mazeSize * mazeSize * sizeof(int[2]));
    // visited_in_walk: Stores step index+1 where a cell was visited *in this walk*. 0 if not visited.
    // Used for loop detection/erasure. Indexed by [x * mazeSize + y].
    int *visited_in_walk = calloc((size_t)mazeSize * mazeSize, sizeof(int));

    if (!path || !visited_in_walk) {
        perror("Error allocating memory in Wilson's algorithm");
        free(path); free(visited_in_walk);
        exit(EXIT_FAILURE); // Critical error
    }

    // --- Main Wilson's Loop ---
    while (visitedCellCount < totalCellsToVisit) {
        // --- Pick a random unvisited cell to start walk ---
        int walkStartX, walkStartY;
        do {
            walkStartX = 1 + 2 * (rand() % numCellsX); // Random odd row
            walkStartY = 1 + 2 * (rand() % numCellsY); // Random odd col
        } while (maze[walkStartX][walkStartY] == PATH_CHAR); // Ensure it's not already in the maze

        // --- Perform Random Walk ---
        int pathLen = 0;
        int currentX = walkStartX;
        int currentY = walkStartY;

        // Walk until the path hits a cell already in the maze
        while (maze[currentX][currentY] != PATH_CHAR) {
            // Check for loop: Has this cell been visited *in this walk*?
            int visitedStep = visited_in_walk[currentX * mazeSize + currentY];
            if (visitedStep > 0) { // Loop detected!
                int loopStartIndex = visitedStep - 1; // 0-based index where loop began

                // Erase loop segment from visited_in_walk tracker
                for (int i = loopStartIndex; i < pathLen; i++) {
                     visited_in_walk[path[i][0] * mazeSize + path[i][1]] = 0;
                }
                // Truncate the 'path' array to remove the loop
                pathLen = loopStartIndex;
            }

            // Record current step in path and mark visited in this walk
            if (pathLen < mazeSize * mazeSize) { // Bounds check
                path[pathLen][0] = currentX;
                path[pathLen][1] = currentY;
                visited_in_walk[currentX * mazeSize + currentY] = pathLen + 1; // Store step index + 1
                pathLen++;
             } else {
                 fprintf(stderr, "Error: Wilson's path exceeded max length.\n");
                 break; // Break inner loop for safety
             }

            // Choose a random valid direction (move 2 cells)
            int dirs[4][2] = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}}; // N, S, W, E
            int valid_dirs[4];
            int validCount = 0;
            for (int i = 0; i < 4; i++) {
                int nextX = currentX + dirs[i][0];
                int nextY = currentY + dirs[i][1];
                // Check if next cell is within grid boundaries (1 to mazeSize-2)
                if (nextX >= 1 && nextX <= mazeSize - 2 && nextY >= 1 && nextY <= mazeSize - 2) {
                    valid_dirs[validCount++] = i;
                }
            }

            if (validCount == 0) {
                 fprintf(stderr, "Error: Wilson's walk stuck!\n"); break; // Should not happen
            }

            // Move to the next cell
            int chosenDirIndex = valid_dirs[rand() % validCount];
            currentX += dirs[chosenDirIndex][0];
            currentY += dirs[chosenDirIndex][1];
            // Loop condition handles hitting maze; loop detection handles hitting self
        } // End of single random walk

        // --- Carve Walked Path into Maze ---
        // (currentX, currentY) is now a cell that was already PATH_CHAR
        if (pathLen > 0) {
            // Iterate through the recorded path
            for (int i = 0; i < pathLen; i++) {
                int px = path[i][0];
                int py = path[i][1];

                // If cell isn't already part of maze (it shouldn't be unless loop erased back to it)
                if (maze[px][py] != PATH_CHAR) {
                    maze[px][py] = PATH_CHAR; // Add cell to maze
                    visitedCellCount++;
                }

                // Carve wall between this cell and *previous* cell in path
                if (i > 0) {
                    int prevX = path[i-1][0];
                    int prevY = path[i-1][1];
                    int wallX = (px + prevX) / 2;
                    int wallY = (py + prevY) / 2;
                    maze[wallX][wallY] = PATH_CHAR;
                }
            }

            // Carve wall between *last* cell of walk path and the maze cell it hit
            int lastPathX = path[pathLen - 1][0];
            int lastPathY = path[pathLen - 1][1];
            // (currentX, currentY) is the hit maze cell
            // Check distance == 2 to ensure direct adjacency before carving wall
            if (abs(lastPathX - currentX) + abs(lastPathY - currentY) == 2) {
                 int connectWallX = (lastPathX + currentX) / 2;
                 int connectWallY = (lastPathY + currentY) / 2;
                 maze[connectWallX][connectWallY] = PATH_CHAR;
            }
        } // End carving

        // --- Reset 'visited_in_walk' for the next walk ---
        if (pathLen > 0) {
            for (int i = 0; i < pathLen; i++) {
                 // Check bounds before access (safety)
                 if (path[i][0] >= 0 && path[i][0] < mazeSize && path[i][1] >= 0 && path[i][1] < mazeSize) {
                    visited_in_walk[path[i][0] * mazeSize + path[i][1]] = 0;
                 }
            }
        }
    } // End of while (visitedCellCount < totalCellsToVisit)

    // --- Cleanup ---
    free(path);
    free(visited_in_walk);
}

// === GAME LOOP & LOGIC FUNCTIONS ===

/*
 * The main game loop. Sets up rendering, then continuously
 * handles input and updates game state until a winner is determined or quit.
 */
void gameLoop() {
    setMazeColorScheme(); // Set the randomized color scheme
    startTime = time(NULL); // Record game start time
    renderMazeAndStatus();  // Initial render

    while (winner == 0) { // Loop while game is in progress
        handlePlayerInput(); // Check for and process player input
        Sleep(30); // Short delay to control game speed and reduce CPU usage
    }
    // Loop exits when 'winner' is non-zero (win or quit)
    // endGame() is called from within handlePlayerInput upon win/quit.
}

/*
 * Handles player input using _kbhit() and _getch().
 * Updates player positions, checks for collisions, bonus collection, win conditions.
 * Calls renderMazeAndStatus on valid move. Handles Quit (Q) and Restart (R).
 */
void handlePlayerInput() {
    // --- Check for key press ---
    if (!_kbhit()) {
        return; // No key pressed
    }

    // --- Read input ---
    char input = _getch(); // Read pressed key (may be extended prefix)
    int newX = playerX, newY = playerY;       // P1 potential new coords
    int newX2 = player2X, newY2 = player2Y;   // P2 potential new coords
    int movedPlayer = 0;                      // 0=no move, 1=P1, 2=P2
    int validMoveMade = 0;                    // Did a player successfully move?

    // --- Process Input ---
    // Player 1 (WASD)
    if (input == 'w' || input == 'W') { newX--; movedPlayer = 1; }
    else if (input == 'a' || input == 'A') { newY--; movedPlayer = 1; }
    else if (input == 's' || input == 'S') { newX++; movedPlayer = 1; }
    else if (input == 'd' || input == 'D') { newY++; movedPlayer = 1; }
    // Player 1 (Arrow Keys - only in single player)
    else if (gameMode == SINGLE_PLAYER && input == -32) {
        input = _getch(); // Read actual arrow key scan code
       if (input == UP_INPUT) { newX--; movedPlayer = 1; }
       else if (input == DOWN_INPUT) { newX++; movedPlayer = 1; }
       else if (input == LEFT_INPUT) { newY--; movedPlayer = 1; }
       else if (input == RIGHT_INPUT) { newY++; movedPlayer = 1; }
       else { movedPlayer = 0; } // Ignore other extended keys
   }

    // Player 2 (Arrow Keys - extended prefix)
    else if (gameMode == DUAL_PLAYER && input == -32) {
         input = _getch(); // Read actual arrow key scan code
        if (input == UP_INPUT) { newX2--; movedPlayer = 2; }
        else if (input == DOWN_INPUT) { newX2++; movedPlayer = 2; }
        else if (input == LEFT_INPUT) { newY2--; movedPlayer = 2; }
        else if (input == RIGHT_INPUT) { newY2++; movedPlayer = 2; }
        else { movedPlayer = 0; } // Ignore other extended keys
    }

    // Quit or Restart
    else if (input == 'q' || input == 'Q') {
        winner = -1; // Indicate quit
        endGame(0);  // End immediately, don't offer restart
        return;
    }
    else if (input == 'r' || input == 'R') {
        initializeAndStartGame(1); // Restart game, skipping prompts
        return;
    }

    // Ignore any other keys
    else {
        clearInputBuffer();
        return;
    }

    // --- Validate and Apply Move ---
    if (movedPlayer == 1) {
        if (isValidMove(newX, newY)) {
            playerX = newX; playerY = newY;
            player1Moves++;
            validMoveMade = 1;

            // Check bonus collection
            if (maze[playerX][playerY] == BONUS_CHAR) {
                player1Score += BONUS_POINTS;
                maze[playerX][playerY] = PATH_CHAR; // Remove dot
            }

            // Check win condition
            if (playerX == exitX && playerY == exitY) {
                winner = 1; // P1 wins
                endGame(1); // End game, offer restart
                return;
            }
        }
    } else if (movedPlayer == 2) {
        if (isValidMove(newX2, newY2)) {
            player2X = newX2; player2Y = newY2;
            player2Moves++;
            validMoveMade = 1;

            // Check bonus collection
            if (maze[player2X][player2Y] == BONUS_CHAR) {
                player2Score += BONUS_POINTS;
                maze[player2X][player2Y] = PATH_CHAR; // Remove dot
            }

            // Check win condition
            if (player2X == exitX && player2Y == exitY) {
                winner = 2; // P2 wins
                endGame(1); // End game, offer restart
                return;
            }
        }
    }

    // --- Render maze if a valid move was made ---
    if (validMoveMade) {
        renderMazeAndStatus();
    }
}

/*
 * Checks if a given coordinate (x, y) is a valid move target.
 * Valid if within maze boundaries (0 to mazeSize-1) and not a wall.
 */
int isValidMove(int x, int y) {
    return (x >= 0 && x < mazeSize && y >= 0 && y < mazeSize && maze[x][y] != WALL_CHAR);
}


// === RENDERING FUNCTIONS ===

/*
 * Renders the entire maze grid and the status information below it.
 * Uses the globally set color scheme (g_current...).
 * Note: Prints an extra space after each maze char, doubling horizontal width. This is for visual clarity.
 */
void renderMazeAndStatus() {
    showConsoleCursor(0); // Hide cursor during render
    gotoxy(0, 0);         // Move cursor to top-left

    WORD current_color;
    char current_char;

    // --- Render Maze Grid ---
    for (int i = 0; i < mazeSize; i++) {
        for (int j = 0; j < mazeSize; j++) {
            int isP1 = (i == playerX && j == playerY);
            int isP2 = (gameMode == DUAL_PLAYER && i == player2X && j == player2Y);

            if (isP1 && isP2) { // Players overlap
                current_char = OVERLAP_CHAR;
                current_color = g_currentOverlapColor;
            } else if (isP1) { // Only Player 1
                current_char = player1Char;
                current_color = g_currentPlayer1Color;
            } else if (isP2) { // Only Player 2
                current_char = player2Char;
                current_color = g_currentPlayer2Color;
            } else { // Cell contains maze element
                current_char = maze[i][j];
                switch (current_char) {
                    case WALL_CHAR:  current_color = g_currentWallColor; break;
                    case PATH_CHAR:  current_color = g_currentPathColor; break;
                    case EXIT_CHAR:  current_color = g_currentExitColor; break;
                    case BONUS_CHAR: current_color = g_currentBonusColor; break;
                    default:         current_color = g_currentDefaultColor; break;
                }
            }

            setConsoleColor(current_color);
            // Print character followed by a space for better visual spacing.
            printf("%c ", current_char);
        }
        setConsoleColor(COLOR_DEFAULT); // Reset color at end of line
        printf("\n"); // Move to next line
    }
    setConsoleColor(COLOR_DEFAULT); // Ensure reset after maze

    // --- Display Status Information Below Maze ---
    int statusRow = mazeSize; // Start right below maze
    setConsoleColor(COLOR_INFO);

    // Time
    gotoxy(0, statusRow + 1);
    displayTime();

    // Scores and Moves
    gotoxy(0, statusRow + 2);
    printf("P1("); setConsoleColor(g_currentPlayer1Color); printf("%c", player1Char); setConsoleColor(COLOR_INFO); printf(") Score: %-4d Moves: %-4d", player1Score, player1Moves);
    if (gameMode == DUAL_PLAYER) {
        printf(" | P2("); setConsoleColor(g_currentPlayer2Color); printf("%c", player2Char); setConsoleColor(COLOR_INFO); printf(") Score: %-4d Moves: %-4d", player2Score, player2Moves);
    }
    printf("        "); // Clear rest of line

    // Controls Help
    gotoxy(0, statusRow + 3);
    printf("P1("); setConsoleColor(g_currentPlayer1Color); printf("%c", player1Char); setConsoleColor(COLOR_INFO); printf(") = WASD "); 
    if (gameMode == SINGLE_PLAYER) printf("or Arrows | ");
    if (gameMode == DUAL_PLAYER) {
        printf("| P2("); setConsoleColor(g_currentPlayer2Color); printf("%c", player2Char); setConsoleColor(COLOR_INFO); printf(") = Arrows | ");
    }
    printf("Q = Quit | R = Restart");
    printf("        "); // Clear rest of line

    // Clear a few extra lines below status
    for (int i = 4; i < 8; ++i) {
        clearConsoleLine(statusRow + i);
    }

    setConsoleColor(COLOR_DEFAULT); // Final color reset
    // Cursor remains hidden
}

/*
 * Calculates and displays the elapsed game time.
 */
void displayTime() {
    time_t currentTime = time(NULL);
    int elapsedTime = (int)difftime(currentTime, startTime);
    printf("Time Elapsed: %d seconds", elapsedTime);
    printf("        "); // Padding to overwrite previous times
}

// === GAME END & SCORING FUNCTIONS ===

/*
 * Prompts the winning player to save their score to "maze_scores.txt".
 * Handles name input and file writing (append mode). Uses fopen_s if MSVC.
 */
void saveScore(int winningPlayer, int score, int moves, int timeTaken) {
    int promptRow = mazeSize + 5; // Position below status lines

    // --- Ask to save ---
    gotoxy(0, promptRow);
    clearConsoleLine(promptRow); clearConsoleLine(promptRow + 1);
    setConsoleColor(COLOR_INFO);
    printf("Player %d Wins! Save score? (Y/n): ", winningPlayer);
    showConsoleCursor(1);
    clearInputBuffer();
    char confirm = _getch();
    clearInputBuffer();
    showConsoleCursor(0);
    clearConsoleLine(promptRow);

    if (confirm == 'n' || confirm == 'N') {
        return; // Don't save
    }

    // --- Get Player Name ---
    char playerName[50] = {0};
    int namePromptRow = promptRow + 1;
    gotoxy(0, namePromptRow); clearConsoleLine(namePromptRow); clearConsoleLine(namePromptRow + 1);
    setConsoleColor(COLOR_INFO);
    showConsoleCursor(1);
    printf("Enter your name (max 49 chars): ");
    clearInputBuffer();
    if (fgets(playerName, sizeof(playerName), stdin) != NULL) {
        playerName[strcspn(playerName, "\n")] = 0; // Remove trailing newline
        if (playerName[0] == '\0') { // Handle empty input
            strcpy(playerName, "Anonymous");
        }
    } else { // Handle fgets error
        setConsoleColor(COLOR_ERROR);
        gotoxy(0, namePromptRow + 1); printf("Input error reading name. Using 'Anonymous'.");
        setConsoleColor(COLOR_INFO);
        strcpy(playerName, "Anonymous");
        clearerr(stdin);
        Sleep(1500);
    }
    showConsoleCursor(0);
    clearConsoleLine(namePromptRow); clearConsoleLine(namePromptRow + 1);

    // --- Get Current Date/Time ---
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char dateTimeStr[26];
    strftime(dateTimeStr, sizeof(dateTimeStr), "%Y-%m-%d %H:%M:%S", tm_info); // Format: YYYY-MM-DD HH:MM:SS

    // --- Open Score File (Append Mode) ---
    FILE *scoreFile = NULL;
    int fileStatusRow = namePromptRow; // Row for status messages
    const char* scoreFilename = "maze_scores.txt";

    #ifdef _MSC_VER // Use fopen_s on MSVC (Visual Studio)
        errno_t err = fopen_s(&scoreFile, scoreFilename, "a");
        if (err != 0 || scoreFile == NULL) {
            gotoxy(0, fileStatusRow); setConsoleColor(COLOR_ERROR);
            fprintf(stderr, "Error %d opening score file '%s'. Score not saved.", err, scoreFilename);
            setConsoleColor(COLOR_INFO); Sleep(2500); clearConsoleLine(fileStatusRow);
            return; // Exit if file cannot be opened
        }
    #else // Standard C fopen otherwise
        scoreFile = fopen(scoreFilename, "a");
        if (scoreFile == NULL) {
            gotoxy(0, fileStatusRow); setConsoleColor(COLOR_ERROR);
            perror("Error opening score file. Score not saved");
            setConsoleColor(COLOR_INFO); Sleep(2500); clearConsoleLine(fileStatusRow);
            return; // Exit function
        }
    #endif

    // --- Write Score Data ---
    int writeError = 0;
    char winnerIcon = (winningPlayer == 1) ? player1Char : player2Char;

    // Use fprintf and check return value (< 0 indicates error)
    if (fprintf(scoreFile, "Winner: Player %d (%c)\n", winningPlayer, winnerIcon) < 0) writeError = 1;
    if (fprintf(scoreFile, "Name: %s\n", playerName) < 0) writeError = 1;
    if (fprintf(scoreFile, "Score (Dots): %d\n", score) < 0) writeError = 1;
    if (fprintf(scoreFile, "Moves: %d\n", moves) < 0) writeError = 1;
    if (fprintf(scoreFile, "Time Taken: %d seconds\n", timeTaken) < 0) writeError = 1;
    if (fprintf(scoreFile, "Maze Seed: %u\n", seed) < 0) writeError = 1;
    if (fprintf(scoreFile, "Maze Size: %d\n", mazeSize) < 0) writeError = 1;
    if (fprintf(scoreFile, "Game Mode: %s\n", (gameMode == SINGLE_PLAYER ? "Single Player" : "Dual Player")) < 0) writeError = 1;
    if (fprintf(scoreFile, "Completed On: %s\n", dateTimeStr) < 0) writeError = 1;
    if (fprintf(scoreFile, "------------------------------------\n") < 0) writeError = 1;

    // --- Close File and Display Status ---
    fflush(scoreFile); // Ensure data written
    if (fclose(scoreFile) != 0) { // Check close error
        writeError = 1;
        perror("Error closing score file");
    }

    int confirmRow = fileStatusRow;
    gotoxy(0, confirmRow);
    if (writeError) {
        setConsoleColor(COLOR_ERROR);
        fprintf(stderr, "Error writing score data. Check file permissions/disk space.");
    } else {
        setConsoleColor(COLOR_SUCCESS);
        printf("Score saved successfully to %s!", scoreFilename);
    }
    setConsoleColor(COLOR_INFO);
    Sleep(2000); // Pause to show message
    clearConsoleLine(confirmRow); // Clear message
}

/*
 * Handles the end of the game: displays win/loss/quit message, calls saveScore,
 * shows ASCII art, and prompts for restart or exit.
 */
void endGame(int offerRestart) {
    time_t endTime = time(NULL);
    int elapsedTime = (int)difftime(endTime, startTime);
    int finalMsgRow = 1; // Start messages near top after clear

    system("cls"); // Clear screen

    // --- Display Win/Quit Message & Stats ---
    gotoxy(0, finalMsgRow++);
    setConsoleColor(COLOR_INFO);
    if (winner == 1) { // P1 won
        printf("Congratulations Player 1 (");
        setConsoleColor(g_currentPlayer1Color); printf("%c", player1Char); setConsoleColor(COLOR_INFO);
        printf(")! You reached the exit!");
        gotoxy(0, finalMsgRow++);
        printf("Time: %d s | Score: %d | Moves: %d", elapsedTime, player1Score, player1Moves);
        // Call saveScore after displaying win message
        saveScore(1, player1Score, player1Moves, elapsedTime);
        finalMsgRow += 3; // Leave space for saveScore messages
    } else if (winner == 2) { // P2 won
        printf("Congratulations Player 2 (");
        setConsoleColor(g_currentPlayer2Color); printf("%c", player2Char); setConsoleColor(COLOR_INFO);
        printf(")! You reached the exit!");
        gotoxy(0, finalMsgRow++);
        printf("Time: %d s | Score: %d | Moves: %d", elapsedTime, player2Score, player2Moves);
        saveScore(2, player2Score, player2Moves, elapsedTime);
        finalMsgRow += 3;
    } else if (winner == -1) { // Game quit
        printf("Game Quit. Time played: %d seconds.", elapsedTime);
        finalMsgRow += 2;
    } else { // Fallback
        printf("Game Over!");
        finalMsgRow += 2;
    }

    // --- Display ASCII Art "Game Over" ---
    // Position adjusted based on where messages ended
    gotoxy(0, finalMsgRow++);
    setConsoleColor(COLOR_OVERLAP); // Use a different color

    // ASCII Art (Multi-line requires multiple gotoxy/printf or careful spacing)
    // Adding gotoxy for each line ensures alignment

    gotoxy(0, finalMsgRow++); printf("         88                                 88                  \n");
    gotoxy(0, finalMsgRow++); printf("   ,d    88                                 88                  \n");
    gotoxy(0, finalMsgRow++); printf("   88    88                                 88                  \n");
    gotoxy(0, finalMsgRow++); printf(" MM88MMM 88,dPPYba,  ,adPPYYba, 8b,dPPYba,  88   ,d8  ,adPPYba, \n");
    gotoxy(0, finalMsgRow++); printf("   88    88P'    \"8a\" \"\"     `Y8 88P'   `\"8a 88 ,a8\"   I8[    \"\" \n");
    gotoxy(0, finalMsgRow++); printf("   88    88       88 ,adPPPPP88 88       88 8888[      `\"Y8ba,  \n");
    gotoxy(0, finalMsgRow++); printf("   88,   88       88 88,    ,88 88       88 88`\"Yba,  aa    ]8I \n");
    gotoxy(0, finalMsgRow++); printf("   \"Y888 88       88 `\"8bbdP\"Y8 88       88 88   `Y8a `\"YbbdP\"' \n\n\n"); 
    
    gotoxy(0, finalMsgRow++); printf("    ad88                      \n");
    gotoxy(0, finalMsgRow++); printf("   d8\"                        \n");
    gotoxy(0, finalMsgRow++); printf("   88                         \n");
    gotoxy(0, finalMsgRow++); printf(" MM88MMM ,adPPYba,  8b,dPPy   \n");
    gotoxy(0, finalMsgRow++); printf("   88   a8\"     \"8a 88P'      \n");
    gotoxy(0, finalMsgRow++); printf("   88   8b       d8 88        \n");
    gotoxy(0, finalMsgRow++); printf("   88   \"8a,   ,a8\" 88        \n");
    gotoxy(0, finalMsgRow++); printf("   88    `\"YbbdP\"'  88        \n\n\n");
    
    gotoxy(0, finalMsgRow++); printf("             88\n");
    gotoxy(0, finalMsgRow++); printf("             88                         88\n");
    gotoxy(0, finalMsgRow++); printf("             88\n");
    gotoxy(0, finalMsgRow++); printf(" 8b,dPPYba,  88 ,adPPYYba, 8b       d8  88 8b,dPPYba,   ,adPPYb,d8 \n");
    gotoxy(0, finalMsgRow++); printf(" 88P'    \"8a 88 \"\"     `Y8 `8b     d8'  88 88P'   `\"8a a8\"    `Y88 \n");
    gotoxy(0, finalMsgRow++); printf(" 88       d8 88 ,adPPPPP88  `8b   d8'   88 88       88 8b       88 \n");
    gotoxy(0, finalMsgRow++); printf(" 88b,   ,a8\" 88 88,    ,88   `8b,d8'    88 88       88 \"8a,   ,d88 \n");
    gotoxy(0, finalMsgRow++); printf(" 88`YbbdP\"'  88 `\"8bbdP\"Y8     Y88'     88 88       88  `\"YbbdP\"Y8 \n");
    gotoxy(0, finalMsgRow++); printf(" 88                            d8'                      aa,    ,88 \n");
    gotoxy(0, finalMsgRow++); printf(" 88                           d8'                        \"Y8bbdP\"  \n");
    gotoxy(0, finalMsgRow++); printf(" \n");

    // --- Prompt for Restart or Exit ---
    int exitPromptRow = finalMsgRow + 1; // Position below ASCII art
    gotoxy(0, exitPromptRow); clearConsoleLine(exitPromptRow);
    setConsoleColor(COLOR_INFO);
    showConsoleCursor(1); // Show cursor for final input

    if (offerRestart && winner > 0) { // Only offer restart on normal win
        printf("Press 'R' to play again (same settings), or any other key to exit...");
        clearInputBuffer();
        char choice = _getch();
        clearInputBuffer();
        showConsoleCursor(0);
        if (choice == 'r' || choice == 'R') {
            // Restart game, skipping prompts, uses new random seed set in initializeAndStartGame
            initializeAndStartGame(1);
            return; // Return to prevent exit(0) below
        }
        // Fall through to exit if not 'r' or 'R'
    } else { // Game quit or no restart offered
        printf("Press any key to exit...");
        clearInputBuffer();
        _getch(); // Wait for key press
        clearInputBuffer();
    }

    // --- Cleanup and Exit ---
    freeMaze(); // Free maze memory
    showConsoleCursor(1); // Ensure cursor visible on exit
    setConsoleColor(COLOR_DEFAULT); // Reset console color
    gotoxy(0, exitPromptRow + 2); // Move cursor down
    system("cls"); // Clear screen one last time
    exit(0); // Terminate program successfully
}

// === MAIN GAME CONTROLLER FUNCTION ===

/*
 * Initializes a new game or restarts. Handles setup prompts (if needed),
 * maze generation (with validation/retries), and starts the game loop.
 */
void initializeAndStartGame(int skipWelcome) {
    if (maze != NULL) { // Free existing maze if restarting
        freeMaze();
    }
    system("cls"); // Clear screen at start of init

    // --- Get Game Settings ---
    if (!skipWelcome) {
        displayWelcomeScreen(); // Show title, objective

        // --- Quick Play / Custom Game Choice ---
        int choiceRow = 12;
        gotoxy(0, choiceRow); setConsoleColor(COLOR_INFO);
        printf("Choose Game Type:\n");
        gotoxy(0, choiceRow + 1); printf("  1. Quick Play (Solo, Size 21, Prim's Algoritm Maze)");
        gotoxy(0, choiceRow + 2); printf("  2. Custom Game (Set Options)");
        gotoxy(0, choiceRow + 3); printf("Press 2 for Custom Game, any key for Quick Play: ");

        clearInputBuffer();
        char choice = _getch(); // Get choice immediately
        showConsoleCursor(0); // Hide cursor after choice

        // Clear choice prompt area
        clearConsoleLine(choiceRow);
        clearConsoleLine(choiceRow + 1);
        clearConsoleLine(choiceRow + 2);
        clearConsoleLine(choiceRow + 3);

        if (choice == '2') { // --- CUSTOM GAME ----
            gotoxy(0, choiceRow); // Show confirmation briefly
            setConsoleColor(COLOR_INFO); printf("Custom Game selected. Configure options...");// Sleep(1000);
            //clearConsoleLine(choiceRow);
            // Run setup prompts for custom game
            // Note: Default globals are set before this function, prompts will overwrite them
            promptForGameMode();      // Sets gameMode
            promptForPlayerIcons();   // Sets player1Char, player2Char
            getMazeSize();          // Sets mazeSize
            seed = getSeed();         // Sets seed
            promptForAlgorithm();     // Sets generationAlgorithmChoice

         
        } else { // --- QUICK PLAY ---
            gotoxy(0, choiceRow); // Show confirmation briefly
            setConsoleColor(COLOR_SUCCESS); printf("Quick Play selected!"); //Sleep(1000);
            //clearConsoleLine(choiceRow);

         gameMode = SINGLE_PLAYER;
         player1Char = DEFAULT_PLAYER1_CHAR;
         mazeSize = DEFAULT_SIZE; // Use the default size (21)
         generationAlgorithmChoice = DEFAULT_ALGORITHM; // Use Prim's
         seed = (unsigned int)time(NULL); // Use random seed
         if (seed == 0) seed = 1; // Ensure seed is not 0
         player2Char = DEFAULT_PLAYER2_CHAR; // Set P2 char even if not used

        }
    } else {
        // Quick restart: Keep existing settings (mode, icons, size, algo)
        seed = (unsigned int)time(NULL); // Generate a new random seed for variety
        if (seed == 0) seed = 1;
    }

    srand(seed); // Seed the random number generator

    // --- Generate Maze with Validation (Exit Reachability Check) ---
    mazeRegenCount = 0; // Reset counter for this generation sequence
    int generationStatusRow = 15; // Position for status messages
    int generationResultRow = generationStatusRow + 1;
    int generationPromptRow = generationResultRow + 2; // For user interaction on failure
    const int maxAutoRetries = 5; // Max attempts before asking user

    do { // Loop until valid maze generated or user quits
        // Clear previous generation status lines
        clearConsoleLine(generationStatusRow);
        clearConsoleLine(generationResultRow);
        clearConsoleLine(generationPromptRow);
        for (int i = 1; i < 5; ++i) clearConsoleLine(generationPromptRow + i); // Clear below too

        // Display generation status
        gotoxy(0, generationStatusRow);
        setConsoleColor(COLOR_INFO);
        const char* algoName = "Unknown"; // Determine algorithm name
        switch(generationAlgorithmChoice) {
           case 1: algoName="Prim's"; break;
           case 2: algoName="Kruskal's"; break;
           case 3: algoName="Wilson's"; break;
           case 4: algoName="Recursive Backtracker"; break;
           case 5: algoName="Recursive Backtracker (Alternative)"; break;
        }
        /*
        "Prim's Algorithm : Cellular Expansion",
        "Kruskal's Algorithm : Wall Bridging",
        "Wilson's Algorithm : Wandering Paths",
        "Recursive Backtracker : Deep First Search",
        "Recursive Backtracker (Alternative) : Iterative Loops"
        */
        printf("Generating maze: %s | Size: %d | Seed: %u ...", algoName, mazeSize, seed);
        showConsoleCursor(0); // Hide cursor

        // Initialize maze state (allocates, calls gen algo, places exit/dots/players)
        initializeMazeState();

        // Validate reachability
        if (isExitReachable()) {
            // Success!
            clearConsoleLine(generationResultRow); gotoxy(0, generationResultRow);
            setConsoleColor(COLOR_SUCCESS);
            printf("Maze generated successfully! Starting game...");
            Sleep(1500); // Short pause
            break; // Exit generation loop
        } else {
            // Failure: Exit unreachable
            mazeRegenCount++;
            clearConsoleLine(generationResultRow); gotoxy(0, generationResultRow);
            setConsoleColor(COLOR_ERROR);
            printf("Maze validation failed: Exit unreachable! (Attempt %d)", mazeRegenCount);
            setConsoleColor(COLOR_INFO);

            // Check if max automatic retries exceeded
            if (mazeRegenCount >= maxAutoRetries) {
                clearConsoleLine(generationPromptRow);
                gotoxy(0, generationPromptRow);
                printf("Failed %d times. Retry new seed (R), Change Algorithm (C), Quit (Q)? ", maxAutoRetries);
                showConsoleCursor(1); // Show cursor for input

                clearInputBuffer();
                char choice = _getch();
                clearInputBuffer();
                showConsoleCursor(0);
                clearConsoleLine(generationPromptRow); // Clear prompt

                if (choice == 'c' || choice == 'C') {
                    freeMaze(); 
                    promptForAlgorithm();
                    mazeRegenCount = 0; 
                    srand(seed);
                    continue; 
                } else if (choice == 'q' || choice == 'Q') {
                    gotoxy(0, generationPromptRow + 1); printf("Exiting generation.\n");
                    showConsoleCursor(1); setConsoleColor(COLOR_DEFAULT);
                    freeMaze();
                    exit(1);
                } else {
                    mazeRegenCount = 0;
                }
            }

            // --- Prepare for next attempt (auto or after user prompt) ---
            freeMaze(); 
            seed++;
            if (seed == 0) seed = 1; 
            srand(seed); 
            Sleep(200);
        }
    } while (1); // Loop continues until `break` on success

    // Clear generation status messages before starting game
    clearConsoleLine(generationStatusRow);
    clearConsoleLine(generationResultRow);
    clearConsoleLine(generationPromptRow);
    system("cls"); // Clear entire screen for game start

    // Start the main game loop
    gameLoop();
}