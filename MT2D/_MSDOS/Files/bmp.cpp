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

void MT2D_Dos_Load_BMP(FILE *file,Sprite *S)
{
  long index;
  word num_colors;
  unsigned char *data;
  long Offset;
  int x;

  /* check to see if it is a valid bitmap file */
/*2 2*/  if (fgetc(file)!='B' || fgetc(file)!='M')  //ID 16 bits
  {
    fclose(file);
    printf("%s is not a bitmap file.\n",file);
    exit(1);
  }

  /* read in the width and height of the image, and the
     number of colors used; ignore the rest */
/*10 8*/  fskip(file,8);/*Skip 8 bytes from the header*/
/*14 4*/  fread(&Offset, sizeof(long), 1, file);

/*22 4*/  fskip(file,4);/*Skip 4 bytes*/
  fread(&S->size.X, sizeof(unsigned short), 1, file);/*read 2 bytes of size X I Width of the bitmap in pixel;*/
  fskip(file,2);                                     /*jump 2 bytes from the rest of the bitmap size*/
  fread(&S->size.Y,sizeof(unsigned short), 1, file);/*read 2 bytes of size Y I Height of the bitmap in pixels*/
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
  /*
    4 4 bytes from 0 important colors
    6 2 bytes from idk
  */

  /* assume we are working with an 8-bit file */
  if (num_colors==0) num_colors=256;

  S->Data = (unsigned char**)malloc(sizeof(unsigned char*));
  S->Data[0] = (unsigned char *) malloc((S->size.X*S->size.Y)*sizeof(unsigned char));
  data = S->Data[0];
  /* Ignore the palette information for now.
     See palette.c for code to read the palette info. */
  //fskip(file,num_colors*4);

  /* jump to where the image is*/
  unsigned char Padding = 4 - (S->size.X%4);
  unsigned int FlippedIndex = S->size.Y*S->size.X - S->size.X;
  fseek(file,Offset,SEEK_SET);
  if(Padding == 4) Padding = 0;
  /* read the bitmap */
  index=0;
  for(int i=0;i<S->size.Y;i++){
    	for(int j=0;j<S->size.X;j++,index++){
      	fread(&data[FlippedIndex + /*S->size.X - */j],sizeof(unsigned char),1,file);
    	}
   	FlippedIndex= FlippedIndex -  S->size.X;
      fskip(file,Padding);
  }
  index =0;
  for(int i=0;i<S->size.Y;i++){
    	for(int j=0;j<S->size.X;j++,index++){
    	}
  }
  fclose(file);
}

#endif