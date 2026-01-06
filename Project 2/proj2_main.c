#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>
#include <HAL/Graphics.h>

static void InitNonBlockingLED(void)
{
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

static void PollNonBlockingLED(void)
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

int main(void)
{
    WDT_A_holdTimer();
    InitSystemTiming();
    HAL hal = HAL_construct();
    Application app = Application_construct();
    InitNonBlockingLED();
    srand((unsigned) time(NULL));

    while (true)
    {
        PollNonBlockingLED();
        HAL_refresh(&hal);
        Application_loop(&app, &hal);
    }
}

// Constructs and initializes the Application structure with starting values.
Application Application_construct(void)
{
    Application app;
    // Initialize game parameters.
    app.jumpHeight = 25;
    app.playerRadius = 5;
    app.playerCenterX = 10;
    app.playerJumpDelay = 1.5;
    app.maxHighScores = 5;
    app.difficulty = 0;

    // Allocate memory for high score array and initialize scores to zero.
    app.highScores = malloc(app.maxHighScores * sizeof(int));
    int i;
    for (i = 0; i < app.maxHighScores; i++)
        app.highScores[i] = 0;

    // Set initial game state and screen flags.
    app.state = STATE_TITLE;
    app.titleScreenShown = false;
    app.screenNeedsRedraw = true;
    app.joystickCentered = true;
    app.arrow = CURSOR_0;
    app.titleTimer = SWTimer_construct(3000); // Title screen display timer.
    app.playerY = 100; // Starting Y position for the player.
    app.lastPlayerY = app.playerY;
    app.jumpState = JUMP_NONE; // No jump in progress.
    app.jumpProgress = 0;
    app.isOnGround = true;
    app.jumpTimer = SWTimer_construct(app.playerJumpDelay); // Timer for jump progress.
    app.scoreTimer = SWTimer_construct(1000); // Score update interval.
    app.score = 0;
    app.fallTimer = SWTimer_construct(10); // Timer controlling falling speed.

    // Initialize color wheel with preset colors.
    app.colorWheel.center = 0xFFFFFF;
    app.colorWheel.up     = 0x00FF00;
    app.colorWheel.down   = 0x0000FF;
    app.colorWheel.left   = 0xFFFF00;
    app.colorWheel.right  = 0xFF00FF;

    app.isFalling = false;
    initFloor(&app); // Initialize the floor segments for the game.
    app.floorTimer = SWTimer_construct(5);
    SWTimer_start(&app.floorTimer);
    return app;
}

// Main loop to update the application state based on the current game state.
void Application_loop(Application* app_p, HAL* hal_p)
{
    switch (app_p->state)
    {
        case STATE_TITLE:
            // Display title screen if not already shown.
            if (!app_p->titleScreenShown)
            {
                GFX_clear(&hal_p->gfx);
                GFX_setForeground(&hal_p->gfx, 0xFFFFFF);
                GFX_print(&hal_p->gfx, "Color Jump", 5, 4);
                
                SWTimer_start(&app_p->titleTimer); // Start timer to show title.
                app_p->titleScreenShown = true;
            }
            // Move to menu after title timer expires.
            if (SWTimer_expired(&app_p->titleTimer))
            {
                app_p->state = STATE_MENU;
                App_Screen_showmainmenu(app_p, &hal_p->gfx);
            }
            break;
        case STATE_MENU:
            App_Screen_handlemainmenu(app_p, hal_p); // Process menu input.
            break;
        case STATE_INSTRUCTIONS:
            App_Screen_handleInstructionsScreen(app_p, hal_p); // Instructions screen.
            break;
        case STATE_HIGHSCORE:
            App_Screen_handleHighScoreScreen(app_p, hal_p); // High score display.
            break;
        case STATE_OPTIONS:
            App_Screen_handleOptionsScreen(app_p, hal_p); // Options menu.
            break;
        case STATE_GAME:
            // Update floor if not falling and floor timer has expired.
            if (!app_p->isFalling && SWTimer_expired(&app_p->floorTimer))
            {
                updateFloor(app_p);
                SWTimer_start(&app_p->floorTimer);
            }
            // Redraw screen if needed.
            if (app_p->screenNeedsRedraw)
                resetSimpleGame(app_p, hal_p, &hal_p->gfx);
            // Handle game screen logic.
            App_Screen_handleGameScreen(app_p, hal_p);
            break;
        case STATE_GAMEOVER:
            App_Screen_handleGameOver(app_p, hal_p); // Handle game over logic.
            break;
        default:
            break;
    }
}

// Displays the main menu screen.
void App_Screen_showmainmenu(Application* app_p, GFX* gfx_p)
{
    GFX_clear(gfx_p);
    GFX_setForeground(gfx_p, 0xFFFFFF);
    GFX_print(gfx_p, "Main Menu", 2, 4);
    GFX_print(gfx_p, "Play Game", 4, 4);
    GFX_print(gfx_p, "Instructions", 5, 4);
    GFX_print(gfx_p, "Options", 6, 4);
    GFX_print(gfx_p, "High Scores", 7, 4);
    GFX_print(gfx_p, ">", 4 + app_p->arrow, 0); // Show cursor next to selected item.
    GFX_print(gfx_p, "Press BB1 to select", 9, 1);
}

// Updates the main menu display, repositioning the cursor.
void App_Screen_updatemainmenu(Application* app_p, GFX* gfx_p)
{
    GFX_print(gfx_p, "  ", 4, 0); // Clear previous cursor positions.
    GFX_print(gfx_p, "  ", 5, 0);
    GFX_print(gfx_p, "  ", 6, 0);
    GFX_print(gfx_p, "  ", 7, 0);
    GFX_print(gfx_p, ">", 4 + app_p->arrow, 0); // Draw cursor at new position.
}

// Handles user input and selection in the main menu.
void App_Screen_handlemainmenu(Application* app_p, HAL* hal_p)
{
    uint16_t joyY = hal_p->joystick.y;
    // Move up through menu items.
    if (joyY > 14000 && app_p->joystickCentered)
    {
        app_p->arrow = (app_p->arrow == CURSOR_0) ? CURSOR_3 : (Cursor)(app_p->arrow - 1);
        App_Screen_updatemainmenu(app_p, &hal_p->gfx);
        app_p->joystickCentered = false;
    }
    // Move down through menu items.
    else if (joyY < 2000 && app_p->joystickCentered)
    {
        app_p->arrow = (app_p->arrow == CURSOR_3) ? CURSOR_0 : (Cursor)(app_p->arrow + 1);
        App_Screen_updatemainmenu(app_p, &hal_p->gfx);
        app_p->joystickCentered = false;
    }
    // Reset joystick centered flag when joystick is in the center range.
    if (joyY >= 6000 && joyY <= 10000)
        app_p->joystickCentered = true;
    // Select menu option on button press.
    if (Button_isTapped(&hal_p->boosterpackS1))
    {
        switch (app_p->arrow)
        {
            case CURSOR_0:
                app_p->state = STATE_GAME;
                app_p->screenNeedsRedraw = true;
                break;
            case CURSOR_1:
                app_p->state = STATE_INSTRUCTIONS;
                App_Screen_showInstructionsScreen(app_p, &hal_p->gfx);
                break;
            case CURSOR_2:
                app_p->state = STATE_OPTIONS;
                App_Screen_showOptionsScreen(app_p, &hal_p->gfx);
                break;
            case CURSOR_3:
                app_p->state = STATE_HIGHSCORE;
                App_Screen_showHighScoreScreen(app_p, &hal_p->gfx);
                break;
            default:
                break;
        }
    }
}

// Displays the instructions screen.
void App_Screen_showInstructionsScreen(Application* app_p, GFX* gfx_p)
{
    GFX_clear(gfx_p);
    GFX_print(gfx_p, "Instructions", 0, 4);
    GFX_print(gfx_p, "Use joystick to swap ", 2, 1);
    GFX_print(gfx_p, "colors. BB1 to jump, ", 3, 1);
    GFX_print(gfx_p, "BB2 to end game", 4, 1);
    GFX_print(gfx_p, "Press BB1 to return.", 8, 1);
}

// Displays the options screen.
void App_Screen_showOptionsScreen(Application* app, GFX* gfx)
{
    GFX_clear(gfx);
    GFX_print(gfx, "Options", 0, 4);

    GFX_print(gfx, "Difficulty: Easy", 2, 1);

    GFX_print(gfx, "Press BB2 to change", 4, 1);
    GFX_print(gfx, "Press BB1 to return", 5, 1);
    GFX_print(gfx, "3 Total modes", 6, 1);
}

// Displays the high score screen.
void App_Screen_showHighScoreScreen(Application* app, GFX* gfx)
{
    GFX_clear(gfx);
    GFX_print(gfx, "High Scores", 0, 4);
    char scoreStr[20];
    int i;
    // Loop through and display each high score.
    for (i = 0; i < app->maxHighScores; i++)
    {
        sprintf(scoreStr, "%d : %d", i + 1, app->highScores[i]);
        GFX_print(gfx, scoreStr, 2 + i, 1);
    }
    GFX_print(gfx, "Press BB1 to return", 8, 1);
}

// Returns to main menu from the high score screen upon button press.
void App_Screen_handleHighScoreScreen(Application* app, HAL* hal)
{
    if (Button_isTapped(&hal->boosterpackS1))
    {
         app->state = STATE_MENU;
         App_Screen_showmainmenu(app, &hal->gfx);
    }
}

// Print the current difficulty setting on the options screen.
void App_Screen_printDifficulty(Application* app, GFX* gfx)
{
    GFX_setForeground(gfx, 0x000000);
    GFX_print(gfx, "                    ", 2, 1); // Clear previous difficulty text.

    GFX_setForeground(gfx, 0xFFFFFF);

    if (app->difficulty == 0)
        GFX_print(gfx, "Difficulty: Easy", 2, 1);
    else if (app->difficulty == 1)
        GFX_print(gfx, "Difficulty: Hard", 2, 1);
    else if (app->difficulty == 2)
        GFX_print(gfx, "Difficulty: Increase", 2, 1);
}

// Handle button input on the options screen to change difficulty or return to menu.
void App_Screen_handleOptionsScreen(Application* app, HAL* hal)
{
    if (Button_isTapped(&hal->boosterpackS2))
    {
        app->difficulty = (app->difficulty + 1) % 3;
        App_Screen_printDifficulty(app, &hal->gfx);
    }

    if (Button_isTapped(&hal->boosterpackS1))
    {
        app->state = STATE_MENU;
        app->screenNeedsRedraw = true;
        App_Screen_showmainmenu(app, &hal->gfx);
    }
}

// Handles the instructions screen; return to menu on button press.
void App_Screen_handleInstructionsScreen(Application* app, HAL* hal)
{
    if (Button_isTapped(&hal->boosterpackS1))
    {
         app->state = STATE_MENU;
         App_Screen_showmainmenu(app, &hal->gfx);
    }
}

// Checks if the player's character collides with the floor.
// If the player is not on the correct colored floor segment, start falling.
void checkPlayerFloorCollision(Application* app)
{
    int playerBottomY = app->playerY + app->playerRadius;
    const int floorTopY = 105;
    if (!app->isFalling && playerBottomY >= floorTopY && playerBottomY <= (floorTopY + 18))
    {
         int px = app->playerCenterX;
         int i;
         // Check collision with each floor segment.
         for (i = 0; i < app->numFloorSegments; i++)
         {
             FloorSegment seg = app->floorSegments[i];
             if (px >= seg.x && px < seg.x + seg.width)
             {
                 // If player color doesn't match floor segment color, trigger falling.
                 if (seg.color != app->colorWheel.center)
                 {
                     app->isFalling = true;
                     SWTimer_start(&app->fallTimer);
                 }
                 else
                 {
                     app->isOnGround = true;
                 }
                 return;
             }
         }
    }
    else
         app->isOnGround = false;
}

// Manages the game screen updates: character, floor collisions, score, etc.
void App_Screen_handleGameScreen(Application* app, HAL* hal)
{
    if (app->screenNeedsRedraw)
    {
         GFX_clear(&hal->gfx);
         char scoreStr[20];
         sprintf(scoreStr, "Score : %d", app->score);
         GFX_print(&hal->gfx, scoreStr, 0, 0);
         drawFloor(app, &hal->gfx);
         drawColorWheel(&hal->gfx, &app->colorWheel);
         drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
         app->screenNeedsRedraw = false;
         app->lastPlayerY = app->playerY;
         return;
    }
    updateCharacter(app, hal);  // Update player character position/jump.
    checkPlayerFloorCollision(app); // Check collision between player and floor.
    updateColorWheel(app, hal); // Handle joystick input to update color wheel.
    updateScore(app, hal);      // Update game score display.
    if (Button_isTapped(&hal->boosterpackS2))
    {
         endGame(app, hal); // End game if designated button is tapped.
         return;
    }
    if (!app->isFalling)
         drawFloor(app, &hal->gfx); // Continuously redraw floor if player is not falling.
}

// Display game screen contents.
void App_Screen_showGameScreen(Application* app, GFX* gfx)
{
    GFX_clear(gfx);
    char scoreStr[20];
    sprintf(scoreStr, "Score : %d", app->score);
    GFX_print(gfx, scoreStr, 0, 0);
    drawFloor(app, gfx);
    drawColorWheel(gfx, &app->colorWheel);
    drawPlayer(app, gfx, app->playerY, app->colorWheel.center);
}

// Resets the game state for a new game round.
void resetSimpleGame(Application* app, HAL* hal, GFX* gfx)
{
    app->screenNeedsRedraw = false;
    app->joystickCentered = true;
    app->playerY = 100;
    app->jumpState = JUMP_NONE;
    app->jumpProgress = 0;
    app->isOnGround = true;
    app->score = 0;
    SWTimer_start(&app->scoreTimer); // Restart score timer.
    initFloor(app); // Reinitialize floor segments.
    App_Screen_showGameScreen(app, gfx);
}

// Draw the player's character as a solid circle.
void drawPlayer(Application* app, GFX* gfx, int y, int ignoredColor)
{
    GFX_setForeground(gfx, app->colorWheel.center);
    GFX_drawSolidCircle(gfx, app->playerCenterX, y, app->playerRadius);
}

// Update the character position based on jump state and handle drawing.
void updateCharacter(Application* app, HAL* hal)
{
    if (app->isFalling)
    {
        handleFalling(app, hal); // If falling, process falling motion.
        return;
    }

    // Handle jump states: start jump, move up or down.
    switch (app->jumpState)
    {
        case JUMP_NONE:
            // Start jump if button is tapped and character is on ground.
            if (Button_isTapped(&hal->boosterpackS1) && app->isOnGround)
            {
                app->jumpState = JUMP_UP;
                app->jumpProgress = 0;
                app->isOnGround = false;
                SWTimer_start(&app->jumpTimer);
            }
            break;

        case JUMP_UP:
        case JUMP_DOWN:
            // Wait for jump timer expiration before updating jump progress.
            if (!SWTimer_expired(&app->jumpTimer)) break;

            app->lastPlayerY = app->playerY;

            if (app->jumpState == JUMP_UP)
            {
                app->playerY -= 1; // Move player up.
                app->jumpProgress++;

                if (app->jumpProgress >= app->jumpHeight)
                {
                    app->jumpState = JUMP_DOWN;
                    app->jumpProgress = 0;
                }
            }
            else
            {
                app->playerY += 1; // Move player down.
                app->jumpProgress++;

                if (app->jumpProgress >= app->jumpHeight)
                {
                    app->jumpState = JUMP_NONE;
                    app->isOnGround = true;
                }
            }

            // Erase previous player drawing and redraw at new position.
            GFX_setForeground(&hal->gfx, 0x000000);
            GFX_drawSolidCircle(&hal->gfx, app->playerCenterX, app->lastPlayerY, app->playerRadius);
            drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
            SWTimer_start(&app->jumpTimer);
            break;
    }
}

// Handles the falling state for the player character.
void handleFalling(Application* app, HAL* hal)
{
    if (SWTimer_expired(&app->fallTimer))
    {
        app->lastPlayerY = app->playerY;
        app->playerY += 1; // Increase Y position to simulate falling.

        GFX_setForeground(&hal->gfx, 0x000000);
        int dx;
        // Erase the area where the player was previously drawn.
        for (dx = -app->playerRadius; dx <= app->playerRadius; dx++)
        {
            int colX = app->playerCenterX + dx;
            if (colX >= 0 && colX < 128)
            {
                GFX_drawLine(&hal->gfx, colX, app->lastPlayerY - app->playerRadius,
                             colX, app->playerY + app->playerRadius);
            }
        }

        drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
        SWTimer_start(&app->fallTimer);

        // Check if player has fallen off screen.
        if (app->playerY > 128 + app->playerRadius)
        {
            app->isFalling = false;
            endGame(app, hal);
        }
    }
}

// Draw the color wheel on the screen.
void drawColorWheel(GFX* gfx, ColorWheel* cw)
{
    GFX_setForeground(gfx, cw->center);
    GFX_drawSolidCircle(gfx, 100, 30, 5);
    GFX_setForeground(gfx, cw->up);
    GFX_drawSolidCircle(gfx, 100, 20, 5);
    GFX_setForeground(gfx, cw->down);
    GFX_drawSolidCircle(gfx, 100, 40, 5);
    GFX_setForeground(gfx, cw->left);
    GFX_drawSolidCircle(gfx, 90, 30, 5);
    GFX_setForeground(gfx, cw->right);
    GFX_drawSolidCircle(gfx, 110, 30, 5);
}

// Updates the color wheel based on joystick direction input.
void updateColorWheel(Application* app, HAL* hal)
{
    if (hal->joystick.y > 14000 && app->joystickCentered)
    {
         // Swap colors for upward joystick motion.
         int temp = app->colorWheel.center;
         app->colorWheel.center = app->colorWheel.up;
         app->colorWheel.up = temp;
         app->joystickCentered = false;
         drawColorWheel(&hal->gfx, &app->colorWheel);
         drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
    }
    else if (hal->joystick.y < 2000 && app->joystickCentered)
    {
         // Swap colors for downward joystick motion.
         int temp = app->colorWheel.center;
         app->colorWheel.center = app->colorWheel.down;
         app->colorWheel.down = temp;
         app->joystickCentered = false;
         drawColorWheel(&hal->gfx, &app->colorWheel);
         drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
    }
    else if (hal->joystick.x < 2000 && app->joystickCentered)
    {
         // Swap colors for leftward joystick motion.
         int temp = app->colorWheel.center;
         app->colorWheel.center = app->colorWheel.left;
         app->colorWheel.left = temp;
         app->joystickCentered = false;
         drawColorWheel(&hal->gfx, &app->colorWheel);
         drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
    }
    else if (hal->joystick.x > 14000 && app->joystickCentered)
    {
         // Swap colors for rightward joystick motion.
         int temp = app->colorWheel.center;
         app->colorWheel.center = app->colorWheel.right;
         app->colorWheel.right = temp;
         app->joystickCentered = false;
         drawColorWheel(&hal->gfx, &app->colorWheel);
         drawPlayer(app, &hal->gfx, app->playerY, app->colorWheel.center);
    }
    else if (hal->joystick.x >= 6000 && hal->joystick.x <= 10000 &&
             hal->joystick.y >= 6000 && hal->joystick.y <= 10000)
    {
         // Reset joystick flag when joystick is centered.
         app->joystickCentered = true;
    }
}

// Updates the game score based on difficulty and elapsed time.
void updateScore(Application* app, HAL* hal)
{
    if (SWTimer_expired(&app->scoreTimer))
    {
         if (app->difficulty == 1)
              app->score += 3;
         else if (app->difficulty == 2)
              app->score += 1;
         else
              app->score += 1;
         char scoreStr[20];
         sprintf(scoreStr, "Score : %d", app->score);

         GFX_setForeground(&hal->gfx, 0xFFFFFF);

         GFX_print(&hal->gfx, scoreStr, 0, 0);
         SWTimer_start(&app->scoreTimer); // Restart the score timer.
    }
}

// Ends the game and updates the high score table.
void endGame(Application* app, HAL* hal)
{
    int newScore = app->score;
    int i;
    int j;
    // Insert new score into high score list if it is high enough.
    for (i = 0; i < app->maxHighScores; i++)
    {
         if (newScore > app->highScores[i])
         {
              for (j = app->maxHighScores - 1; j > i; j--)
              {
                   app->highScores[j] = app->highScores[j - 1];
              }
              app->highScores[i] = newScore;
              break;
         }
    }
    app->state = STATE_GAMEOVER;
    app->screenNeedsRedraw = true;
}

// Handles the game over screen and input for restarting or returning to the menu.
void App_Screen_handleGameOver(Application* app, HAL* hal)
{
    if (app->screenNeedsRedraw)
    {
         GFX_clear(&hal->gfx);
         GFX_setForeground(&hal->gfx, 0xFFFFFF);
         char gameOverMsg[30];
         sprintf(gameOverMsg, "Game Over! Score: %d", app->score);
         GFX_print(&hal->gfx, gameOverMsg, 4, 0);
         GFX_print(&hal->gfx, "Press BB1 to Retry", 6, 0);
         GFX_print(&hal->gfx, "Press BB2 for Menu", 7, 0);
         app->screenNeedsRedraw = false;
    }
    if (Button_isTapped(&hal->boosterpackS1))
    {
         resetSimpleGame(app, hal, &hal->gfx); // Reset game state for retry.
         app->state = STATE_GAME;
         app->screenNeedsRedraw = true;
    }
    if (Button_isTapped(&hal->boosterpackS2))
    {
         app->state = STATE_MENU; // Return to main menu.
         app->screenNeedsRedraw = true;
         App_Screen_showmainmenu(app, &hal->gfx);
    }
}

// Returns a random color from the color wheel.
uint32_t getRandomColor(ColorWheel* wheel)
{
    int r = rand() % 5;
    switch (r)
    {
         case 0: return wheel->center;
         case 1: return wheel->up;
         case 2: return wheel->down;
         case 3: return wheel->left;
         case 4: return wheel->right;
         default: return wheel->center;
    }
}

// Initializes the floor segments for the game.
void initFloor(Application* app)
{
    int segCount = 0;
    int x = 0;
    // Create floor segments until the screen width is covered.
    while (x < 128)
    {
         app->floorSegments[segCount].x = x;
         app->floorSegments[segCount].width = 125;
         if (segCount == 0)
              app->floorSegments[segCount].color = app->colorWheel.center;
         else
              app->floorSegments[segCount].color = getRandomColor(&app->colorWheel);
         x += 125;
         segCount++;
    }
    app->numFloorSegments = segCount;
}

// Update the floor by scrolling segments to create movement and adding new segments.
void updateFloor(Application* app)
{
    int scrollSpeed = (app->difficulty == 1 ? 2 : (app->difficulty == 2 ? 1 + (app->score / 25) : 1));
    if (scrollSpeed > 4) scrollSpeed = 4;
    int i;
    // Scroll each floor segment to the left.
    for (i = 0; i < app->numFloorSegments; i++)
         app->floorSegments[i].x -= scrollSpeed;
    // Remove floor segments that have scrolled off screen.
    while (app->numFloorSegments > 0 &&
           (app->floorSegments[0].x + app->floorSegments[0].width) <= 0)
    {
         for (i = 1; i < app->numFloorSegments; i++)
              app->floorSegments[i - 1] = app->floorSegments[i];
         app->numFloorSegments--;
    }
    int rightEdge = 0;
    if (app->numFloorSegments > 0)
         rightEdge = app->floorSegments[app->numFloorSegments - 1].x +
                     app->floorSegments[app->numFloorSegments - 1].width;
    // Add a new floor segment if there's space on the right.
    if (rightEdge < 128)
    {
         FloorSegment newSeg;
         newSeg.x = rightEdge;
         newSeg.width = 125;
         newSeg.color = getRandomColor(&app->colorWheel);
         app->floorSegments[app->numFloorSegments] = newSeg;
         app->numFloorSegments++;
    }
}

// Draws the floor by iterating through each floor segment.
void drawFloor(Application* app, GFX* gfx)
{
    const int floorY = 105;
    const int floorHeight = 18;
    int i;
    // Draw each segment of the floor.
    for (i = 0; i < app->numFloorSegments; i++)
    {
         FloorSegment seg = app->floorSegments[i];
         int startX = (seg.x < 0 ? 0 : seg.x);
         int endX = seg.x + seg.width;
         if (endX > 128) endX = 128;
         if (startX < endX)
         {
              GFX_setForeground(gfx, seg.color);
              int x;
              // Draw vertical lines to form a filled rectangle segment.
              for (x = startX; x < endX; x++)
                   GFX_drawLine(gfx, x, floorY, x, floorY + floorHeight - 1);
         }
    }
}
