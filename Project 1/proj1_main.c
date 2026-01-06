#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>
#include <HAL/Graphics.h>
#include <stdlib.h>

// Set up non-blocking LED on P1.0
static void InitNonBlockingLED() {
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Check button and toggle LED
static void PollNonBlockingLED() {
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

int main(void) {
    WDT_A_holdTimer();
    InitSystemTiming();
    HAL hal = HAL_construct();
    Application app = Application_construct();
    InitNonBlockingLED();

    App_Screen_showmainmenu(&app, &hal.gfx);

    while (true) {
        PollNonBlockingLED();
        HAL_refresh(&hal);
        Application_loop(&app, &hal);
    }
}

// Wraps value around when it reaches maximum
uint32_t CircularIncrement(uint32_t value, uint32_t maximum) {
    return (value + 1) % maximum;
}

// Set up initial application state
Application Application_construct() {
    Application app;
    app.baudChoice = BAUD_9600;
    app.firstCall = true;
    app.state = MAIN_MENU;
    app.arrow = CURSOR_0;
    app.gamePromptSent = false;
    app.player_x = 15;
    app.player_y = 123;
    app.borderOffsetX = 0;
    app.borderOffsetY = 28;
    app.blockWidth = 10;
    app.blockHeight = 10;
    app.mazeDrawn = false;
    app.start_y = 123;
    app.start_x = 15;
    app.enemy_x = 75;
    app.enemy_y = 43;
    app.moveCount = 0;
    app.gameState = GAME_RUNNING;
    app.gameEndScreenShown = false;

    // 9999 means empty slot
    int i;
    for (i = 0; i < MAX_HIGH_SCORES; i++) {
        app.highScores[i] = 9999;
    }

    return app;
}

// Main loop - handles all menu states
void Application_loop(Application* app_p, HAL* hal_p) {
    switch (app_p->state) {
        case MAIN_MENU:
            App_Screen_handlemainmenu(app_p, hal_p);
            break;
        case SEE_INSTRUCTIONS:
            App_Screen_handleInstructionsScreen(app_p, hal_p);
            break;
        case START_GAME:
            if (app_p->gameState == GAME_RUNNING) {
                App_Screen_handleGameScreen(app_p, hal_p);
            } else if (app_p->gameState == GAME_WIN) {
                App_Screen_showGameWin(app_p, &hal_p->gfx);
            } else if (app_p->gameState == GAME_OVER) {
                App_Screen_showGameOver(app_p, &hal_p->gfx);
            }
            break;
        case SEE_HIGHSCORE:
            App_Screen_handleHighScoreScreen(app_p, hal_p);
            break;
        default:
            break;
    }

    // BB1 returns to main menu after game ends
    if (Button_isTapped(&hal_p->boosterpackS1)) {
        if (app_p->gameState == GAME_OVER || app_p->gameState == GAME_WIN) {
            app_p->gameEndScreenShown = false;
            app_p->state = MAIN_MENU;
            App_Screen_showmainmenu(app_p, &hal_p->gfx);
            app_p->gameState = GAME_RUNNING;
        }
    }

    // Handle baud rate changes
    if (Button_isTapped(&hal_p->boosterpackS2) || app_p->firstCall) {
        Application_updateCommunications(app_p, hal_p);
    }

    // Echo received UART characters
    if (UART_hasChar(&hal_p->uart)) {
        char rxChar = UART_getChar(&hal_p->uart);
        char txChar = Application_interpretIncomingChar(rxChar);

        if (UART_canSend(&hal_p->uart)) {
            UART_sendChar(&hal_p->uart, rxChar);
        }
    }
}

// Update baud rate and show LED indicator
void Application_updateCommunications(Application* app_p, HAL* hal_p) {
    if (app_p->firstCall) {
        app_p->firstCall = false;
    } else {
        uint32_t newBaudNumber = CircularIncrement((uint32_t)app_p->baudChoice, NUM_BAUD_CHOICES);
        app_p->baudChoice = (UART_Baudrate)newBaudNumber;
    }

    UART_SetBaud_Enable(&hal_p->uart, app_p->baudChoice);
    LED_turnOff(&hal_p->launchpadLED2Red);
    LED_turnOff(&hal_p->launchpadLED2Green);
    LED_turnOff(&hal_p->launchpadLED2Blue);

    // LED color shows current baud rate
    switch (app_p->baudChoice) {
        case BAUD_9600:
            LED_turnOff(&hal_p->launchpadLED2Green);
            LED_turnOff(&hal_p->launchpadLED2Blue);
            LED_turnOn(&hal_p->launchpadLED2Red);
            break;
        case BAUD_19200:
            LED_turnOff(&hal_p->launchpadLED2Red);
            LED_turnOff(&hal_p->launchpadLED2Blue);
            LED_turnOn(&hal_p->launchpadLED2Green);
            break;
        case BAUD_38400:
            LED_turnOff(&hal_p->launchpadLED2Red);
            LED_turnOff(&hal_p->launchpadLED2Green);
            LED_turnOn(&hal_p->launchpadLED2Blue);
            break;
        case BAUD_57600:
            LED_turnOn(&hal_p->launchpadLED2Red);
            LED_turnOn(&hal_p->launchpadLED2Green);
            LED_turnOn(&hal_p->launchpadLED2Blue);
            break;
        default:
            break;
    }
}

// Check if input is number or letter
char Application_interpretIncomingChar(char rxChar) {
    char txChar = 'O';
    if (rxChar >= '0' && rxChar <= '9') {
        txChar = 'N';
    }
    if ((rxChar >= 'a' && rxChar <= 'z') || (rxChar >= 'A' && rxChar <= 'Z')) {
        txChar = 'L';
    }
    return (txChar);
}

// Update cursor position on main menu
void App_Screen_updatemainmenu(Application* app_p, GFX* gfx_p) {
    GFX_print(gfx_p, "  ", 4, 0);
    GFX_print(gfx_p, "  ", 5, 0);
    GFX_print(gfx_p, "  ", 6, 0);
    GFX_print(gfx_p, ">", 4 + app_p->arrow, 0);
}

// Handle main menu navigation
void App_Screen_handlemainmenu(Application* app_p, HAL* hal_p) {
    if (Button_isTapped(&hal_p->launchpadS1)) {
        if (app_p->arrow == CURSOR_0) {
            app_p->arrow = CURSOR_2;
        } else {
            app_p->arrow = (Cursor)(app_p->arrow - 1);
        }
        App_Screen_updatemainmenu(app_p, &hal_p->gfx);
    }

    if (Button_isTapped(&hal_p->launchpadS2)) {
        if (app_p->arrow == CURSOR_2) {
            app_p->arrow = CURSOR_0;
        } else {
            app_p->arrow = (Cursor)(app_p->arrow + 1);
        }
        App_Screen_updatemainmenu(app_p, &hal_p->gfx);
    }

    // Select menu option
    if (Button_isTapped(&hal_p->boosterpackS1)) {
        switch (app_p->arrow) {
            case CURSOR_0:
                app_p->state = START_GAME;
                resetGame(app_p, hal_p);
                break;
            case CURSOR_1:
                app_p->state = SEE_INSTRUCTIONS;
                App_Screen_showInstructionsScreen(app_p, &hal_p->gfx);
                break;
            case CURSOR_2:
                app_p->state = SEE_HIGHSCORE;
                App_Screen_showHighScoreScreen(app_p, &hal_p->gfx);
                break;
            default:
                break;
        }
    }
}

// Go back to main menu when S1 pressed
void App_Screen_handleInstructionsScreen(Application* app_p, HAL* hal_p) {
    if (Button_isTapped(&hal_p->boosterpackS1)) {
        app_p->state = MAIN_MENU;
        App_Screen_showmainmenu(app_p, &hal_p->gfx);
    }
}

// Go back to main menu when S1 pressed
void App_Screen_handleHighScoreScreen(Application* app_p, HAL* hal_p) {
    if (Button_isTapped(&hal_p->boosterpackS1)) {
        app_p->state = MAIN_MENU;
        App_Screen_showmainmenu(app_p, &hal_p->gfx);
    }
}

// Handle game input and logic
void App_Screen_handleGameScreen(Application* app_p, HAL* hal_p) {
    if (app_p->gameState != GAME_RUNNING)
        return;

    if (!app_p->gamePromptSent) {
        showNextMovePrompt(hal_p);
        app_p->gamePromptSent = true;
    }

    if (UART_hasChar(&hal_p->uart)) {
        char rxChar = UART_getChar(&hal_p->uart);

        if (isValidPlayerCommand(rxChar)) {
            processPlayerCommand(app_p, hal_p, rxChar);
        } else {
            notifyInvalidInput(hal_p);
            moveEnemy(app_p, hal_p);
            showNextMovePrompt(hal_p);
        }
    }

    if (Button_isTapped(&hal_p->boosterpackS1)) {
        resetGame(app_p, hal_p);
    }
}

// Check for valid movement commands
bool isValidPlayerCommand(char c) {
    return c == 'U' || c == 'u' || c == 'D' || c == 'd' ||
           c == 'L' || c == 'l' || c == 'R' || c == 'r';
}

// Move player based on input
void processPlayerCommand(Application* app_p, HAL* hal_p, char rxChar) {
    int new_x = app_p->player_x;
    int new_y = app_p->player_y;

    // Check collision with enemy
    if (new_x == app_p->enemy_x && new_y == app_p->enemy_y) {
        app_p->gameState = GAME_OVER;
        return;
    }

    // Calculate new position
    switch (rxChar) {
        case 'U': case 'u': new_y -= 10; break;
        case 'D': case 'd': new_y += 10; break;
        case 'L': case 'l': new_x -= 10; break;
        case 'R': case 'r': new_x += 10; break;
    }

    if (isValidMove(app_p, &hal_p->gfx, new_x, new_y)) {
        App_Screen_updateGameScreen(app_p, &hal_p->gfx, new_x, new_y);
        app_p->moveCount++;

        if (UART_canSend(&hal_p->uart)) {
            UART_sendChar(&hal_p->uart, rxChar);
            UART_sendChar(&hal_p->uart, '\r');
        }
    } else {
        notifyInvalidMove(hal_p);
    }

    moveEnemy(app_p, hal_p);
    showNextMovePrompt(hal_p);
}

// Show movement prompt via UART
void showNextMovePrompt(HAL* hal_p) {
    UART_sendString(&hal_p->uart,
        "\r\nEnter movement u=up, l=left, d=down, r=right (U/L/D/R or u/l/d/r):\r\n");
}

// Tell user move was invalid
void notifyInvalidMove(HAL* hal_p) {
    UART_sendString(&hal_p->uart,
        "Invalid move! Player would go out of bounds or hit a wall.");
}

// Tell user input was invalid
void notifyInvalidInput(HAL* hal_p) {
    UART_sendString(&hal_p->uart,
        "Invalid input! Use only U/L/D/R or u/l/d/r.");
}

// Reset game to starting state
void resetGame(Application* app_p, HAL* hal_p) {
    app_p->gamePromptSent = false;
    app_p->mazeDrawn = false;
    app_p->gameEndScreenShown = false;
    app_p->moveCount = 0;

    app_p->player_x = app_p->start_x;
    app_p->player_y = app_p->start_y;
    app_p->enemy_x = 75;
    app_p->enemy_y = 43;

    GFX_clear(&hal_p->gfx);
    App_Screen_showGameScreen(app_p, &hal_p->gfx);

    app_p->gameState = GAME_RUNNING;
}

// Draw main menu screen
void App_Screen_showmainmenu(Application* app_p, GFX* gfx_p) {
    GFX_clear(gfx_p);
    GFX_resetColors(gfx_p);

    GFX_print(gfx_p, "Maze Runner Game", 0, 2);
    GFX_print(gfx_p, "ECE 2564 Project 1", 1, 2);
    GFX_print(gfx_p, " ", 2, 2);

    GFX_print(gfx_p, "Play Game", 4, 4);
    GFX_print(gfx_p, "Instructions", 5, 4);
    GFX_print(gfx_p, "High Score", 6, 4);

    GFX_print(gfx_p, "LB1 to go up", 8, 2);
    GFX_print(gfx_p, "LB2 to go down", 9, 2);
    GFX_print(gfx_p, "BB1 to select", 10, 2);

    GFX_print(gfx_p, ">", 4 + app_p->arrow, 0);
}

// Draw high score screen
void App_Screen_showHighScoreScreen(Application* app_p, GFX* gfx_p) {
    GFX_clear(gfx_p);
    GFX_print(gfx_p, "High Scores", 1, 5);
    GFX_print(gfx_p, "Best Moves", 3, 5.9);
    GFX_print(gfx_p, "----------------", 4, 3);

    int i;
    for (i = 0; i < MAX_HIGH_SCORES; i++) {
        char scoreText[20];
        if (app_p->highScores[i] == 9999) {
            snprintf(scoreText, sizeof(scoreText), "--");
        } else {
            snprintf(scoreText, sizeof(scoreText), "%d", app_p->highScores[i]);
        }
        GFX_print(gfx_p, scoreText, 5 + i, 10);
    }

    GFX_print(gfx_p, "Press BB1 to return.", 11, 1);
    GFX_resetColors(gfx_p);
}

// Draw instructions screen
void App_Screen_showInstructionsScreen(Application* app_p, GFX* gfx_p) {
    GFX_clear(gfx_p);

    GFX_print(gfx_p, "Instructions", 0, 4);
    GFX_print(gfx_p, "Reach the goal,.", 2, 1);
    GFX_print(gfx_p, "avoid enemy.Finish", 3, 1);
    GFX_print(gfx_p, "in fewest moves.", 4, 1);

    GFX_print(gfx_p, "UART:l=left,r=right,", 6, 1);
    GFX_print(gfx_p, "u=up, d=down.", 7, 1);

    GFX_print(gfx_p, "Press BB1 to return.", 8, 1);
}

// Draw game screen with maze and player
void App_Screen_showGameScreen(Application* app_p, GFX* gfx_p) {
    GFX_clear(gfx_p);

    if (!app_p->mazeDrawn) {
        showMaze(app_p, gfx_p);
        app_p->mazeDrawn = true;
    }

    GFX_setForeground(gfx_p, 0xFFFFFF);
    GFX_print(gfx_p, "Moves: 0", 0, 6);

    // Draw player (blue)
    GFX_setForeground(gfx_p, 0x0000FF);
    Graphics_fillCircle(&gfx_p->context, app_p->player_x, app_p->player_y, 4);

    App_Screen_updateEnemy(app_p, gfx_p, app_p->enemy_x, app_p->enemy_y);
}

// Update player position on screen
void App_Screen_updateGameScreen(Application* app_p, GFX* gfx_p, int new_x, int new_y) {
    // Clear old position
    GFX_removeSolidCircle(gfx_p, app_p->player_x, app_p->player_y, 4);

    int old_x = app_p->player_x;
    int old_y = app_p->player_y;

    int blockWidth = app_p->blockWidth;
    int blockHeight = app_p->blockHeight;

    int adjusted_old_x = old_x - (blockWidth / 2);
    int adjusted_old_y = old_y - (blockHeight / 2);

    int oldMaze_x = (adjusted_old_x - app_p->borderOffsetX) / blockWidth;
    int oldMaze_y = (adjusted_old_y - app_p->borderOffsetY) / blockHeight;
    int newMaze_x = (new_x - app_p->borderOffsetX) / blockWidth;
    int newMaze_y = (new_y - app_p->borderOffsetY) / blockHeight;

    // Redraw background if moved to new cell
    if (oldMaze_x != newMaze_x || oldMaze_y != newMaze_y) {
        GFX_setForeground(gfx_p, 0xFFFFFF);
        GFX_drawSolidRectangle(gfx_p, adjusted_old_x, adjusted_old_y, blockWidth, blockHeight);
    }

    app_p->player_x = new_x;
    app_p->player_y = new_y;

    // Update move counter display
    char movesText[20];
    GFX_setForeground(gfx_p, 0xFFFFFF);
    snprintf(movesText, sizeof(movesText), "Moves: %d", app_p->moveCount);
    GFX_print(gfx_p, movesText, 0, 6);

    // Draw player at new position (blue)
    GFX_setForeground(gfx_p, 0x0000FF);
    Graphics_fillCircle(&gfx_p->context, app_p->player_x, app_p->player_y, 4);

    App_Screen_updateEnemy(app_p, gfx_p, app_p->enemy_x, app_p->enemy_y);
}

// Update enemy position on screen
void App_Screen_updateEnemy(Application* app_p, GFX* gfx_p, int new_x, int new_y) {
    // Clear old position
    GFX_removeSolidCircle(gfx_p, app_p->enemy_x, app_p->enemy_y, 4);

    int old_x = app_p->enemy_x;
    int old_y = app_p->enemy_y;

    int blockWidth = app_p->blockWidth;
    int blockHeight = app_p->blockHeight;

    int adjusted_old_x = old_x - (blockWidth / 2);
    int adjusted_old_y = old_y - (blockHeight / 2);

    int oldMaze_x = (adjusted_old_x - app_p->borderOffsetX) / blockWidth;
    int oldMaze_y = (adjusted_old_y - app_p->borderOffsetY) / blockHeight;
    int newMaze_x = (new_x - app_p->borderOffsetX) / blockWidth;
    int newMaze_y = (new_y - app_p->borderOffsetY) / blockHeight;

    // Redraw background if moved to new cell
    if (oldMaze_x != newMaze_x || oldMaze_y != newMaze_y) {
        GFX_setForeground(gfx_p, 0xFFFFFF);
        GFX_drawSolidRectangle(gfx_p, adjusted_old_x, adjusted_old_y, blockWidth, blockHeight);
    }

    app_p->enemy_x = new_x;
    app_p->enemy_y = new_y;

    // Draw enemy at new position (red)
    GFX_setForeground(gfx_p, 0xFF0000);
    Graphics_fillCircle(&gfx_p->context, app_p->enemy_x, app_p->enemy_y, 4);
}

// Draw the maze grid
void showMaze(Application* app_p, GFX* gfx_p) {
    char maze[10][10] = {
        { 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', '_', '_', 'G' },
        { 'X', '_', 'X', 'X', 'X', '_', '_', '_', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', '_', 'X', 'X', '_', '_', '_', '_', 'X' },
        { 'X', '_', '_', '_', 'X', 'X', 'X', '_', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', 'X', 'X', '_', '_', '_', '_', '_', 'X' },
        { 'X', 'S', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X' }
    };

    int j, k;

    for (j = 0; j < 10; j++) {
        for (k = 0; k < 10; k++) {
            if (maze[j][k] == 'X') {
                GFX_setForeground(gfx_p, 0xFF0000);  // Red for walls
            } else if (maze[j][k] == 'S') {
                GFX_setForeground(gfx_p, 0xFFFF00);  // Yellow for start
            } else if (maze[j][k] == 'G') {
                GFX_setForeground(gfx_p, 0x00FF00);  // Green for goal
            } else {
                GFX_setForeground(gfx_p, 0xFFFFFF);  // White for path
            }

            GFX_drawSolidRectangle(gfx_p,
                app_p->borderOffsetX + (k * app_p->blockWidth),
                app_p->borderOffsetY + (j * app_p->blockHeight),
                app_p->blockWidth,
                app_p->blockHeight);
        }
    }
}

// Check if player can move to new position
bool isValidMove(Application* app_p, GFX* gfx_p, int new_x, int new_y){
    char maze[10][10] = {
        { 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', '_', '_', 'G' },
        { 'X', '_', 'X', 'X', 'X', '_', '_', '_', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', '_', 'X', 'X', '_', '_', '_', '_', 'X' },
        { 'X', '_', '_', '_', 'X', 'X', 'X', '_', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', 'X', 'X', '_', '_', '_', '_', '_', 'X' },
        { 'X', 'S', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X' }
    };

    // Check bounds
    if (new_x < app_p->borderOffsetX || new_x >= (app_p->borderOffsetX + 10 * app_p->blockWidth) ||
        new_y < app_p->borderOffsetY || new_y >= (app_p->borderOffsetY + 10 * app_p->blockHeight)) {
        return false;
    }

    int maze_x = (new_x - app_p->borderOffsetX) / app_p->blockWidth;
    int maze_y = (new_y - app_p->borderOffsetY) / app_p->blockHeight;

    // Check if reached goal
    if (maze[maze_y][maze_x] == 'G') {
        app_p->gameState = GAME_WIN;
        return true;
    }

    // Check if hit wall
    if (maze[maze_y][maze_x] == 'X') {
        return false;
    }

    return true;
}

// Check if enemy can move to new position
bool isenemyValidMove(Application* app_p, GFX* gfx_p, int enemy_new_x, int enemy_new_y){
    char maze[10][10] = {
        { 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', '_', '_', 'G' },
        { 'X', '_', 'X', 'X', 'X', '_', '_', '_', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', '_', 'X', 'X', '_', '_', '_', '_', 'X' },
        { 'X', '_', '_', '_', 'X', 'X', 'X', '_', '_', 'X' },
        { 'X', '_', '_', '_', '_', '_', '_', 'X', '_', 'X' },
        { 'X', '_', 'X', 'X', '_', '_', '_', '_', '_', 'X' },
        { 'X', 'S', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X' }
    };

    // Check bounds
    if (enemy_new_x < app_p->borderOffsetX || enemy_new_x >= (app_p->borderOffsetX + 10 * app_p->blockWidth) ||
        enemy_new_y < app_p->borderOffsetY || enemy_new_y >= (app_p->borderOffsetY + 10 * app_p->blockHeight)) {
        return false;
    }

    int maze_x = (enemy_new_x - app_p->borderOffsetX) / app_p->blockWidth;
    int maze_y = (enemy_new_y - app_p->borderOffsetY) / app_p->blockHeight;

    // Check if hit wall
    if (maze[maze_y][maze_x] == 'X') {
        return false;
    }

    return true;
}

// Move enemy randomly
void moveEnemy(Application* app_p, HAL* hal_p) {
    int directions[4][2] = {
        {0, -10},  // Up
        {0, 10},   // Down
        {-10, 0},  // Left
        {10, 0}    // Right
    };

    while (true) {
        int choice = rand() % 4;
        int dx = directions[choice][0];
        int dy = directions[choice][1];

        int new_x = app_p->enemy_x + dx;
        int new_y = app_p->enemy_y + dy;

        // Check if enemy caught player
        if (new_x == app_p->player_x && new_y == app_p->player_y) {
            app_p->gameState = GAME_OVER;
            return;
        }

        if (isenemyValidMove(app_p, &hal_p->gfx, new_x, new_y)) {
            App_Screen_updateEnemy(app_p, &hal_p->gfx, new_x, new_y);
            app_p->enemy_x = new_x;
            app_p->enemy_y = new_y;
            break;
        }
    }
}

// Show win screen and save high score
void App_Screen_showGameWin(Application* app_p, GFX* gfx_p) {
    if (app_p->gameState == GAME_WIN && !app_p->gameEndScreenShown) {
        int i;
        int j;

        // Insert score if it's good enough
        for (i = 0; i < MAX_HIGH_SCORES; i++) {
            if (app_p->moveCount < app_p->highScores[i]) {
                for (j = MAX_HIGH_SCORES - 1; j > i; j--) {
                    app_p->highScores[j] = app_p->highScores[j - 1];
                }
                app_p->highScores[i] = app_p->moveCount;
                break;
            }
        }

        sortHighScores(app_p);

        GFX_clear(gfx_p);
        GFX_setForeground(gfx_p, 0xFFFFFF);
        GFX_print(gfx_p, "Congratulations!", 5, 2);
        GFX_print(gfx_p, "You reached the goal!", 6, 1);
        GFX_print(gfx_p, "Press BB1 to return.", 8, 2);

        app_p->gameEndScreenShown = true;
    }
}

// Show game over screen
void App_Screen_showGameOver(Application* app_p, GFX* gfx_p) {
    if (app_p->gameState == GAME_OVER && !app_p->gameEndScreenShown) {
        GFX_clear(gfx_p);
        GFX_setForeground(gfx_p, 0xFFFFFF);
        GFX_print(gfx_p, "Game Over!", 5, 4);
        GFX_print(gfx_p, "You were caught!", 6, 3);
        GFX_print(gfx_p, "Try again.", 7, 4);
        GFX_print(gfx_p, "Press BB1 to return.", 9, 2);

        app_p->gameEndScreenShown = true;
    }
}

// Sort high scores (lowest is best)
void sortHighScores(Application* app_p) {
    int i, j;
    for (i = 0; i < MAX_HIGH_SCORES - 1; i++) {
        for (j = i + 1; j < MAX_HIGH_SCORES; j++) {
            if (app_p->highScores[i] > app_p->highScores[j]) {
                int temp = app_p->highScores[i];
                app_p->highScores[i] = app_p->highScores[j];
                app_p->highScores[j] = temp;
            }
        }
    }
}
