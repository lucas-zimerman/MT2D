/*===================================================================
Window_core.h/ Window_core.cpp created By Lucas Zimerman Fraulob

This file will contain the basic MT2D windows display system core

	List of functions and what they do and their returns:
	-bool MT2D_Init()
		Start the inital MT2D struct (mostly start each specific os/api struct)
	-clear_display()
		Description: Clears the WINDOW1 buffer, not the display, it's not the best name and will be replaced
		on the future.
		Return: none
	-Print_Display()
		Description: Takes all the video buffer on WINDOW1 or WINDOW2 and put it on the terminal or screen memory
		Return: none
	-goto_topscreen()
		Description: Move the terminal blink position to the top of the screen (WINDOWS ONLY)
		Return: none
	-change_cursor_position()
		Description: Change the cursor position to any valid place (WINDOWS ONLY)
		Return: none
	-create_window_layout()
		Description: Draw a Window with specific position, size and others elements on it on WINDOW1 or WINDOW2
		Buffer.
		Return: none
	-transfer_window1_to_window2()
		Description: Transfer the WINDOW1 buffer to WINDOW2
		Return: none
	-transfer_window2_to_window1()
		Description: Transfer the WINDOW2 buffer to WINDOW1
		Return: none
	-insert_string_on_display()
		Description: Write a string on a specific location of WINDOW1 or WINDOW2.
		Return: none
	-insert_number_on_display()
		Description: Write a number on a specific location of WINDOW1 or WINDOW2 (integer only)
		Return: none
======================================================================*/

/** === HOW TO MT2D WITH LINUX: (tested with ubuntu)
on terminal:
sudo apt-cache search libsdl2-image
sudo apt-cache search libsdl2-dev
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-mixer-dev

linker settings: (In case of codeblocks: project -> build options -> linker settings -> other linker settings
-lSDL2
-lSDL2_image
-lSDL2_mixer

*/
#ifndef MT2D_HEADER
#define MT2D_HEADER


#include "MT2D_Terminal_Define.h"

#ifdef SDL_USE //SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib
	#ifdef WINDOWS_TARGET
	#include <SDL.h>
	#elif defined(linux) && !defined(__ANDROID__)
    #include <SDL2/SDL.h>
	#elif defined(linux) && defined(__ANDROID__)
    #include <SDL.h>
    #endif
#endif

//#include <stdbool.h>

#define DISPLAY_WINDOW1 1
#define DISPLAY_WINDOW2 0
#define MAX_HOR 80
#define MAX_VER 25
#define MT2D_WINDOWED_MODE // Can be disabled : set the initial window size, if not defines it starts in fullscreen mode

extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER + 1][MAX_HOR];

/*OS SPECIFIC CODE*/
#if defined(SDL_USE)
//#undef main
//#include "SDL/MT2D_SDL_main.h"
//#define MT2D_Init MT2D_SDL_Init
#elif defined(_WIN32)
#include "_WINDOWS\MT2D_Win_Init.h"
#define MT2D_Init MT2D_Win_Init
#elif defined(__MSDOS__)
#include "_MSDOS\MT2D_Dos_Init.h"
#define MT2D_Init MT2D_Dos_Init
#elif defined(linux)
#include "_LINUX\MT2D_Linux_Init.h"
#define MT2D_Init MT2D_Linux_Init
#else
#error "MT2D doesn't have code to support this operational system."
#endif

bool MT2D_Init();
void transfer_window1_to_window2();
void transfer_window2_to_window1();
void insert_string_on_display(char *x, int pos_ver, int pos_hor, bool which);
void insert_number_on_display(int number, int pos_y,int pos_x, bool display);


#endif
