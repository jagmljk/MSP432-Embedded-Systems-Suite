/*
 * Graphics.c - Display drawing functions
 */

#include <HAL/Graphics.h>

// Initialize display and graphics context
GFX GFX_construct(uint32_t defaultForeground, uint32_t defaultBackground)
{
    GFX gfx;

    gfx.defaultForeground = defaultForeground;
    gfx.defaultBackground = defaultBackground;

    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    Graphics_initContext(&gfx.context, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
    Graphics_setFont(&gfx.context, &g_sFontFixed6x8);

    GFX_resetColors(&gfx);
    GFX_clear(&gfx);

    return gfx;
}

// Reset to default colors
void GFX_resetColors(GFX* gfx_p)
{
    gfx_p->foreground = gfx_p->defaultForeground;
    gfx_p->background = gfx_p->defaultBackground;

    Graphics_setForegroundColor(&gfx_p->context, gfx_p->defaultForeground);
    Graphics_setBackgroundColor(&gfx_p->context, gfx_p->defaultBackground);
}

// Clear the screen
void GFX_clear(GFX* gfx_p)
{
    Graphics_clearDisplay(&gfx_p->context);
}

// Print text at row and column
void GFX_print(GFX* gfx_p, char* string, int row, int col)
{
    int yPosition = row * Graphics_getFontHeight(gfx_p->context.font);
    int xPosition = col * Graphics_getFontMaxWidth(gfx_p->context.font);

    Graphics_drawString(&gfx_p->context, (int8_t*) string, -1, xPosition, yPosition, OPAQUE_TEXT);
}

// Set foreground color
void GFX_setForeground(GFX* gfx_p, uint32_t foreground)
{
    gfx_p->foreground = foreground;
    Graphics_setForegroundColor(&gfx_p->context, foreground);
}

// Set background color
void GFX_setBackground(GFX* gfx_p, uint32_t background)
{
    gfx_p->background = background;
    Graphics_setBackgroundColor(&gfx_p->context, background);
}

// Draw filled circle
void GFX_drawSolidCircle(GFX* gfx_p, int x, int y, int radius)
{
    Graphics_fillCircle(&gfx_p->context, x, y, radius);
}

// Draw circle outline
void GFX_drawHollowCircle(GFX* gfx_p, int x, int y, int radius)
{
    Graphics_drawCircle(&gfx_p->context, x, y, radius);
}

// Erase filled circle by drawing with background color
void GFX_removeSolidCircle(GFX* gfx_p, int x, int y, int radius)
{
    uint32_t oldForegroundColor = gfx_p->foreground;
    GFX_setForeground(gfx_p, gfx_p->background);
    GFX_drawSolidCircle(gfx_p, x, y, radius);
    GFX_setForeground(gfx_p, oldForegroundColor);
}

// Erase circle outline by drawing with background color
void GFX_removeHollowCircle(GFX* gfx_p, int x, int y, int radius)
{
    uint32_t oldForegroundColor = gfx_p->foreground;
    GFX_setForeground(gfx_p, gfx_p->background);
    GFX_drawHollowCircle(gfx_p, x, y, radius);
    GFX_setForeground(gfx_p, oldForegroundColor);
}

// Draw filled rectangle
void GFX_drawSolidRectangle(GFX* gfx_p, int j, int k, int blockWidth, int blockHeight)
{
    Graphics_Rectangle rec;
    rec.xMin = j;
    rec.yMin = k;
    rec.xMax = j + blockWidth;
    rec.yMax = k + blockHeight;

    Graphics_fillRectangle(&gfx_p->context, &rec);
}

// Display move count on screen
void GFX_displayMoveCount(GFX* gfx_p, int moveCount) {
    char movesText[20];
    GFX_setForeground(gfx_p, FG_COLOR);
    snprintf(movesText, sizeof(movesText), "Moves: %d", moveCount);
    GFX_print(gfx_p, movesText, 2, 3);
}
