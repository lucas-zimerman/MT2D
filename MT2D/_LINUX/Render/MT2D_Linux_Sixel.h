#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Linux_Sixel.h / MT2D_Linux_Sixel.c

Sixel encoder of the screen, used by the Linux terminal backend when
MT2D_TERMINAL_USE_GRAPHICS is defined (see MT2D_Terminal_Define.h).

The frame is drawn on a canvas of palette indexed pixels:
  1. MT2D_Sixel_Draw_Text: every cell (a CP850 byte) is drawn with an 8x16 bitmap
     font, so the 80x24 screen is a 640x384 image, light gray on black.
  2. MT2D_Sixel_Draw_Image: image sprites are drawn over the text (index 0 of the
     image is transparent), their colors are added to the palette of the canvas.
  3. MT2D_Sixel_Encode: the canvas becomes a Sixel string (up to 256 colors).
This file does not depend on curses nor on the terminal.
===================================================================*/
#ifndef MT2DLIN_SIXEL_H
#define MT2DLIN_SIXEL_H

#define MT2D_SIXEL_CELL_W 8
#define MT2D_SIXEL_CELL_H 16
#define MT2D_SIXEL_COLORS 256

typedef struct MT2D_Sixel_Canvas {
	unsigned char *Pixels;                     /* Width * Height palette indexes, row major, owned by the caller */
	int Width;
	int Height;
	int Colors;                                /* used entries of Palette, index 0 is the background */
	unsigned char Palette[MT2D_SIXEL_COLORS][3]; /* RGB 0..255 */
} MT2D_Sixel_Canvas;

MT2D_C_BEGIN
/* Sets the canvas size to cols x rows cells, clears it (index 0 = black) and draws the cells.
   Pixels must have room for cols*8 * rows*16 bytes. Index 1 is the color of the text. */
void MT2D_Sixel_Draw_Text(MT2D_Sixel_Canvas *canvas, unsigned char *pixels, const unsigned char *cells, int cols, int rows);

/* Index of a color in the palette (it is added when new). When the palette is full the nearest color is used. */
int MT2D_Sixel_Add_Color(MT2D_Sixel_Canvas *canvas, int r, int g, int b);

/* Draws a srcW x srcH image (palette indexed, row major, index 0 transparent, palette RGB 0..255) over the canvas,
   stretched (nearest pixel) to the w x h rectangle at (x,y), clipped to the canvas. */
void MT2D_Sixel_Draw_Image(MT2D_Sixel_Canvas *canvas, const unsigned char *src, const unsigned char (*palette)[3], int colors,
	int srcW, int srcH, int x, int y, int w, int h);

/* Encodes the canvas as a complete Sixel sequence (ESC P ... ESC \), '\0' terminated.
   Returns the length without the '\0', or 0 when outSize is too small. */
unsigned int MT2D_Sixel_Encode(const MT2D_Sixel_Canvas *canvas, char *out, unsigned int outSize);

/* Reads a reply to the "primary device attributes" query (ESC [ c): "ESC [ ? 62 ; 4 ; 22 c". Returns 1 when the
   list has the attribute 4 (the terminal can draw Sixel), 0 for anything else (also a NULL or truncated reply). */
int MT2D_Sixel_Reply_Supports_Sixel(const char *reply);
MT2D_C_END

#endif
