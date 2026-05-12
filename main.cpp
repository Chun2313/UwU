#include "raylib.h"
#include <ctime>
#include <cstdlib>
#include <cmath>

using namespace std;

const int H = 20;
const int W = 15;
const int cellSize = 30;
const int screenWidth = 600;
const int screenHeight = 600;
const int MAX_PARTICLES = 600;

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

int scorePopupValue = 0;
float scorePopupTimer = 0;

float pX[MAX_PARTICLES], pY[MAX_PARTICLES];
float pVX[MAX_PARTICLES], pVY[MAX_PARTICLES];
float pLife[MAX_PARTICLES];
Color pColor[MAX_PARTICLES];
int particleCount = 0;

void spawnParticles() {
    particleCount = 0;
    for (int k = 0; k < clearingRowCount && particleCount < MAX_PARTICLES; k++) {
        int row = clearingRows[k];
        for (int j = 1; j < W - 1 && particleCount < MAX_PARTICLES; j++) {
            int n = 4 + rand() % 4;
            for (int p = 0; p < n && particleCount < MAX_PARTICLES; p++) {
                pX[particleCount] = (j + 0.5f) * cellSize + (rand() % 20 - 10);
                pY[particleCount] = (row + 0.5f) * cellSize + (rand() % 20 - 10);
                pVX[particleCount] = (rand() % 400 - 200) * 1.2f;
                pVY[particleCount] = -(100 + rand() % 400) * 1.2f;
                pLife[particleCount] = 0.5f + (rand() % 100) * 0.003f;
                pColor[particleCount] = ColorFromHSV(rand() % 360, 0.9f, 1);
                particleCount++;
            }
        }
    }
}

void updateParticles(float dt) {
    for (int i = 0; i < particleCount; i++) {
        pX[i] += pVX[i] * dt;
        pY[i] += pVY[i] * dt;
        pVY[i] += 400 * dt;
        pLife[i] -= dt;
    }
}

void drawParticles() {
    for (int i = 0; i < particleCount; i++) {
        if (pLife[i] > 0) {
            Color c = pColor[i];
            c.a = (unsigned char)((pLife[i] / 0.8f) * 255);
            float size = pLife[i] * 5 + 1;
            DrawCircle((int)pX[i], (int)pY[i], size, c);
            DrawCircle((int)pX[i], (int)pY[i], size * 0.4f, Fade(WHITE, c.a / 255.0f * 0.6f));
        }
    }
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

void rotateBlock() {
    char rotated[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            rotated[j][3 - i] = blocks[blockType][i][j];

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
            blocks[blockType][i][j] = rotated[i][j];
}


int main() {
    InitWindow(screenWidth, screenHeight, "Tetris Raylib - Square GUI");
    SetTargetFPS(60);
    srand(time(0));

    initBoard();
    blockType = rand() % 7;
    nextBlockType = rand() % 7;

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
        if (scorePopupTimer > 0) scorePopupTimer -= dt;

        if (isClearing) {
            clearTimer -= dt;
            updateParticles(dt);
            if (clearTimer <= 0) {
                for (int k = 0; k < clearingRowCount; k++)
                    for (int r = clearingRows[k]; r > 0; r--)
                        for (int j = 1; j < W - 1; j++)
                            board[r][j] = board[r - 1][j];
                score += clearingRowCount * 100;
                scorePopupValue = clearingRowCount * 100;
                scorePopupTimer = 1.5f;
                isClearing = false;
                posX = 5;
                posY = 0;
                blockType = nextBlockType;
                nextBlockType = rand() % 7;
                if (!canMove(0, 0)) {
                    initBoard();
                    score = 0;
                    gameTimer = 0;
                }
            }
        } else {
            dropTimer += dt;
            if (dropTimer >= moveSpeed) {
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
                        spawnParticles();
                    } else {
                        posX = 5;
                        posY = 0;
                        blockType = nextBlockType;
                        nextBlockType = rand() % 7;
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

        float shakeX = 0, shakeY = 0;
        if (isClearing) {
            float intensity = clearTimer * 4;
            shakeX = (rand() % 200 - 100) * intensity / 100.0f;
            shakeY = (rand() % 200 - 100) * intensity / 100.0f;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        float hue = fmod(clearTimer * 480, 360);

        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                int rx = j * cellSize + (int)shakeX;
                int ry = i * cellSize + (int)shakeY;
                if (board[i][j] == '#')
                    DrawRectangle(rx, ry, cellSize - 1, cellSize - 1, DARKGRAY);
                else if (board[i][j] != ' ') {
                    bool isFlashRow = false;
                    for (int k = 0; k < clearingRowCount; k++)
                        if (clearingRows[k] == i) { isFlashRow = true; break; }
                    if (isFlashRow && isClearing) {
                        Color c = ColorFromHSV(hue + i * 30, 1, 1);
                        DrawRectangle(rx, ry, cellSize - 1, cellSize - 1, c);
                    } else {
                        DrawRectangle(rx, ry, cellSize - 1, cellSize - 1, BLUE);
                    }
                }
            }
        }

        if (isClearing) {
            for (int k = 0; k < clearingRowCount; k++) {
                int row = clearingRows[k];
                float glowSize = (1.0f - clearTimer) * 20;
                for (int g = 0; g < 3; g++) {
                    float s = glowSize + g * 4;
                    Color glow = ColorFromHSV(hue + row * 30, 1, 1);
                    glow.a = (unsigned char)((60 - g * 15) * (1.0f - clearTimer));
                    DrawRectangle(
                        (int)(1 * cellSize - s + shakeX),
                        (int)(row * cellSize - s + shakeY),
                        (int)((W - 2) * cellSize + s * 2),
                        (int)(cellSize + s * 2),
                        glow
                    );
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (blocks[blockType][i][j] != ' ') {
                    DrawRectangle((posX + j) * cellSize + (int)shakeX,
                                  (posY + i) * cellSize + (int)shakeY,
                                  cellSize - 1, cellSize - 1, RED);
                }
            }
        }

        for (int i = 0; i <= H; i++)
            DrawLine(0, i * cellSize + (int)shakeY,
                     W * cellSize, i * cellSize + (int)shakeY, Fade(DARKGRAY, 0.5f));
        for (int j = 0; j <= W; j++)
            DrawLine(j * cellSize + (int)shakeX, 0,
                     j * cellSize + (int)shakeX, H * cellSize, Fade(DARKGRAY, 0.5f));

        drawParticles();

        if (isClearing) {
            float progress = 1.0f - clearTimer;
            float alpha = progress < 0.2f ? progress / 0.2f : 1.0f;
            if (progress > 0.7f) alpha = 1.0f - (progress - 0.7f) / 0.3f;
            int fontSize = 40 + (int)(progress * 60);
            const char* label = TextFormat("+%d", clearingRowCount * 100);
            int tw = MeasureText(label, fontSize);
            int cx = (W * cellSize - tw) / 2;
            int cy = (H * cellSize - fontSize) / 2;
            Color tc = ColorFromHSV(hue, 1, 1);
            tc.a = (unsigned char)(alpha * 255);
            DrawText(label, cx + (int)shakeX, cy + (int)shakeY - 30, fontSize, tc);

            if (clearingRowCount >= 3) {
                Color flash = Fade(WHITE, 0.3f * alpha);
                DrawRectangle(0, 0, W * cellSize, H * cellSize, flash);
            }
        }

        int sidebarX = W * cellSize + 20;
        DrawText("TETRIS", sidebarX, 20, 40, RAYWHITE);

        DrawText("SCORE", sidebarX, 100, 20, LIGHTGRAY);
        DrawText(TextFormat("%06d", score), sidebarX, 125, 30, YELLOW);

        if (scorePopupTimer > 0) {
            float a = scorePopupTimer / 1.5f;
            int offsetY = (int)((1.0f - a) * 40);
            Color c = YELLOW;
            c.a = (unsigned char)(a * 255);
            DrawText(TextFormat("+%d", scorePopupValue), sidebarX, 125 - offsetY, 25, c);
        }

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

        DrawText("A/D: Move", sidebarX, 480, 15, GRAY);
        DrawText("X: Drop", sidebarX, 500, 15, GRAY);
        DrawText("R: Rotate", sidebarX, 520, 15, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
