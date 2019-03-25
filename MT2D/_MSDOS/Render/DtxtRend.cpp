/** MT2D Created By Lucas Zimerman Fraulob
 This is the core of MT2D engine in MS-DOS where it deals the rendering part.
*/

#include <MT2D/MT2D_Terminal_Define.h>

#if defined(__MSDOS__)
#ifdef __DJGPP__ // DJGPP uses another method for writing data on screen
#include <sys/farptr.h> /* _farpokeb() */
#include <go32.h> /* _dos_ds */
#else //Turbo C | Borland C
#include <dos.h>
#endif


#ifdef MSDOS_USE_GRAPHICS
#include <MT2D\_MSDOS\Render\dVgaRen.h>
#endif

#include <MT2D/MT2D.h>


void MT2D_Dos_Clear_Main_Window() {
	unsigned int x, y, offset;
	for (x = 0; x<80; x++) {
		for (y = 0; y <= 24; y++) {//should be 25 but the windows code isn't ready for that
			WINDOW1[y][x] = ' ';
		}
	}
   #ifdef MSDOS_USE_GRAPHICS
   Dos_Clear_ImgBuffer();
   #endif
}

#ifndef MSDOS_USE_GRAPHICS
void MT2D_Dos_Draw_Window(int which) {
	unsigned int x, y, offset;
	if (which == DISPLAY_WINDOW1) {//code that will save directly into video memory
		for (x = 0; x <= MAX_HOR; x++) {
			for (y = 0; y <= MAX_VER; y++) {//should be 25 but the windows code isn't ready for that
				offset = (80 * y + x) * 2;

#ifdef __DJGPP__
				_farpokeb(_dos_ds, 0xB8000 + offset, WINDOW1[y][x]);
#else
				pokeb(0xB800, offset, WINDOW1[y][x]);
#endif
			}
		}
	}
	else {
		for (x = 0; x <= MAX_HOR; x++) {
			for (y = 0; y <= MAX_VER; y++) {//should be 25 but the windows code isn't ready for that
				offset = (80 * y + x) * 2;
#ifdef __DJGPP__
				_farpokeb(_dos_ds, 0xB8000 + offset, WINDOW2[y][x]);
#else
				pokeb(0xB800, offset, WINDOW2[y][x]);
#endif
			}
		}
	}
	//transfer window1 or 2 memory to video memory, also include other systems that have video memory acess here too
	//"ansi" function that will work with any operational system
}
#else

#endif

#ifdef MSDOS_USE_GRAPHICS
void MT2D_Dos_Draw_Window(int which) {
	if (which == DISPLAY_WINDOW1) {//code that will save directly into video memory
        MT2D_Dos_Render_VGA_ASCII(WINDOW1);
	}
	else{
        MT2D_Dos_Render_VGA_ASCII(WINDOW2);
	}
}
#endif

#endif
