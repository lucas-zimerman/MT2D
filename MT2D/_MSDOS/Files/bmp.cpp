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

int MT2D_Dos_Load_BMP(FILE *file,Sprite *S)
{
  word num_colors;
  unsigned char *data;
  long Offset = 0;
  unsigned short Width = 0, Height = 0;
  unsigned char Padding;
  unsigned int FlippedIndex;
  int i, j;

  /* check to see if it is a valid bitmap file */
  if (fgetc(file)!='B' || fgetc(file)!='M')  /* ID 16 bits */
  {
    return 0;
  }

  /* read in the width and height of the image, and the
     number of colors used; ignore the rest */
  fskip(file,8);/*Skip 8 bytes from the header*/
  fread(&Offset, sizeof(long), 1, file);

  fskip(file,4);/*Skip 4 bytes*/
  fread(&Width, sizeof(unsigned short), 1, file);/*Width of the bitmap in pixels*/
  fskip(file,2);                                 /*jump 2 bytes from the rest of the bitmap size*/
  fread(&Height,sizeof(unsigned short), 1, file);/*Height of the bitmap in pixels*/
  fskip(file,22);
  /*
    2  2 bytes from the rest of the Y size
    4  2 bytes from  the number of color planes
    8  4 bytes from BI_RGB
    12 4 bytes from The size of the raw bitmap data
    16 4 bytes from the print resolution of the image in horizontal
    20 4 bytes from the print resolution of the image in vertical
    22 2 bytes from the number of colors in the palette (2 bytes left)
  */
  fread(&num_colors,sizeof(unsigned short), 1, file);
  fskip(file,6);
  if (Width == 0 || Height == 0) return 0;

  /* assume we are working with an 8-bit file */
  if (num_colors==0) num_colors=256;

  S->Image = (MT2D_Internal_Image*)malloc(sizeof(MT2D_Internal_Image));
  if (!S->Image) return 0;
  S->Image->Pixels = (unsigned char *) malloc((size_t)Width*Height);
  if (!S->Image->Pixels) {
    free(S->Image);
    S->Image = 0;
    return 0;
  }
  S->SizeX = Width;
  S->SizeY = Height;
  S->type = 1;
  data = S->Image->Pixels;
  /* Ignore the palette information for now. */

  /* jump to where the image is*/
  Padding = (unsigned char)((4 - (Width%4)) % 4);
  FlippedIndex = (unsigned int)Height*Width - Width;
  fseek(file,Offset,SEEK_SET);
  /* read the bitmap (BMP rows are stored bottom up) */
  for(i=0;i<Height;i++){
    for(j=0;j<Width;j++){
      fread(&data[FlippedIndex + j],sizeof(unsigned char),1,file);
    }
    FlippedIndex -= Width;
    fskip(file,Padding);
  }
  return 1;
}

#endif