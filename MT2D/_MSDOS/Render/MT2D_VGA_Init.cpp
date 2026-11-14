/******************************************************
 MT2D_Dos_Graphics created by Lucas Zimerman Fraulob
 this code starts the VGA mode 13h, resets the
 renderer and also goes back to the text mode.
*******************************************************/
#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MSDOS_USE_GRAPHICS
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#ifdef __DJGPP__
#include <sys/nearptr.h> /* __djgpp_nearptr_enable, __djgpp_conventional_base */
#endif
#include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>
#include <MT2D/_MSDOS/Render/dVgaRen.h>
#include <MT2D/_MSDOS/MT2D_Dos_Init.h>

unsigned char VGA_ASCII_COLORS[2]; /*Stores the foreground and background index to be used by the char in VGA mode*/
/* 16 bit compilers can use the real mode far pointer, DJGPP is 32 bit protected mode and
   reaches the video memory through the near pointer base (set in MT2D_Dos_VGA_Init). */
#ifdef __DJGPP__
unsigned char *VGA = 0;
#else
unsigned char *VGA = (unsigned char*)0xA0000000L;
#endif
unsigned short VGA_MemOffset;

void MSDOS_Call_Functionn(unsigned char ah, unsigned char al){
  union REGS regs;
  regs.h.ah = ah;
  regs.h.al = al;
  int86(0x10,&regs,&regs); /*execute the asm call*/
}

bool MT2D_Dos_VGA_Init(void){
  #ifdef __DJGPP__
  if (!__djgpp_nearptr_enable()) {
    return false; /* no near pointer access (DPMI host refused) */
  }
  VGA = (unsigned char*)(0xA0000 + __djgpp_conventional_base);
  #endif
  VGA_ASCII_COLORS[0] = 16;  /*background*/
  VGA_ASCII_COLORS[1] = 31;  /*foreground*/
  MSDOS_Call_Functionn(0x00,0x13);
  /* 0x00 function 00h = mode set*/
  /* 0x13 256 color*/

  MT2D_Dos_Render_Invalidate();/*the first render draws every tile*/

  return true;
}

MT2D_Result MT2D_Dos_VGA_Set_Settings(MT2D_Setting_Operation operation, int param1, int param2) {
  return MT2D_Dos_Set_Settings(operation, param1, param2); /* nothing of the VGA mode takes one or two params */
}

MT2D_Result MT2D_Dos_VGA_Set_Settings_Argv(int argc, const int *argv) {
  if (argc < 1 || argv == 0) {
    return MT2D_INVALID;
  }
  if (argv[0] == MT2D_DOS_SET_PALETTE) {
    /* argv: the 256 0xRRGGBB colors, the whole palette */
    if (argc != 1 + 256) {
      return MT2D_BAD_PARAMETER;
    }
    MT2D_Dos_Set_Palette_RGB(0, argv + 1, 256); /* only the low 24 bits of a color are used */
    return MT2D_OK;
  }
  return MT2D_Dos_Set_Settings_Argv(argc, argv); /* audio is shared with the text mode */
}

void MT2D_Dos_VGA_Close(void){
  MT2D_Dos_Render_Free();
  MSDOS_Call_Functionn(0x00,0x03);
  #ifdef __DJGPP__
  __djgpp_nearptr_disable();
  #endif
  /* 0x00 function 00h = mode set*/
  /* 0x03 text mode*/
}

#endif