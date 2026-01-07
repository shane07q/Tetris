# 🧱 Console Tetris in C++

A robust, feature-rich implementation of the classic Tetris game, built entirely in C++ for the Windows Console. This project utilizes the Windows API to deliver a smooth, colorful, and interactive gaming experience directly in your terminal.

## ✨ Features

* **Full Tetris Mechanics:** Includes all 7 Tetromino shapes (I, O, T, S, Z, J, L) with proper rotation logic.
* **Modern Gameplay Elements:**
    * **Hold Piece:** Swap the current piece with a held piece for strategic play.
    * **Next Piece Preview:** See up to 3 upcoming pieces.
    * **Hard Drop:** Instantly drop pieces to the bottom.
* **Dynamic Difficulty:** Select from Easy, Medium, or Hard modes. The game speed increases as you clear lines and level up.
* **Customizable Board:** Set your preferred board width and height at the start of the game.
* **Persistent High Scores:** Saves player names and scores to `tetrisHighScore.txt` so you can compete against yourself or friends.
* **Colorful Graphics:** Uses Windows Console colors to distinguish different Tetromino shapes.
* **Pause/Resume:** Need a break? Pause the game at any time.

## 💻 Tech Stack

* **Language:** C++
* **Platform:** Windows (Requires `windows.h` and `conio.h`)
* **Libraries:** Standard Template Library (STL) vectors and maps.

> **⚠️ Note:** This game is designed specifically for **Microsoft Windows** due to the reliance on `<windows.h>` for console manipulation (colors, cursor hiding) and `<conio.h>` for non-blocking input.

## 🎮 Controls

| Key | Action |
| :---: | :--- |
| **A** | Move Left |
| **D** | Move Right |
| **W** | Rotate Clockwise |
| **S** | Rotate Counter-Clockwise |
| **Spacebar** | Hard Drop (Instant Place) |
| **C** | Hold / Swap Piece |
| **P** | Pause Game |
| **R** | Resume Game |

## 🚀 How to Run

### Prerequisites
1.  A Windows Operating System.
2.  A C++ Compiler (MinGW, TDM-GCC, or MSVC).

### Compilation & Execution

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/your-username/console-tetris-cpp.git](https://github.com/your-username/console-tetris-cpp.git)
    cd console-tetris-cpp
    ```

2.  **Compile the code:**
    If using G++ (MinGW):
    ```bash
    g++ main.cpp -o tetris.exe
    ```

3.  **Run the game:**
    ```bash
    ./tetris.exe
    ```

4.  **Initial Setup:**
    When the game starts, you will be prompted to enter the **Board Width** and **Height**.
    * *Recommended:* Width 10, Height 20.

## 🏆 Scoring System

Points are awarded based on the number of lines cleared simultaneously, multiplied by the current level:

* **1 Line:** 100 points × Level
* **2 Lines:** 300 points × Level
* **3 Lines:** 500 points × Level
* **4 Lines (Tetris):** 800 points × Level

*The level increases every 10 lines cleared, increasing the drop speed.*

## 📂 File Structure

* `main.cpp`: Contains the entire source code (Game logic, rendering, menu system).
* `tetrisHighScore.txt`: Automatically created/updated to store high scores.

## 🔮 Future Improvements

* Implement "Ghost Piece" (shadow) to show where the piece will land.
* Add sound effects using the Windows multimedia library.
* Implement "Wall Kicks" (SRS) for smoother rotation near edges.

## 📄 License

This project is open-source and available under the [MIT License](LICENSE).

---
By Shane Christian.
