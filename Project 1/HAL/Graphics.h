/*
 * Graphics.h - Display drawing functions
 */

#ifndef HAL_GRAPHICS_H_
#define HAL_GRAPHICS_H_

#include <HAL/LcdDriver/Crystalfontz128x128_ST7735.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>

#define FG_COLOR GRAPHICS_COLOR_WHITE
#define BG_COLOR GRAPHICS_COLOR_BLACK
#define X_COLOR GRAPHICS_COLOR_RED
#define S_COLOR GRAPHICS_COLOR_BLUE
#define G_COLOR GRAPHICS_COLOR_GREEN

// Graphics context struct
struct _GFX
{
    Graphics_Context context;
    uint32_t foreground;
    uint32_t background;
    uint32_t defaultForeground;
    uint32_t defaultBackground;
};
typedef struct _GFX GFX;

// Create graphics context
GFX GFX_construct(uint32_t defaultForeground, uint32_t defaultBackground);

// Reset to default colors
void GFX_resetColors(GFX* gfx_p);

// Clear the screen
void GFX_clear(GFX* gfx_p);

// Print text at row and column
void GFX_print(GFX* gfx_p, char* string, int row, int col);

// Set drawing color
void GFX_setForeground(GFX* gfx_p, uint32_t foreground);

// Set background color
void GFX_setBackground(GFX* gfx_p, uint32_t background);

// Draw filled circle
void GFX_drawSolidCircle(GFX* gfx_p, int x, int y, int radius);

// Draw circle outline
void GFX_drawHollowCircle(GFX* gfx_p, int x, int y, int radius);

// Erase filled circle
void GFX_removeSolidCircle(GFX* gfx_p, int x, int y, int radius);

// Erase circle outline
void GFX_removeHollowCircle(GFX* gfx_p, int x, int y, int radius);

// Draw filled rectangle
void GFX_drawSolidRectangle(GFX* gfx_p, int j, int k, int blockWidth, int blockHeight);

#endif /* HAL_GRAPHICS_H_ */
