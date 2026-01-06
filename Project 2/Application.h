#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <HAL/HAL.h>
#include <HAL/Graphics.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    STATE_TITLE,
    STATE_MENU,
    STATE_INSTRUCTIONS,
    STATE_HIGHSCORE,
    STATE_GAME,
    STATE_OPTIONS,
    STATE_GAMEOVER
} AppState;

typedef enum {
    JUMP_NONE,
    JUMP_UP,
    JUMP_DOWN
} JumpState;

typedef enum {
    CURSOR_0 = 0,
    CURSOR_1 = 1,
    CURSOR_2 = 2,
    CURSOR_3 = 3
} Cursor;

typedef struct {
    int center;
    int up;
    int down;
    int left;
    int right;
} ColorWheel;

typedef struct {
    int x;
    int width;
    int color;
} FloorSegment;

typedef struct {
    AppState state;
    bool titleScreenShown;
    bool screenNeedsRedraw;
    Cursor arrow;
    bool joystickCentered;

    SWTimer titleTimer;
    SWTimer jumpTimer;
    SWTimer scoreTimer;
    SWTimer floorTimer;
    SWTimer fallTimer;

    int playerY;
    JumpState jumpState;
    int jumpProgress;
    bool isOnGround;
    int lastPlayerY;

    ColorWheel colorWheel;

    int score;
    int *highScores;
    int jumpHeight;
    int playerRadius;
    int playerCenterX;
    int playerJumpDelay;
    int maxHighScores;

    bool isFalling;

    FloorSegment floorSegments[5];
    int numFloorSegments;

    int difficulty;
} Application;

Application Application_construct();
void Application_loop(Application* app, HAL* hal);

void App_Screen_showmainmenu(Application* app, GFX* gfx);
void App_Screen_handlemainmenu(Application* app, HAL* hal);
void App_Screen_handleHighScoreScreen(Application* app, HAL* hal);
void App_Screen_handleOptionsScreen(Application* app, HAL* hal);
void App_Screen_handleInstructionsScreen(Application* app, HAL* hal);
void App_Screen_handleGameScreen(Application* app, HAL* hal);

void App_Screen_showInstructionsScreen(Application* app, GFX* gfx);
void App_Screen_showHighScoreScreen(Application* app, GFX* gfx);
void App_Screen_showGameScreen(Application* app, GFX* gfx);
void App_Screen_showOptionsScreen(Application* app, GFX* gfx);

void resetSimpleGame(Application* app, HAL* hal, GFX* gfx);

void drawColorWheel(GFX* gfx, ColorWheel* cw);
void updateColorWheel(Application* app, HAL* hal);
void updateScore(Application* app, HAL* hal);
void endGame(Application* app, HAL* hal);

void drawPlayer(Application* app, GFX* gfx, int y, int ignoredColor);
void updateCharacter(Application* app, HAL* hal);

void initFloor(Application* app);
void updateFloor(Application* app);
void drawFloor(Application* app, GFX* gfx);
uint32_t getRandomColor(ColorWheel* wheel);
void App_Screen_handleGameOver(Application* app, HAL* hal);
void checkPlayerFloorCollision(Application* app_p);
void handleFalling(Application* app, HAL* hal);
void App_Screen_printDifficulty(Application* app, GFX* gfx);


#endif
