#include "raylib.h"
#include <ctime>
#include <vector>

using namespace std;

const int H = 20;
const int W = 15;
const int cellSize = 30;

Color colors[] = {LIGHTGRAY, RED, GREEN, BLUE, YELLOW, PURPLE, ORANGE, PINK};

char board[H][W] = {};
char blocks[][4][4] = {{{' ', 'I', ' ', ' '},
                        {' ', 'I', ' ', ' '},
                        {' ', 'I', ' ', ' '},
                        {' ', 'I', ' ', ' '}},
                       {{' ', ' ', ' ', ' '},
                        {' ', 'O', 'O', ' '},
                        {' ', 'O', 'O', ' '},
                        {' ', ' ', ' ', ' '}},
                       {{' ', ' ', ' ', ' '},
                        {' ', 'T', ' ', ' '},
                        {'T', 'T', 'T', ' '},
                        {' ', ' ', ' ', ' '}},
                       {{' ', ' ', ' ', ' '},
                        {' ', 'S', 'S', ' '},
                        {'S', 'S', ' ', ' '},
                        {' ', ' ', ' ', ' '}},
                       {{' ', ' ', ' ', ' '},
                        {'Z', 'Z', ' ', ' '},
                        {' ', 'Z', 'Z', ' '},
                        {' ', ' ', ' ', ' '}},
                       {{' ', ' ', ' ', ' '},
                        {'J', ' ', ' ', ' '},
                        {'J', 'J', 'J', ' '},
                        {' ', ' ', ' ', ' '}},
                       {{' ', ' ', ' ', ' '},
                        {' ', ' ', 'L', ' '},
                        {'L', 'L', 'L', ' '},
                        {' ', ' ', ' ', ' '}}};

int posX = 4, posY = 0, blockType = 1;
float timer = 0;
float moveSpeed = 0.5f;
bool isClearing = false;
float clearTimer = 0;
int clearingRows[4];
int clearingRowCount = 0;

void initBoard() {
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            if ((i == H - 1) || (j == 0) || (j == W - 1))
                board[i][j] = '#';
            else
                board[i][j] = ' ';
}

bool canMove(int dx, int dy) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (blocks[blockType][i][j] != ' ') {
                int tx = posX + j + dx;
                int ty = posY + i + dy;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1)
                    return false;
                if (board[ty][tx] != ' ')
                    return false;
            }
        }
    }
    return true;
}

void block2Board() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[blockType][i][j] != ' ')
                board[posY + i][posX + j] = blocks[blockType][i][j];
}

int main() {
    InitWindow(W * cellSize, H * cellSize, "Tetris Raylib - MacOS");
    SetTargetFPS(60);
    srand(time(0));

    initBoard();
    blockType = rand() % 7;

    while (!WindowShouldClose()) {
        if (!isClearing) {
            if (IsKeyPressed(KEY_A) && canMove(-1, 0))
                posX--;
            if (IsKeyPressed(KEY_D) && canMove(1, 0))
                posX++;
            if (IsKeyPressed(KEY_X) && canMove(0, 1))
                posY++;
        }

        float dt = GetFrameTime();

        if (isClearing) {
            clearTimer -= dt;
            if (clearTimer <= 0) {
                for (int k = 0; k < clearingRowCount; k++)
                    for (int r = clearingRows[k]; r > 0; r--)
                        for (int j = 1; j < W - 1; j++)
                            board[r][j] = board[r - 1][j];
                isClearing = false;
                posX = 5;
                posY = 0;
                blockType = rand() % 7;
                if (!canMove(0, 0))
                    initBoard();
            }
        } else {
            timer += dt;
            if (timer >= moveSpeed) {
                if (canMove(0, 1))
                    posY++;
                else {
                    block2Board();
                    clearingRowCount = 0;
                    for (int i = H - 2; i > 0; i--) {
                        int filledCount = 0;
                        for (int j = 1; j < W - 1; j++)
                            if (board[i][j] != ' ')
                                filledCount++;
                        if (filledCount == W - 2)
                            clearingRows[clearingRowCount++] = i;
                    }
                    if (clearingRowCount > 0) {
                        isClearing = true;
                        clearTimer = 1.0f;
                    } else {
                        posX = 5;
                        posY = 0;
                        blockType = rand() % 7;
                        if (!canMove(0, 0))
                            initBoard();
                    }
                }
                timer = 0;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        Color flashColors[] = {WHITE, YELLOW, ORANGE, PINK};
        int flashIdx = isClearing ? (int)(clearTimer * 20) % 4 : 0;

        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                if (board[i][j] == '#')
                    DrawRectangle(j * cellSize, i * cellSize, cellSize - 1,
                                  cellSize - 1, DARKGRAY);
                else if (board[i][j] != ' ') {
                    bool isFlashRow = false;
                    for (int k = 0; k < clearingRowCount; k++)
                        if (clearingRows[k] == i) { isFlashRow = true; break; }
                    DrawRectangle(j * cellSize, i * cellSize, cellSize - 1,
                                  cellSize - 1,
                                  isFlashRow && isClearing ? flashColors[flashIdx] : BLUE);
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (blocks[blockType][i][j] != ' ') {
                    DrawRectangle((posX + j) * cellSize, (posY + i) * cellSize,
                                  cellSize - 1, cellSize - 1, RED);
                }
            }
        }

        for (int i = 0; i < H; i++)
            DrawLine(0, i * cellSize, W * cellSize, i * cellSize, DARKGRAY);
        for (int j = 0; j < W; j++)
            DrawLine(j * cellSize, 0, j * cellSize, H * cellSize, DARKGRAY);

        if (isClearing)
            DrawRectangle(0, 0, W * cellSize, H * cellSize, Fade(WHITE, 0.15f));

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
