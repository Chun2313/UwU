#include "raylib.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>

using namespace std;

const int H = 20;
const int W = 15;
const int cellSize = 30;
const int screenWidth = 650;
const int screenHeight = 600;
const int MAX_PARTICLES = 600;

Color palette[] = {
    {0x00, 0xF0, 0xF0, 0xFF}, {0x00, 0x00, 0xF0, 0xFF},
    {0xF0, 0xA0, 0x00, 0xFF}, {0xF0, 0xF0, 0x00, 0xFF},
    {0x00, 0xF0, 0x00, 0xFF}, {0xA0, 0x00, 0xF0, 0xFF},
    {0xF0, 0x00, 0x00, 0xFF},
};
Color blockColors[7];

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

int gameState = 0;
const int MAX_ENTRIES = 10;
char playerNames[MAX_ENTRIES][32];
int playerScores[MAX_ENTRIES];
int entryCount = 0;
char currentName[32] = "";
int nameCharCount = 0;
int pendingScore = 0;

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

bool isDropping = false;
float dropStartY = 0, dropEndY = 0, dropAnimTimer = 0;

int scorePopupValue = 0;
float scorePopupTimer = 0;
float acePopupTimer = 0;

float pX[MAX_PARTICLES], pY[MAX_PARTICLES];
float pVX[MAX_PARTICLES], pVY[MAX_PARTICLES];
float pLife[MAX_PARTICLES];
Color pColor[MAX_PARTICLES];
int particleCount = 0;

void spawnParticles() {
    particleCount = 0;
    for (int k = 0; k < clearingRowCount && particleCount < MAX_PARTICLES;
         k++) {
        int row = clearingRows[k];
        for (int j = 1; j < W - 1 && particleCount < MAX_PARTICLES; j++) {
            int n = 4 + rand() % 4;
            for (int p = 0; p < n && particleCount < MAX_PARTICLES; p++) {
                pX[particleCount] = (j + 0.5f) * cellSize + (rand() % 20 - 10);
                pY[particleCount] =
                    (row + 0.5f) * cellSize + (rand() % 20 - 10);
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
            DrawCircle((int)pX[i], (int)pY[i], size * 0.4f,
                       Fade(WHITE, c.a / 255.0f * 0.6f));
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
                board[posY + i][posX + j] = '0' + blockType;
}

void initColors() {
    for (int i = 0; i < 7; i++)
        blockColors[i] = palette[i];
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
    InitAudioDevice();
    Sound milestoneSounds[5];
    milestoneSounds[0] = LoadSound("audio/500diem_A01_R.wav");
    milestoneSounds[1] = LoadSound("audio/100diem_A01_R.wav");
    milestoneSounds[2] = LoadSound("audio/200diem_A01_R.wav");
    milestoneSounds[3] = LoadSound("audio/300diem_A01_R.wav");
    milestoneSounds[4] = LoadSound("audio/400diem_A01_R.wav");
    Music menuMusic = LoadMusicStream("audio/anhdochauphi.mp3");
    PlayMusicStream(menuMusic);
    Music gameMusic = LoadMusicStream("audio/tidalwave.mp3");
    SetMusicVolume(gameMusic, 0.07f);
    for (int i = 0; i < 5; i++)
        SetSoundVolume(milestoneSounds[i], 3.0f);
    SetTargetFPS(60);
    srand(time(0) + clock());

    initColors();
    initBoard();
    for (int i = 0; i < 4; i++)
        rand();
    blockType = rand() % 7;
    nextBlockType = rand() % 7;
    spawnBlock();

    while (!WindowShouldClose()) {
        UpdateMusicStream(menuMusic);
        if (gameState == 2)
            UpdateMusicStream(gameMusic);
        if (gameState == 0) {
            Vector2 mp = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                float bx = screenWidth / 2.0f - 130;
                if (mp.x >= bx && mp.x <= bx + 260) {
                    for (int i = 0; i < 3; i++) {
                        float by = 180.0f + i * 90.0f;
                        if (mp.y >= by && mp.y <= by + 60) {
                            if (i == 0) {
                                gameState = 1;
                                StopMusicStream(menuMusic);
                                nameCharCount = 0;
                                currentName[0] = '\0';
                            }
                            if (i == 2)
                                gameState = 4;
                        }
                    }
                }
            }
        }

        if (gameState == 1 || gameState == 3) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 125 && nameCharCount < 30) {
                    currentName[nameCharCount] = (char)key;
                    nameCharCount++;
                    currentName[nameCharCount] = '\0';
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && nameCharCount > 0) {
                nameCharCount--;
                currentName[nameCharCount] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER) && nameCharCount > 0) {
                if (gameState == 1) {
                    gameState = 2;
                    PlayMusicStream(gameMusic);
                } else if (gameState == 3) {
                    if (entryCount < MAX_ENTRIES) {
                        strcpy(playerNames[entryCount], currentName);
                        playerScores[entryCount] = pendingScore;
                        entryCount++;
                        for (int i = 0; i < entryCount - 1; i++) {
                            for (int j = 0; j < entryCount - 1 - i; j++) {
                                if (playerScores[j] > playerScores[j + 1]) {
                                    int ts = playerScores[j];
                                    playerScores[j] = playerScores[j + 1];
                                    playerScores[j + 1] = ts;
                                    char tn[32];
                                    strcpy(tn, playerNames[j]);
                                    strcpy(playerNames[j], playerNames[j + 1]);
                                    strcpy(playerNames[j + 1], tn);
                                }
                            }
                        }
                    }
                    gameState = 4;
                }
            }
            if (IsKeyPressed(KEY_Q)) {
                gameState = 0;
                PlayMusicStream(menuMusic);
            }
        }

        if (gameState == 4 && IsKeyPressed(KEY_Q)) {
            gameState = 0;
            PlayMusicStream(menuMusic);
        }

        if (gameState == 2) {
            if (IsKeyPressed(KEY_Q)) { gameState = 0; initBoard(); score = 0; gameTimer = 0; PlayMusicStream(menuMusic); StopMusicStream(gameMusic); }
            if (!isClearing && !isDropping) {
                if (IsKeyPressed(KEY_A) && canMove(-1, 0))
                    posX--;
                if (IsKeyPressed(KEY_D) && canMove(1, 0))
                    posX++;
                if (IsKeyPressed(KEY_R))
                    rotateBlock();
                if (IsKeyPressed(KEY_X) && canMove(0, 1)) {
                    int target = posY;
                    while (canMove(0, target - posY + 1))
                        target++;
                    if (target > posY) {
                        isDropping = true;
                        dropStartY = posY;
                        dropEndY = target;
                        dropAnimTimer = 0.19f;
                    }
                }
            }

            float dt = GetFrameTime();
            gameTimer += dt;
            if (scorePopupTimer > 0)
                scorePopupTimer -= dt;
            if (acePopupTimer > 0)
                acePopupTimer -= dt;

            if (isDropping) {
                dropAnimTimer -= dt;
                float t = 1.0f - dropAnimTimer / 0.19f;
                t = t < 0 ? 0 : (t > 1 ? 1 : t);
                posY = (int)(dropStartY + (dropEndY - dropStartY) * t);
                if (dropAnimTimer <= 0) {
                    posY = (int)dropEndY;
                    isDropping = false;
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
                        int ps = score;
                        int ns[] = {0, 100, 200, 300, 400};
                        for (int m = 0; m < 5; m++) {
                            int t =
                                ((ps + clearingRowCount * 100) / 500) * 500 +
                                ns[m];
                            if (t > 0 && ps < t &&
                                ps + clearingRowCount * 100 >= t)
                                PlaySound(milestoneSounds[m]);
                        }
                        isClearing = true;
                        clearTimer = 1.0f;
                        spawnParticles();
                    } else {
                        posX = 5;
                        posY = 0;
                        blockType = nextBlockType;
                        nextBlockType = rand() % 7;
                        spawnBlock();
                        if (!canMove(0, 0)) {
                            pendingScore = score;
                            gameState = 3;
                            nameCharCount = 0;
                            currentName[0] = '\0';
                        }
                    }
                    dropTimer = 0;
                }
            }

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
                    if (score > 0 &&
                        score / 500 > (score - clearingRowCount * 100) / 500)
                        acePopupTimer = 2.0f;
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
                            int ps = score;
                            int ns[] = {0, 100, 200, 300, 400};
                            for (int m = 0; m < 5; m++) {
                                int t = ((ps + clearingRowCount * 100) / 500) *
                                            500 +
                                        ns[m];
                                if (t > 0 && ps < t &&
                                    ps + clearingRowCount * 100 >= t)
                                    PlaySound(milestoneSounds[m]);
                            }
                            isClearing = true;
                            clearTimer = 1.0f;
                            spawnParticles();
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
        }

        float shakeX = 0, shakeY = 0;
        if (isClearing) {
            float intensity = clearTimer * 4;
            shakeX = (rand() % 200 - 100) * intensity / 100.0f;
            shakeY = (rand() % 200 - 100) * intensity / 100.0f;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (gameState == 0) {
            float bx = screenWidth / 2.0f - 130;
            int tw = MeasureText("TETRIS", 70);
            DrawText("TETRIS", (screenWidth - tw) / 2, 70, 70, RAYWHITE);
            Color titleShadow = Fade(RAYWHITE, 0.15f);
            DrawText("TETRIS", (screenWidth - tw) / 2 + 3, 73, 70, titleShadow);
            const char *items[] = {"Single Player", "2-Players", "Leaderboard"};
            for (int i = 0; i < 3; i++) {
                float by = 180.0f + i * 90.0f;
                Rectangle rec = {bx, by, 260, 60};
                bool hover = CheckCollisionPointRec(GetMousePosition(), rec);
                Color bg, border;
                if (hover) {
                    bg = (Color){0x4A, 0x6F, 0xE0, 0xFF};
                    border = (Color){0x6C, 0x8E, 0xF0, 0xFF};
                } else {
                    bg = (Color){0x2C, 0x2C, 0x3E, 0xCC};
                    border = (Color){0x4A, 0x4A, 0x5E, 0xAA};
                }
                DrawRectangleRounded(rec, 0.3f, 6, bg);
                DrawRectangleRoundedLines(rec, 0.3f, 6, border);
                int tw2 = MeasureText(items[i], 22);
                Color textColor =
                    hover ? RAYWHITE : (Color){0xCC, 0xCC, 0xDD, 0xFF};
                DrawText(items[i], (int)(bx + (260 - tw2) / 2), (int)by + 17,
                         22, textColor);
            }
            EndDrawing();
            continue;
        }

        if (gameState == 4) {
            DrawText("LEADERBOARD", (screenWidth - MeasureText("LEADERBOARD", 50)) / 2, 50, 50, RAYWHITE);
            int startY = 130;
            for (int i = 0; i < entryCount && i < 10; i++) {
                char line[64];
                snprintf(line, sizeof(line), "%d. %s - %d", i + 1, playerNames[i], playerScores[i]);
                Color c = (i == 0) ? GOLD : ((i == 1) ? LIGHTGRAY : ((i == 2) ? ORANGE : GRAY));
                int lw = MeasureText(line, 24);
                DrawText(line, (screenWidth - lw) / 2, startY + i * 40, 24, c);
            }
            if (entryCount == 0) {
                int lw = MeasureText("No scores yet!", 24);
                DrawText("No scores yet!", (screenWidth - lw) / 2, startY, 24, GRAY);
            }
            const char* hint2 = "Press Q to return to menu";
            int hw2 = MeasureText(hint2, 16);
            DrawText(hint2, (screenWidth - hw2) / 2, startY + 10 * 40 + 20, 16, GRAY);
            EndDrawing();
            continue;
        }

        if (gameState == 3) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
            const char* prompt = "Game Over! Enter your name:";
            int pw = MeasureText(prompt, 30);
            DrawText(prompt, (screenWidth - pw) / 2, 200, 30, RAYWHITE);
            Rectangle inputBox = {(float)(screenWidth / 2 - 150), 260, 300, 50};
            DrawRectangleRounded(inputBox, 0.2f, 4, (Color){0x4A, 0x6F, 0xE0, 0xAA});
            DrawRectangleRoundedLines(inputBox, 0.2f, 4, (Color){0x6C, 0x8E, 0xF0, 0xFF});
            int cw = MeasureText(currentName, 28);
            DrawText(currentName, screenWidth / 2 - cw / 2, 270, 28, RAYWHITE);
            if ((int)(GetTime() * 2) % 2 == 0) {
                int cursorX = screenWidth / 2 + cw / 2 + 2;
                DrawText("|", cursorX, 270, 28, RAYWHITE);
            }
            const char* hint = "Press ENTER to confirm, Q to go back";
            int hw = MeasureText(hint, 16);
            DrawText(hint, (screenWidth - hw) / 2, 330, 16, GRAY);
            EndDrawing();
            continue;
        }

        if (gameState == 1) {
            const char* prompt = "Enter your name:";
            int pw = MeasureText(prompt, 30);
            DrawText(prompt, (screenWidth - pw) / 2, 200, 30, RAYWHITE);
            Rectangle inputBox = {(float)(screenWidth / 2 - 150), 260, 300, 50};
            DrawRectangleRounded(inputBox, 0.2f, 4, (Color){0x4A, 0x6F, 0xE0, 0xAA});
            DrawRectangleRoundedLines(inputBox, 0.2f, 4, (Color){0x6C, 0x8E, 0xF0, 0xFF});
            int cw = MeasureText(currentName, 28);
            DrawText(currentName, screenWidth / 2 - cw / 2, 270, 28, RAYWHITE);
            if ((int)(GetTime() * 2) % 2 == 0) {
                int cursorX = screenWidth / 2 + cw / 2 + 2;
                DrawText("|", cursorX, 270, 28, RAYWHITE);
            }
            const char* hint = "Press ENTER to confirm, Q to go back";
            int hw = MeasureText(hint, 16);
            DrawText(hint, (screenWidth - hw) / 2, 330, 16, GRAY);
            EndDrawing();
            continue;
        }

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
                        if (clearingRows[k] == i) {
                            isFlashRow = true;
                            break;
                        }
                    Color blockColor;
                    if (isFlashRow && isClearing)
                        blockColor = ColorFromHSV(hue + i * 30, 1, 1);
                    else if (board[i][j] >= '0' && board[i][j] <= '6')
                        blockColor = blockColors[board[i][j] - '0'];
                    else
                        blockColor = BLUE;
                    DrawRectangleRounded((Rectangle){(float)rx, (float)ry,
                                                     cellSize - 1,
                                                     cellSize - 1},
                                         0.25f, 4, blockColor);
                    DrawRectangleRounded(
                        (Rectangle){(float)rx + 2, (float)ry + 2, cellSize - 5,
                                    cellSize - 5},
                        0.2f, 4, Fade(WHITE, 0.12f));
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
                    glow.a =
                        (unsigned char)((60 - g * 15) * (1.0f - clearTimer));
                    DrawRectangle((int)(1 * cellSize - s + shakeX),
                                  (int)(row * cellSize - s + shakeY),
                                  (int)((W - 2) * cellSize + s * 2),
                                  (int)(cellSize + s * 2), glow);
                }
            }
        }

        if (isDropping) {
            float progress = 1.0f - dropAnimTimer / 0.19f;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (blocks[blockType][i][j] != ' ') {
                        float gx = (float)((posX + j) * cellSize + (int)shakeX);
                        float gy = (float)(((int)dropEndY + i) * cellSize +
                                           (int)shakeY);
                        Color gc = blockColors[blockType];
                        gc.a = (unsigned char)(80 * (1.0f - progress));
                        DrawRectangleRounded(
                            (Rectangle){gx, gy, cellSize - 1, cellSize - 1},
                            0.25f, 4, gc);
                    }
                }
            }
            for (int trail = 1; trail < (int)(dropEndY - dropStartY); trail++) {
                float trailAlpha = (1.0f - progress) *
                                   (1.0f - trail / (dropEndY - dropStartY)) *
                                   100;
                if (trailAlpha < 5)
                    continue;
                int trailY = (int)dropStartY + trail - 1;
                if (trailY >= posY)
                    continue;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (blocks[blockType][i][j] != ' ') {
                            Color tc = blockColors[blockType];
                            tc.a = (unsigned char)trailAlpha;
                            DrawRectangleRounded(
                                (Rectangle){(float)((posX + j) * cellSize +
                                                    (int)shakeX),
                                            (float)((trailY + i) * cellSize +
                                                    (int)shakeY),
                                            cellSize - 1, cellSize - 1},
                                0.25f, 4, tc);
                        }
                    }
                }
            }
        }

        if (!isDropping) {
            int ghostY = posY;
            while (canMove(0, ghostY - posY + 1))
                ghostY++;
            if (ghostY > posY) {
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (currentBlock[i][j] != ' ') {
                            float gx =
                                (float)((posX + j) * cellSize + (int)shakeX);
                            float gy =
                                (float)((ghostY + i) * cellSize + (int)shakeY);
                            Color gc = blockColors[blockType];
                            gc.a = 60;
                            DrawRectangleRounded(
                                (Rectangle){gx, gy, cellSize - 1, cellSize - 1},
                                0.25f, 4, gc);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentBlock[i][j] != ' ') {
                    float bx = (float)((posX + j) * cellSize + (int)shakeX);
                    float by = (float)((posY + i) * cellSize + (int)shakeY);
                    Color c = blockColors[blockType];
                    DrawRectangleRounded(
                        (Rectangle){bx, by, cellSize - 1, cellSize - 1}, 0.25f,
                        4, c);
                    DrawRectangleRounded(
                        (Rectangle){bx + 2, by + 2, cellSize - 5, cellSize - 5},
                        0.2f, 4, Fade(WHITE, 0.15f));
                }
            }
        }

        for (int i = 0; i <= H; i++)
            DrawLine(0, i * cellSize + (int)shakeY, W * cellSize,
                     i * cellSize + (int)shakeY, Fade(DARKGRAY, 0.5f));
        for (int j = 0; j <= W; j++)
            DrawLine(j * cellSize + (int)shakeX, 0, j * cellSize + (int)shakeX,
                     H * cellSize, Fade(DARKGRAY, 0.5f));

        drawParticles();

        if (isClearing) {
            float progress = 1.0f - clearTimer;
            float alpha = progress < 0.2f ? progress / 0.2f : 1.0f;
            if (progress > 0.7f)
                alpha = 1.0f - (progress - 0.7f) / 0.3f;
            int fontSize = 40 + (int)(progress * 60);
            const char *label = TextFormat("+%d", clearingRowCount * 100);
            int tw = MeasureText(label, fontSize);
            int cx = (W * cellSize - tw) / 2;
            int cy = (H * cellSize - fontSize) / 2;
            Color tc = ColorFromHSV(hue, 1, 1);
            tc.a = (unsigned char)(alpha * 255);
            DrawText(label, cx + (int)shakeX, cy + (int)shakeY - 30, fontSize,
                     tc);

            if (clearingRowCount >= 3) {
                Color flash = Fade(WHITE, 0.3f * alpha);
                DrawRectangle(0, 0, W * cellSize, H * cellSize, flash);
            }
        }

        if (acePopupTimer > 0) {
            float progress = 1.0f - acePopupTimer / 2.0f;
            float alpha = progress < 0.15f ? progress / 0.15f : 1.0f;
            if (progress > 0.8f)
                alpha = 1.0f - (progress - 0.8f) / 0.2f;
            int fontSize = 50 + (int)(progress * 80);
            const char *aceText = "ACE";
            int tw = MeasureText(aceText, fontSize);
            int cx = (W * cellSize - tw) / 2;
            int cy = (H * cellSize - fontSize) / 2;
            Color ac = ColorFromHSV(progress * 720, 1, 1);
            ac.a = (unsigned char)(alpha * 255);
            DrawText(aceText, cx + (int)shakeX, cy + (int)shakeY - 30, fontSize,
                     ac);
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
            DrawText(TextFormat("+%d", scorePopupValue), sidebarX,
                     125 - offsetY, 25, c);
        }

        DrawText("TIME", sidebarX, 200, 20, LIGHTGRAY);
        DrawText(
            TextFormat("%02d:%02d", (int)gameTimer / 60, (int)gameTimer % 60),
            sidebarX, 225, 30, GREEN);

        DrawText("NEXT", sidebarX, 300, 20, LIGHTGRAY);
        DrawRectangleRounded((Rectangle){(float)sidebarX, 330, 120, 120}, 0.15f,
                             4, Fade(DARKGRAY, 0.3f));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (blocks[nextBlockType][i][j] != ' ') {
                    float nb = (float)(sidebarX + j * 25 + 10);
                    float nby = (float)(330 + i * 25 + 10);
                    Color c = blockColors[nextBlockType];
                    DrawRectangleRounded((Rectangle){nb, nby, 23, 23}, 0.25f, 4,
                                         c);
                    DrawRectangleRounded(
                        (Rectangle){nb + 1.5f, nby + 1.5f, 20, 20}, 0.2f, 4,
                        Fade(WHITE, 0.12f));
                }
            }
        }

        DrawText("A/D: Move", sidebarX, 480, 15, GRAY);
        DrawText("X: Drop", sidebarX, 500, 15, GRAY);
        DrawText("R: Rotate", sidebarX, 520, 15, GRAY);

        EndDrawing();
    }

    for (int i = 0; i < 5; i++)
        UnloadSound(milestoneSounds[i]);
    UnloadMusicStream(menuMusic);
    UnloadMusicStream(gameMusic);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
