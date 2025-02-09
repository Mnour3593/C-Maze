# C-Maze

**C-Maze** is a terminal-based maze game where players navigate a randomly generated maze to find the exit. Use the **WASD** keys to move, and track your time and move count as you try to reach the goal.

### 🧩 Features:
- Customizable maze size (odd numbers only due to a bug).
- Randomly generated maze using a backtracking algorithm.
- Displays move counter and timer during and after a game session to track your performance.
- Simple terminal-based controls for easy gameplay.
- Adjustable seed for different maze patterns.
- Clear game-over screen with option to restart or quit.


### 🎮 Controls:
- **W**: Move up
- **A**: Move left
- **S**: Move down
- **D**: Move right
- **Q**: Quit the game
- **R**: Restart the game

### 🚀 How to Play:
1. Run the program in your terminal.
2. Choose the size of the maze or use the default one.
3. Enter a seed for a specific map or let it generate randomly.
4. Navigate through the maze using the **WASD** keys to reach the exit.
6. Press **Q** to quit the game or **R** to restart if you want to try again or if the maze is impossible.

### ⚙️ Installation:
1. Clone this repository:
    ```bash
    git clone https://github.com/Mnour3593/C-Maze
    ```

2. Navigate to the project folder:
   ```bash
   cd C-Maze

3. Compile and run:
   ```bash
    gcc maze_game.c -o maze_game
    ./maze_game

### 🔄 Alternative Installation:
1. Copy the source code.
2. Paste it in your preferred editor.
3. Compile and Run.

### 🐛 Known Bugs:

1. The maze may occasionally generate with a path that leads to a dead-end, requiring a restart.
2. Player movement could be slightly delayed on slower systems.
3. The program may crash on rare occasions if the random seed produces an unusual pattern in maze generation. Try entering your lucky numbers as seeds. (Mine's 47)

### 🙏 Acknowledgements:
The inspiration for this project stemmed from a personal idea of mine, with ChatGPT offering valuable advice that influenced the overall design. I spent countless hours communicating, debugging, and refining the code myself, with ChatGPT stepping in to assist with specific challenges, like structuring the code and adding features. While I did the heavy lifting, the collaboration helped polish and improve the game, making the end result more effective and polished. Special thanks to my professor, Prof. Dr. Ahmet Gürhanlı, for his valuable lectures and teachings, which helped guide me through the development process of this project. 🎓


License:

This project is open-source and available under the MIT License.
