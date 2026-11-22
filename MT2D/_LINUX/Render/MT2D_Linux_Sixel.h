#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Linux_Sixel.h / MT2D_Linux_Sixel.c

Sixel encoder of the screen, used by the Linux terminal backend when
MT2D_TERMINAL_USE_GRAPHICS is defined (see MT2D_Terminal_Define.h).

Like the MS-DOS VGA mode, the screen has one palette of 256 colors and everything is drawn with palette indexes:
  * the palette is global. MT2D_Init gives it the VGA default (MT2D_Sixel_Default_Palette) through the
    MT2D_SET_PALETTE setting, a program replaces it with that setting (all 256 colors) and loading a BMP
    sets it too (the palette of the file), the same way the MS-DOS VGA mode programs the DAC. It is never built on the fly.
  * MT2D_Sixel_Draw_Text draws every cell (a CP850 byte) with an 8x16 bitmap font, so the 80x24 screen is a 640x384 image.
  * MT2D_Sixel_Draw_Image draws image sprites over it, their pixels are palette indexes (0 is transparent).
  * MT2D_Sixel_Encode turns the canvas into a Sixel string, defining only the palette colors the canvas uses.
This file does not depend on curses nor on the terminal.
===================================================================*/
#ifndef MT2DLIN_SIXEL_H
#define MT2DLIN_SIXEL_H

#define MT2D_SIXEL_CELL_W 8
#define MT2D_SIXEL_CELL_H 16
#define MT2D_SIXEL_COLORS 256

typedef struct MT2D_Sixel_Canvas {
	unsigned char *Pixels;                       /* Width * Height palette indexes, row major, owned by the caller */
	int Width;
	int Height;
	unsigned char Palette[MT2D_SIXEL_COLORS][3]; /* RGB 0..255, a copy of the global palette made by MT2D_Sixel_Draw_Text */
} MT2D_Sixel_Canvas;

MT2D_C_BEGIN
/* The default palette of the VGA mode 13h (0..15 the EGA colors, 16..31 grays, 32..247 the color ramps, 248..255 black)
   as 256 colors 0xRRGGBB. The text colors (16 black, 31 white) and the transparent index 0 of the sprites follow MS-DOS. */
void MT2D_Sixel_Default_Palette(int rgb[MT2D_SIXEL_COLORS]);

/* Sets the global palette. Until it is set (MT2D_Init does it) every color is black. */
void MT2D_Sixel_Palette_Set_Rgb(int first, const int *rgb, int count); /* count colors 0xRRGGBB from index first */
void MT2D_Sixel_Palette_Set_Bgra(const unsigned char *bgra, int count); /* a BMP palette (B,G,R,0) from index 0 */

/* The 3 bytes (R,G,B) of a palette color, or NULL when index is out of range. */
const unsigned char *MT2D_Sixel_Palette_Entry(int index);

/* Sets the canvas size to cols x rows cells, copies the palette, fills the canvas with the index bg and draws the cells
   with the index fg. Pixels must have room for cols*8 * rows*16 bytes. */
void MT2D_Sixel_Draw_Text(MT2D_Sixel_Canvas *canvas, unsigned char *pixels, const unsigned char *cells, int cols, int rows,
	int bg, int fg);

/* Draws a srcW x srcH image (palette indexes, row major, index 0 transparent) over the canvas, stretched (nearest pixel)
   to the w x h rectangle at (x,y), clipped to the canvas. */
void MT2D_Sixel_Draw_Image(MT2D_Sixel_Canvas *canvas, const unsigned char *src, int srcW, int srcH, int x, int y, int w, int h);

/* Encodes the canvas as a complete Sixel sequence (ESC P ... ESC \), '\0' terminated.
   Returns the length without the '\0', or 0 when outSize is too small. */
unsigned int MT2D_Sixel_Encode(const MT2D_Sixel_Canvas *canvas, char *out, unsigned int outSize);

/* Reads a reply to the "primary device attributes" query (ESC [ c): "ESC [ ? 62 ; 4 ; 22 c". Returns 1 when the
   list has the attribute 4 (the terminal can draw Sixel), 0 for anything else (also a NULL or truncated reply). */
int MT2D_Sixel_Reply_Supports_Sixel(const char *reply);
MT2D_C_END

#endif
