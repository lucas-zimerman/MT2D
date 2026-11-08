/******************************************************
 MT2D_Dos_Graphics created by Lucas Zimerman Fraulob
 this code start the VGA mode 13h, alloc the memory for
 the double buffer if requested and also goes back to
 the text mode.
*******************************************************/
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>

#ifdef MSDOS_USE_SPRITE_TILE_RENDER
#include <MT2D/_MSDOS/Render/VGAsprTI.h>
SpritesTilesRender SpriteTiles[25][80];/*8KB of initial data ,will expand from size when needed*/
#endif

unsigned char VGA_ASCII_COLORS[2]; /*Stores the foreground and background index to be used by the char in VGA mode*/
unsigned char *VGA = (unsigned char*)0xA0000000L;
#ifdef MSDOS_USE_DOUBLE_BUFFER
unsigned char *DOUBLE_BUFFER;
#ifdef MSDOS_GRAPHICS_SPECIAL_RENDER
unsigned short ChangedMasks[200];
#endif
#endif
unsigned short VGA_MemOffset;

void MSDOS_Call_Functionn(unsigned char ah, unsigned char al){
  union REGS regs;
  regs.h.ah = ah;
  regs.h.al = al;
  int86(0x10,&regs,&regs); /*execute the asm call*/
}

bool MT2D_Dos_VGA_Init(){
  VGA_ASCII_COLORS[0] = 16;  /*background*/
  VGA_ASCII_COLORS[1] = 31;  /*foreground*/
  #ifdef MSDOS_USE_DOUBLE_BUFFER
  DOUBLE_BUFFER = (unsigned char*)malloc(320*200*sizeof(unsigned char));
  #endif
  MSDOS_Call_Functionn(0x00,0x13);
  /* 0x00 function 00h = mode set*/
  /* 0x13 256 color*/

  #ifdef MSDOS_USE_SPRITE_TILE_RENDER
  /*Clear the sprite tile buffer*/
  unsigned char i=0,j=0;
  while(i < 25){
		while(j< 80){
      	SpriteTiles[i][j].Ids =0;
         SpriteTiles[i][j].Count =0;
         SpriteTiles[i][j].DEBUG_OLD_COUNT = 0;
         SpriteTiles[i][j].OldData =0;
         SpriteTiles[i][j].NewData =0;
       	j++;
      }
  		j=0;
      i++;
  }
  #endif

  return true;
}

void MT2D_Dos_VGA_Close(){
  MSDOS_Call_Functionn(0x00,0x03);
  /* 0x00 function 00h = mode set*/
  /* 0x03 text mode*/
}

#endif