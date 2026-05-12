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

void removeLine() {
    for (int i = H - 2; i > 0; i--) {
        int filledCount = 0;
        for (int j = 1; j < W - 1; j++)
            if (board[i][j] != ' ')
                filledCount++;

        if (filledCount == W - 2) {
            for (int ii = i; ii > 0; ii--)
                for (int jj = 1; jj < W - 1; jj++)
                    board[ii][jj] = board[ii - 1][jj];
            i++;
        }
    }
}

int main() {
    InitWindow(W * cellSize, H * cellSize, "Tetris Raylib - MacOS");
    SetTargetFPS(60);
    srand(time(0));

    initBoard();
    blockType = rand() % 7;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_A) && canMove(-1, 0))
            posX--;
        if (IsKeyPressed(KEY_D) && canMove(1, 0))
            posX++;
        if (IsKeyPressed(KEY_X) && canMove(0, 1))
            posY++;

        timer += GetFrameTime();
        if (timer >= moveSpeed) {
            if (canMove(0, 1))
                posY++;
            else {
                block2Board();
                removeLine();
                posX = 5;
                posY = 0;
                blockType = rand() % 7;
                if (!canMove(0, 0))
                    initBoard();
            }
            timer = 0;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                if (board[i][j] == '#')
                    DrawRectangle(j * cellSize, i * cellSize, cellSize - 1,
                                  cellSize - 1, DARKGRAY);
                else if (board[i][j] != ' ')
                    DrawRectangle(j * cellSize, i * cellSize, cellSize - 1,
                                  cellSize - 1, BLUE);
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

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
