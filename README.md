
# 🎮 C-Maze

**C-Maze** is a terminal-based maze game where players navigate randomly generated mazes to reach the exit as fast as possible.  
Originally developed as my **first C project**, C-Maze has evolved from a simple single-player experience into a fully-featured V2 version packed with new modes, visuals, and gameplay improvements!

---

## 🛠 About the Project

- **V1 - Base Game**  
  The original version focused on delivering a clean, simple gameplay experience:
  - Randomly generated mazes using recursive backtracking.
  - Single-player gameplay with basic WASD controls.
  - Timer and move counter to track performance.
  - Customizable maze size and seed-based generation.

- **V2 - Enhanced Version**  
  After gaining more experience and experimenting with new ideas (with the help of AI brainstorming via Google Gemini), C-Maze expanded into a more dynamic and feature-rich project:
  - Multiple maze generation algorithms (Prim's, Kruskal's, Wilson's, Recursive Backtracking variants).
  - Single and **Dual Player** modes.
  - Bonus collectibles ('.') for scoring.
  - Colorful Windows terminal visuals.
  - Quick Play and fully Customizable Game modes.
  - Score saving to `maze_scores.txt`.
  - Improved UI and a more polished player experience.

> **Note:** While some V2 ideas were inspired through AI brainstorming, I personally spent countless hours fine-tuning, debugging, testing, and learning deeper programming concepts to bring the project to life.

---

## ✨ Key Features

- Single and Dual Player modes.
- Multiple maze generation algorithms.
- Bonus collectibles and scoring system.
- Vibrant Windows console colors.
- Fully customizable maze setup.
- Guaranteed maze solvability checks.
- High-score saving functionality.

---

## 🎮 Controls

**Single Player / Player 1**
- **W**: Move Up
- **A**: Move Left
- **S**: Move Down
- **D**: Move Right

**Dual Player / Player 2**
- **Arrow Keys**: Move

> **Note:** Arrow keys also works for player 1 on single player mode.

**General Controls**
- **Q**: Quit game
- **R**: Restart game

---

## 🚀 How to Play

1. Compile the project using GCC or MSVC (see Installation) or run the .exe file.
2. Choose Quick Play for a fast start or set up a Custom Game.
3. Navigate to the exit ('E') using your controls, collecting bonuses for higher scores.
4. In Dual Mode, compete to be the first player reaching the exit!
5. Save your high score after completing the maze.

---

## 🖥 Installation (Windows Only)

```bash
git clone https://github.com/Mnour3593/C-Maze
cd C-Maze
gcc c-maze-v2.c -o c-maze-v2 -lws2_32 -lwinmm
./c-maze-v2.exe
```
Or compile with Visual Studio/MSVC (adjust linking flags if necessary).

---

## 📄 Example High Score Entry

```txt
------------------------------------
Winner: Player 2 (S)
Name: Muhammet Nur
Score (Dots): 30
Moves: 37
Time Taken: 12 seconds
Maze Seed: 1745520772
Maze Size: 21
Game Mode: Dual
Completed On: 2025-04-24 21:53:16
------------------------------------
```
- Score saves to `maze_scores.txt` created in the `.exe` file's directory.
---

## 📈 Evolution Timeline

- **February 2025** — First idea of developing my own game.
- **March 2025** — Began development of **C-Maze V1** as my first C project.
- **April 2025** — Completed V1, uploaded to GitHub.
- **Mid-April 2025** — Started planning V2 after learning more algorithms and Windows console handling. And uploaded V1 to itch.io
- **Late April 2025** — Released **C-Maze V2** featuring major updates like multiple algorithms, scoring, Dual Player mode, and visuals.

---

## 🙏 Acknowledgements

Special thanks to **Prof. Dr. Ahmet Gürhanlı** for his invaluable lectures that helped shape my coding foundation.  
Thanks to **OpenAI ChatGPT** for V1, and **Google Gemini** for V2's development.  
While some of the feature inspirations came from AI tools, the design, debugging, coding, and final polish were driven by my own learning process and persistence.

---

## 📜 License

This project is open-source and available under the [MIT License](LICENSE).

---

---

Would you also like me to quickly design a **badges** section (with C language, Windows, Terminal Game, etc. badges) at the top if you want it to look even cooler? 🎖️🚀  
It’s super easy to add!
