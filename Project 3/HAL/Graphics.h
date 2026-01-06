/*
 * Graphics.h
 *
 * Simple graphics API for the 128x128 LCD display.
 * Wraps TI's grlib with easier-to-use functions for text, shapes, and colors.
 */

#ifndef HAL_GRAPHICS_H_
#define HAL_GRAPHICS_H_

#include <HAL/LcdDriver/Crystalfontz128x128_ST7735.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>

#define FG_COLOR GRAPHICS_COLOR_WHITE
#define BG_COLOR GRAPHICS_COLOR_BLACK

// Graphics context that tracks colors and display state
struct _GFX
{
    Graphics_Context context;
    uint32_t foreground;
    uint32_t background;
    uint32_t defaultForeground;
    uint32_t defaultBackground;
};
typedef struct _GFX GFX;

// Initialize the LCD and create a graphics context
GFX GFX_construct(uint32_t defaultForeground, uint32_t defaultBackground);

// Reset colors to defaults
void GFX_resetColors(GFX* gfx_p);

// Clear the entire screen
void GFX_clear(GFX* gfx_p);

// Draw text at row/column position (character grid, not pixels)
void GFX_print(GFX* gfx_p, char* string, float row, float col);

// Erase text by drawing it in background color
void GFX_eraseText(GFX* gfx_p, char* string, float row, float col);

// Print multiple lines of text, returns next available row
int GFX_printTextRows(GFX* gfx_p, char* strings[], int numStrings, float firstRow, float col);

// Change drawing colors
void GFX_setForeground(GFX* gfx_p, uint32_t foreground);
void GFX_setBackground(GFX* gfx_p, uint32_t background);

// Circle drawing
void GFX_drawSolidCircle(GFX* gfx_p, int x, int y, int radius);
void GFX_drawHollowCircle(GFX* gfx_p, int x, int y, int radius);
void GFX_removeSolidCircle(GFX* gfx_p, int x, int y, int radius);
void GFX_removeHollowCircle(GFX* gfx_p, int x, int y, int radius);

// Rectangle drawing (coordinates are pixel positions)
void GFX_drawSolidRectangle(GFX* gfx_p, int x1, int x2, int y1, int y2);
void GFX_drawHollowRectangle(GFX* gfx_p, int x1, int x2, int y1, int y2);
void GFX_removeSolidRectangle(GFX* gfx_p, int x1, int x2, int y1, int y2);

// Line drawing
void GFX_drawLine(GFX* gfx_p, int x1, int y1, int x2, int y2);

#endif /* HAL_GRAPHICS_H_ */
