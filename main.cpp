#include "raylib.h"
#include <ctime>
#include <vector>
#include <string>

using namespace std;

const int H = 20;
const int W = 15;
const int cellSize = 30;
const int screenWidth = 600;
const int screenHeight = 600;

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

int posX = 4, posY = 0, blockType = 1, nextBlockType = 0;
int score = 0;
float gameTimer = 0;
float dropTimer = 0;
float moveSpeed = 0.5f;
bool isClearing = false;
float clearTimer = 0;
int clearingRows[4];
int clearingRowCount = 0;
char currentBlock[4][4];

void spawnBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            currentBlock[i][j] = blocks[blockType][i][j];
}

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
            if (currentBlock[i][j] != ' ') {
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
            if (currentBlock[i][j] != ' ')
                board[posY + i][posX + j] = currentBlock[i][j];
}

void rotateBlock() {
    char rotated[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            rotated[j][3 - i] = currentBlock[i][j];

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (rotated[i][j] != ' ') {
                int tx = posX + j;
                int ty = posY + i;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1)
                    return;
                if (board[ty][tx] != ' ')
                    return;
            }

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            currentBlock[i][j] = rotated[i][j];
}


int main() {
    InitWindow(screenWidth, screenHeight, "Tetris Raylib - Square GUI");
    SetTargetFPS(60);
    srand(time(0) + clock());

    initBoard();
    for (int i = 0; i < 4; i++) rand();
    blockType = rand() % 7;
    nextBlockType = rand() % 7;
    spawnBlock();

    while (!WindowShouldClose()) {
        if (!isClearing) {
            if (IsKeyPressed(KEY_A) && canMove(-1, 0))
                posX--;
            if (IsKeyPressed(KEY_D) && canMove(1, 0))
                posX++;
            if (IsKeyPressed(KEY_X) && canMove(0, 1))
                posY++;
            if (IsKeyPressed(KEY_R))
                rotateBlock();
        }

        float dt = GetFrameTime();
        gameTimer += dt;

        if (isClearing) {
            clearTimer -= dt;
            if (clearTimer <= 0) {
                for (int k = 0; k < clearingRowCount; k++)
                    for (int r = clearingRows[k]; r > 0; r--)
                        for (int j = 1; j < W - 1; j++)
                            board[r][j] = board[r - 1][j];
                score += clearingRowCount * 100;
                isClearing = false;
                posX = 5;
                posY = 0;
                blockType = nextBlockType;
                nextBlockType = rand() % 7;
                spawnBlock();
                if (!canMove(0, 0)) {
                    initBoard();
                    score = 0;
                    gameTimer = 0;
                }
            }
        } else {
            float speed = IsKeyDown(KEY_X) ? moveSpeed / 4 : moveSpeed;
            dropTimer += dt;
            if (dropTimer >= speed) {
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
                        blockType = nextBlockType;
                        nextBlockType = rand() % 7;
                        spawnBlock();
                        if (!canMove(0, 0)) {
                            initBoard();
                            score = 0;
                            gameTimer = 0;
                        }
                    }
                }
                dropTimer = 0;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                if (board[i][j] == '#')
                    DrawRectangle(j * cellSize, i * cellSize, cellSize - 1,
                                  cellSize - 1, DARKGRAY);
                else if (board[i][j] != ' ') {
                    bool isHighlightRow = false;
                    for (int k = 0; k < clearingRowCount; k++)
                        if (clearingRows[k] == i) { isHighlightRow = true; break; }
                    DrawRectangle(j * cellSize, i * cellSize, cellSize - 1,
                                  cellSize - 1, isHighlightRow && isClearing ? WHITE : BLUE);
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentBlock[i][j] != ' ') {
                    DrawRectangle((posX + j) * cellSize, (posY + i) * cellSize,
                                  cellSize - 1, cellSize - 1, RED);
                }
            }
        }

        for (int i = 0; i <= H; i++)
            DrawLine(0, i * cellSize, W * cellSize, i * cellSize, Fade(DARKGRAY, 0.5f));
        for (int j = 0; j <= W; j++)
            DrawLine(j * cellSize, 0, j * cellSize, H * cellSize, Fade(DARKGRAY, 0.5f));

        int sidebarX = W * cellSize + 20;
        DrawText("TETRIS", sidebarX, 20, 40, RAYWHITE);

        DrawText("SCORE", sidebarX, 100, 20, LIGHTGRAY);
        DrawText(TextFormat("%06d", score), sidebarX, 125, 30, YELLOW);

        DrawText("TIME", sidebarX, 200, 20, LIGHTGRAY);
        DrawText(TextFormat("%02d:%02d", (int)gameTimer / 60, (int)gameTimer % 60), sidebarX, 225, 30, GREEN);

        DrawText("NEXT", sidebarX, 300, 20, LIGHTGRAY);
        DrawRectangle(sidebarX, 330, 120, 120, Fade(DARKGRAY, 0.3f));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (blocks[nextBlockType][i][j] != ' ') {
                    DrawRectangle(sidebarX + j * 25 + 10, 330 + i * 25 + 10, 23, 23, RED);
                }
            }
        }

        DrawText("A/D: Move", sidebarX, 500, 15, GRAY);
        DrawText("X: Drop", sidebarX, 520, 15, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
