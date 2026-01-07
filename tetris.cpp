#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#include <fstream>
using namespace std;

const int BLOCK_SIZE = 4;
const int PAD_WIDTH = 80; // fixed width for output lines

enum Difficulty { EASY, MEDIUM, HARD };
const int INITIAL_SPEED[] = {300, 200, 100}; // drop speed (ms)

const int SHAPES[7][BLOCK_SIZE][BLOCK_SIZE] = {
    // I
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    // O
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
    // T
    {{0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0}},
    // S
    {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
    // Z
    {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
    // J
    {{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
    // L
    {{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}}
};

enum ConsoleColor {
    CYAN = 11, YELLOW = 14, MAGENTA = 13, GREEN = 10,
    RED = 12, BLUE = 1, BROWN = 6, BOUNDARY = 9, WHITE = 15
};

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void resetColor() { setColor(WHITE); }

void gotoxy(int x, int y) {
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void clearLine(int y) {
    gotoxy(0, y);
    cout << string(PAD_WIDTH, ' ');
}

void printLine(int y, const string &text, int color = WHITE) {
    clearLine(y);
    gotoxy(0, y);
    setColor(color);
    cout << text << string(max(0, PAD_WIDTH - (int)text.size()), ' ');
    resetColor();
}

// ****************************************
// MENU FUNCTIONS
// ****************************************

int menu(const string &title, const vector<string> &options,
         int titleColor = MAGENTA, int optionColor = WHITE) {
    int selected = 0;
    while (true) {
        system("cls");
        int line = 2;
        printLine(line++, title, titleColor);
        line++;
        for (int i = 0; i < (int)options.size(); i++) {
            string optStr = (i == selected ? "-> " : "   ") + options[i];
            printLine(line++, optStr, optionColor);
        }
        char key = _getch();
        if (key == 72) {         // up arrow
            selected = (selected - 1 + options.size()) % options.size();
        } else if (key == 80) {  // down arrow
            selected = (selected + 1) % options.size();
        } else if (key == '\r') {// Enter
            return selected;
        }
    }
}

// ****************************************
// HIGH SCORE FUNCTIONS (using a map)
// ****************************************

map<string, int> loadHighScores(const string &filename) {
    map<string, int> hs;
    ifstream infile(filename);
    if (infile.is_open()) {
        string name;
        int score;
        while (infile >> name >> score) {
            hs[name] = score;
        }
        infile.close();
    }
    return hs;
}

void saveHighScores(const string &filename, const map<string, int> &hs) {
    ofstream outfile(filename);
    if (outfile.is_open()) {
        for (auto &p : hs) {
            outfile << p.first << " " << p.second << "\n";
        }
        outfile.close();
    }
}

// ****************************************
// TETROMINO CLASS
// ****************************************

class Tetromino {
public:
    int shape[BLOCK_SIZE][BLOCK_SIZE];
    int x, y;
    int type;
    int color;
    
    // Spawns near the top: y = -1 (partial off-screen)
    Tetromino(int boardWidth) {
        type = rand() % 7;
        for (int i = 0; i < BLOCK_SIZE; i++)
            for (int j = 0; j < BLOCK_SIZE; j++)
                shape[i][j] = SHAPES[type][i][j];
        x = boardWidth / 2 - BLOCK_SIZE / 2;
        y = -1;
        switch (type) {
            case 0: color = CYAN;    break;
            case 1: color = YELLOW;  break;
            case 2: color = MAGENTA; break;
            case 3: color = GREEN;   break;
            case 4: color = RED;     break;
            case 5: color = BLUE;    break;
            case 6: color = BROWN;   break;
        }
    }
    
    void rotate() {
        int temp[BLOCK_SIZE][BLOCK_SIZE];
        for (int i = 0; i < BLOCK_SIZE; i++)
            for (int j = 0; j < BLOCK_SIZE; j++)
                temp[j][BLOCK_SIZE - 1 - i] = shape[i][j];
        for (int i = 0; i < BLOCK_SIZE; i++)
            for (int j = 0; j < BLOCK_SIZE; j++)
                shape[i][j] = temp[i][j];
    }
    
    void rotateCounter() {
        for (int i = 0; i < 3; i++) rotate();
    }
};

// ****************************************
// TETRIS GAME CLASS
// ****************************************

class TetrisGame {
private:
    int boardWidth, boardHeight;
    vector<vector<int>> grid;
    vector<vector<int>> colorGrid;
    Tetromino currentPiece;
    vector<Tetromino> nextPieces;
    Tetromino *holdPiece = nullptr;
    bool canHold = true;
    
    int score, level, linesCleared;
    bool paused = false;
    string playerName;
    Difficulty difficulty;
    int dropSpeed; // in ms

    bool isValidMove(int newX, int newY, const Tetromino &piece) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (piece.shape[i][j]) {
                    int gridX = newX + j;
                    int gridY = newY + i;
                    if (gridX < 0 || gridX >= boardWidth || gridY >= boardHeight)
                        return false;
                    if (gridY >= 0 && grid[gridY][gridX])
                        return false;
                }
            }
        }
        return true;
    }
    
    void mergePiece() {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (currentPiece.shape[i][j]) {
                    int gx = currentPiece.x + j;
                    int gy = currentPiece.y + i;
                    if (gy >= 0) {
                        grid[gy][gx] = 1;
                        colorGrid[gy][gx] = currentPiece.color;
                    }
                }
            }
        }
        canHold = true;
    }
    
    void clearLines() {
        int linesCompleted = 0;
        for (int i = boardHeight - 1; i >= 0; i--) {
            bool full = true;
            for (int j = 0; j < boardWidth; j++) {
                if (!grid[i][j]) {
                    full = false;
                    break;
                }
            }
            if (full) {
                grid.erase(grid.begin() + i);
                grid.insert(grid.begin(), vector<int>(boardWidth, 0));
                colorGrid.erase(colorGrid.begin() + i);
                colorGrid.insert(colorGrid.begin(), vector<int>(boardWidth, 0));
                linesCompleted++;
                i++; // recheck same index after shifting
            }
        }
        if (linesCompleted > 0) {
            switch (linesCompleted) {
                case 1: score += 100 * level; break;
                case 2: score += 300 * level; break;
                case 3: score += 500 * level; break;
                case 4: score += 800 * level; break;
            }
            linesCleared += linesCompleted;
            int oldLevel = level;
            level = 1 + (linesCleared / 10);
            if (level > oldLevel) updateSpeed();
        }
    }
    
    void updateSpeed() {
        dropSpeed = max(50, INITIAL_SPEED[difficulty] - (level - 1) * 10);
    }
    
    void generateNextPieces() {
        while ((int)nextPieces.size() < 3)
            nextPieces.push_back(Tetromino(boardWidth));
    }
    
    void render() {
        int line = 0;
        // --- Next Pieces ---
        printLine(line++, "Next Pieces:", MAGENTA);
        for (int i = 0; i < BLOCK_SIZE; i++) {
            clearLine(line);
            gotoxy(0, line);
            cout << "  ";
            for (int p = 0; p < (int)nextPieces.size(); p++) {
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    if (nextPieces[p].shape[i][j]) {
                        setColor(nextPieces[p].color);
                        cout << "#";
                        resetColor();
                    } else {
                        cout << " ";
                    }
                }
                cout << "  ";
            }
            line++;
        }
        line++;
        // --- Hold Piece ---
        printLine(line++, "Hold Piece:", MAGENTA);
        if (holdPiece) {
            for (int i = 0; i < BLOCK_SIZE; i++) {
                clearLine(line);
                gotoxy(0, line);
                cout << "  ";
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    if (holdPiece->shape[i][j]) {
                        setColor(holdPiece->color);
                        cout << "#";
                        resetColor();
                    } else {
                        cout << " ";
                    }
                }
                line++;
            }
        } else {
            printLine(line++, "  None");
        }
        line++;
        // --- Game Info ---
        printLine(line++, playerName + "'s Score: " + to_string(score), YELLOW);
        printLine(line++, "Level: " + to_string(level) + " | Lines: " + to_string(linesCleared), YELLOW);
        string diff = "Difficulty: ";
        diff += (difficulty == EASY ? "Easy" : (difficulty == MEDIUM ? "Medium" : "Hard"));
        printLine(line++, diff, YELLOW);
        line++;
        // --- Playfield ---
        string boundary = "+" + string(boardWidth, '-') + "+";
        printLine(line++, boundary, BOUNDARY);
        
        vector<vector<int>> tempGrid = grid;
        vector<vector<int>> tempColor = colorGrid;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (currentPiece.shape[i][j]) {
                    int gx = currentPiece.x + j;
                    int gy = currentPiece.y + i;
                    if (gy >= 0 && gy < boardHeight) {
                        tempGrid[gy][gx] = 1;
                        tempColor[gy][gx] = currentPiece.color;
                    }
                }
            }
        }
        for (int r = 0; r < boardHeight; r++) {
            gotoxy(0, line);
            cout << "|";
            for (int c = 0; c < boardWidth; c++) {
                if (tempGrid[r][c]) {
                    setColor(tempColor[r][c]);
                    cout << "#";
                    resetColor();
                } else {
                    cout << " ";
                }
            }
            cout << "|";
            int printed = boardWidth + 2;
            if (printed < PAD_WIDTH)
                cout << string(PAD_WIDTH - printed, ' ');
            line++;
        }
        printLine(line++, boundary, BOUNDARY);
        
        if (paused)
            printLine(line++, "Paused - Press 'r' to resume", RED);
    }
    
public:
    TetrisGame(int w, int h, const string &name, Difficulty diff)
        : boardWidth(w), boardHeight(h),
          grid(h, vector<int>(w, 0)),
          colorGrid(h, vector<int>(w, 0)),
          currentPiece(w),
          score(0), level(1), linesCleared(0),
          playerName(name), difficulty(diff)
    {
        srand((unsigned)time(0));
        // Hide the cursor for smoother output
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hOut, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hOut, &cursorInfo);
        
        system("cls");
        printLine(2, "Controls:", MAGENTA);
        printLine(3, "  a - Move Left    d - Move Right");
        printLine(4, "  w - Rotate Clockwise    s - Rotate Counterclockwise");
        printLine(5, "  Spacebar - Hard Drop    c - Hold Piece");
        printLine(6, "  p - Pause    r - Resume");
        printLine(8, "Press any key to start...");
        _getch();
        
        dropSpeed = INITIAL_SPEED[difficulty];
        generateNextPieces();
    }
    
    void processInput() {
        while (_kbhit()) {
            char key = _getch();
            if (paused) {
                if (key == 'r') paused = false;
                continue;
            }
            int newX = currentPiece.x;
            switch (key) {
                case 'a': {
                    newX--;
                    if (isValidMove(newX, currentPiece.y, currentPiece))
                        currentPiece.x = newX;
                } break;
                case 'd': {
                    newX++;
                    if (isValidMove(newX, currentPiece.y, currentPiece))
                        currentPiece.x = newX;
                } break;
                case 'w': {
                    Tetromino temp = currentPiece;
                    temp.rotate();
                    if (isValidMove(temp.x, temp.y, temp))
                        currentPiece.rotate();
                } break;
                case 's': {
                    Tetromino temp = currentPiece;
                    temp.rotateCounter();
                    if (isValidMove(temp.x, temp.y, temp))
                        currentPiece.rotateCounter();
                } break;
                case ' ': {
                    while (isValidMove(currentPiece.x, currentPiece.y + 1, currentPiece))
                        currentPiece.y++;
                } break;
                case 'c': {
                    if (canHold) {
                        if (!holdPiece) {
                            holdPiece = new Tetromino(boardWidth);
                            *holdPiece = currentPiece;
                            currentPiece = nextPieces[0];
                            nextPieces.erase(nextPieces.begin());
                            generateNextPieces();
                        } else {
                            swap(currentPiece, *holdPiece);
                        }
                        canHold = false;
                    }
                } break;
                case 'p':
                    paused = true;
                    break;
                case 'r':
                    paused = false;
                    break;
            }
        }
    }
    
    bool update() {
        if (paused) return true;
        if (isValidMove(currentPiece.x, currentPiece.y + 1, currentPiece)) {
            currentPiece.y++;
        } else {
            mergePiece();
            clearLines();
            if (nextPieces.empty()) {
                currentPiece = Tetromino(boardWidth);
                generateNextPieces();
            } else {
                currentPiece = nextPieces[0];
                nextPieces.erase(nextPieces.begin());
                generateNextPieces();
            }
            if (!isValidMove(currentPiece.x, currentPiece.y, currentPiece))
                return false;
        }
        return true;
    }
    
    int run() {
        DWORD lastDropTime = GetTickCount();
        bool running = true;
        while (running) {
            processInput();
            DWORD currentTime = GetTickCount();
            if (!paused && currentTime - lastDropTime >= (DWORD)dropSpeed) {
                if (!update())
                    running = false;
                lastDropTime = currentTime;
            }
            render();
            Sleep(10);
        }
        return score;
    }
};

void gameOverAnimation() {
    for (int i = 0; i < 6; i++) {
        system("cls");
        if (i % 2 == 0)
            printLine(10, "    *** GAME OVER ***", RED);
        else
            printLine(10, "                     ", RED);
        Sleep(300);
    }
}

int main() {
    // Hide cursor in menus
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    int customWidth = 10, customHeight = 20;
    cout << "Enter board width (min 10 recommended): ";
    cin >> customWidth;
    cout << "Enter board height (min 20 recommended): ";
    cin >> customHeight;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    const string highScoreFile = "tetrisHighScore.txt";
    // Load high scores from file into a map: username -> high score
    map<string, int> highScores = loadHighScores(highScoreFile);
    
    bool playAgain = true;
    while (playAgain) {
        vector<string> mainMenuOptions = {"Start", "Highscore", "Exit"};
        int mainChoice = menu("=== TETRIS ===", mainMenuOptions);
        
        if (mainChoice == 1) {
            // Display high scores for all users
            system("cls");
            printLine(2, "High Scores:", MAGENTA);
            int line = 4;
            for (auto &p : highScores) {
                gotoxy(0, line);
                cout << p.first << ": " << p.second;
                line++;
            }
            printLine(line + 1, "Press any key to return to menu...");
            _getch();
            continue;
        } else if (mainChoice == 2) {
            break;
        }
        
        system("cls");
        printLine(4, "Enter your name: ");
        gotoxy(20, 4);
        string playerName;
        getline(cin, playerName);
        
        // Get this player's high score (if exists; else 0)
        int playerHigh = 0;
        if (highScores.find(playerName) != highScores.end())
            playerHigh = highScores[playerName];
        
        vector<string> modeOptions = {"Easy", "Medium", "Hard"};
        int modeChoice = menu("Select Mode", modeOptions);
        Difficulty chosenMode = static_cast<Difficulty>(modeChoice);
        
        TetrisGame game(customWidth, customHeight, playerName, chosenMode);
        int score = game.run();
        
        if (score > playerHigh) {
            playerHigh = score;
            highScores[playerName] = score;
            saveHighScores(highScoreFile, highScores);
        }
        
        gameOverAnimation();
        
        vector<string> playOptions = {"Play Again", "Exit"};
        int playChoice = menu("Play Again?", playOptions);
        if (playChoice == 1)
            playAgain = false;
    }
    
    system("cls");
    printLine(10, "Thank you for playing Tetris!", GREEN);
    gotoxy(0, 12);
    return 0;
}