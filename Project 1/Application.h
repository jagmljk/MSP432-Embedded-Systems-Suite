#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <HAL/HAL.h>
#include <HAL/Graphics.h>
#include <HAL/Timer.h>

#define NUM_TEST_OPTIONS 3
#define MAX_HIGH_SCORES 5

// Game states
typedef enum {
    GAME_RUNNING,
    GAME_WIN,
    GAME_OVER
} GameState;

// Menu states
typedef enum {
    MAIN_MENU,
    SEE_INSTRUCTIONS,
    SEE_HIGHSCORE,
    START_GAME
} MenuState;

// Cursor positions for menu
typedef enum {
    CURSOR_0 = 0,
    CURSOR_1 = 1,
    CURSOR_2 = 2
} Cursor;

// Main application struct
typedef struct {
    MenuState state;
    Cursor arrow;
    UART_Baudrate baudChoice;

    int player_x, player_y;
    int enemy_x, enemy_y;
    int start_x, start_y;

    int borderOffsetX, borderOffsetY;
    int blockWidth, blockHeight;

    bool firstCall;
    bool gamePromptSent;
    bool mazeDrawn;
    bool gameEndScreenShown;

    int highScores[MAX_HIGH_SCORES];

    int moveCount;
    GameState gameState;

} Application;

// Core functions
Application Application_construct(void);
void Application_loop(Application* app, HAL* hal);
void Application_updateCommunications(Application* app, HAL* hal);
char Application_interpretIncomingChar(char);
uint32_t CircularIncrement(uint32_t value, uint32_t maximum);

// Screen display functions
void App_Screen_showmainmenu(Application* app, GFX* gfx);
void App_Screen_showInstructionsScreen(Application* app, GFX* gfx);
void App_Screen_showHighScoreScreen(Application* app_p, GFX* gfx_p);
void App_Screen_showGameScreen(Application* app, GFX* gfx);

// Screen handling functions
void App_Screen_handlemainmenu(Application* app, HAL* hal);
void App_Screen_handleInstructionsScreen(Application* app, HAL* hal);
void App_Screen_handleHighScoreScreen(Application* app_p, HAL* hal_p);
void App_Screen_handleGameScreen(Application* app, HAL* hal);

// Screen update functions
void App_Screen_updatemainmenu(Application* app, GFX* gfx);
void App_Screen_updateGameScreen(Application* app, GFX* gfx, int new_x, int new_y);
void App_Screen_updateEnemy(Application* app, GFX* gfx, int new_x, int new_y);

void App_Screen_showGameWin(Application* app, GFX* gfx);
void App_Screen_showGameOver(Application* app, GFX* gfx);

// Game logic
void showMaze(Application* app, GFX* gfx);
bool isValidMove(Application* app, GFX* gfx, int new_x, int new_y);
bool isenemyValidMove(Application* app, GFX* gfx, int new_x, int new_y);

// Input and movement
bool isValidPlayerCommand(char c);
void processPlayerCommand(Application* app, HAL* hal, char rxChar);
void moveEnemy(Application* app, HAL* hal);
void resetGame(Application* app, HAL* hal);
void showNextMovePrompt(HAL* hal);
void notifyInvalidMove(HAL* hal);
void notifyInvalidInput(HAL* hal);
void sortHighScores(Application* app_p);

#endif /* APPLICATION_H_ */
