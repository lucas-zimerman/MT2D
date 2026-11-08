/**************************************************************************
 * bitmap.c                                                               *
 * written by David Brackeen                                              *
 * http://www.brackeen.com/home/vga/                                      *
 *                                                                        *
 * This is a 16-bit program.                                              *
 * Tab stops are set to 2.                                                *
 * Remember to compile in the LARGE memory model!                         *
 * To compile in Borland C: bcc -ml bitmap.c                              *
 *                                                                        *
 * This program will only work on DOS- or Windows-based systems with a    *
 * VGA, SuperVGA or compatible video adapter.                             *
 *                                                                        *
 * Please feel free to copy this source code.                             *
 *                                                                        *
 * DESCRIPTION: This program demostrates drawing bitmaps, including       *
 * transparent bitmaps.                                                   *
 **************************************************************************/
 /*
   	MT2D_Dos_BMP.cpp, created by: Lucas Zimerman Fraulob
      this specific code was adapted to be integrated with the MT2D engine
      parts from the original author that weren't neeeded were deleted.
 */
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>
#include <MT2D/ObjectCore/Sprites.h>
#include <MT2D/_MSDOS/Render/DosImg.h>
#include <MT2D/_MSDOS/Files/bmp.h>
#include <MT2D/_MSDOS/Render/dVgaRen.h>
#include <string.h>

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;


extern unsigned char *VGA;
extern unsigned short VGA_MemOffset;


typedef struct tagBITMAP              /* the structure for a bitmap. */
{
  word width;
  word height;
  byte *data;
} BITMAP;

/**************************************************************************
 *  fskip                                                                 *
 *     Skips bytes in a file.                                             *
 **************************************************************************/

void fskip(FILE *fp, int num_bytes)
{
   int i;
   for (i=0; i<num_bytes; i++)
      fgetc(fp);
}

/**************************************************************************
 *  load_bmp                                                              *
 *    Loads a bitmap file into memory.                                    *
 **************************************************************************/

static unsigned long Le32(const unsigned char *b) {
  return (unsigned long)b[0] | ((unsigned long)b[1] << 8) | ((unsigned long)b[2] << 16) | ((unsigned long)b[3] << 24);
}
static unsigned int Le16(const unsigned char *b) {
  return (unsigned int)b[0] | ((unsigned int)b[1] << 8);
}

/* Loads an uncompressed 8 bit BMP: the pixels become Image->Pixels (top row first),
   the palette is sent to the VGA DAC (index 0 is the transparent color of the sprites). */
int MT2D_Dos_Load_BMP(FILE *file,Sprite *S)
{
  unsigned char head[54];
  unsigned char pal[1024];
  unsigned long Offset, DibSize;
  long Width, Height;
  unsigned int Colors;
  unsigned char *data;
  unsigned long Padding;
  unsigned long FlippedIndex;
  long i;

  if (fread(head, 1, sizeof(head), file) != sizeof(head)) return 0;
  if (head[0] != 'B' || head[1] != 'M') return 0;
  Offset = Le32(head + 10);
  DibSize = Le32(head + 14);
  Width = (long)Le32(head + 18);
  Height = (long)Le32(head + 22);
  if (DibSize < 40 || Le16(head + 28) != 8 || Le32(head + 30) != 0) return 0; /* 8 bpp, BI_RGB only */
  if (Width <= 0 || Width > 32767 || Height <= 0 || Height > 32767) return 0;
  Colors = (unsigned int)Le32(head + 46);
  if (Colors == 0 || Colors > 256) Colors = 256;

  /* the palette is stored as B,G,R,0 right after the headers */
  fseek(file, 14L + (long)DibSize, SEEK_SET);
  memset(pal, 0, sizeof(pal));
  if (fread(pal, 4, Colors, file) != Colors) return 0;
  MT2D_Dos_Set_Palette(pal, (int)Colors);

  S->Image = (MT2D_Internal_Image*)malloc(sizeof(MT2D_Internal_Image));
  if (!S->Image) return 0;
  S->Image->Pixels = (unsigned char *) malloc((size_t)Width*Height);
  if (!S->Image->Pixels) {
    free(S->Image);
    S->Image = 0;
    return 0;
  }
  S->SizeX = (int)Width;
  S->SizeY = (int)Height;
  S->type = 1;
  data = S->Image->Pixels;

  /* rows are stored bottom up and padded to 4 bytes */
  Padding = (4UL - ((unsigned long)Width % 4UL)) % 4UL;
  FlippedIndex = (unsigned long)(Height - 1) * (unsigned long)Width;
  fseek(file, (long)Offset, SEEK_SET);
  for (i = 0; i < Height; i++) {
    if (fread(&data[FlippedIndex], 1, (size_t)Width, file) != (size_t)Width) {
      free(S->Image->Pixels);
      free(S->Image);
      S->Image = 0;
      return 0;
    }
    FlippedIndex -= (unsigned long)Width;
    fskip(file, (int)Padding);
  }
  return 1;
}

#endif