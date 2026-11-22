/* The text screen buffer: WINDOW1 and the type of the buffers the display functions draw on. */
#ifndef MT2D_WINDOW_H
#define MT2D_WINDOW_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

#define MAX_HOR 80
#define MAX_VER 25

/* A screen buffer, [row][column], one CP850 character per cell. Declare your own to draw off screen:
       MT2D_Window my_window;
       insert_string_on_display((char *)"Hi", 0, 0, my_window);
   The display functions take a pointer to its rows (unsigned char (*)[MAX_HOR]), NULL means WINDOW1. */
typedef unsigned char MT2D_Window[MAX_VER + 1][MAX_HOR];

/* NULL: the "window" argument of the display functions for the main screen (WINDOW1). */
#define DISPLAY_WINDOW1 0

/* The window argument, or WINDOW1 when it is NULL. */
#define MT2D_WINDOW_OR_MAIN(window) ((window) ? (window) : WINDOW1)

MT2D_C_BEGIN
extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
MT2D_C_END

#endif
