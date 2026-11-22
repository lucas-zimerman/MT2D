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
                Description: Takes a screen buffer (WINDOW1 when NULL) and put it on the terminal or screen memory
                Return: none
        -goto_topscreen()
                Description: Move the terminal blink position to the top of the screen (WINDOWS ONLY)
                Return: none
        -change_cursor_position()
                Description: Change the cursor position to any valid place (WINDOWS ONLY)
                Return: none
        -create_window_layout()
                Description: Draw a Window with specific position, size and others elements on it on a screen buffer (WINDOW1 when NULL)
                Buffer.
                Return: none
        -MT2D_Window_Save_Area() / MT2D_Window_Restore_Area()
                Description: Save a rectangle of a screen buffer into a buffer and put it back later (used by the popups)
                Return: none
        -insert_string_on_display()
                Description: Write a string on a specific location of a screen buffer (WINDOW1 when NULL).
                Return: none
        -insert_number_on_display()
                Description: Write a number on a specific location of a screen buffer (WINDOW1 when NULL, integer only)
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

#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/MT2D_CDecl.h> /* C linkage macros and "bool" */
#include <MT2D/MT2D_Window.h> /* WINDOW1, MT2D_Window, MAX_HOR, MAX_VER, DISPLAY_WINDOW1 */

#ifdef SDL_USE /* SDL2.lib SDL2main.lib SDL2_image.lib SDL2_ttf.lib */
        #ifdef WINDOWS_TARGET
        #include <SDL.h>
        #elif defined(linux) || defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    #include <SDL2/SDL.h>
        #elif defined(linux) && defined(__ANDROID__)
    #include <SDL.h>
    #endif
#endif

#define MT2D_WINDOWED_MODE /* Can be disabled : set the initial window size, if not defines it starts in fullscreen mode */

MT2D_C_BEGIN

/* window: the buffer to use, NULL (DISPLAY_WINDOW1) for WINDOW1 */
void MT2D_Window_Save_Area(unsigned char *buffer, int x, int y, int w, int h, unsigned char (*window)[MAX_HOR]);
void MT2D_Window_Restore_Area(unsigned char *buffer, int x, int y, int w, int h, unsigned char (*window)[MAX_HOR]);
void insert_string_on_display(char *x, int pos_ver, int pos_hor, unsigned char (*window)[MAX_HOR]);
void insert_number_on_display(int number, int pos_y,int pos_x, unsigned char (*window)[MAX_HOR]);
MT2D_C_END

#ifdef WINDOWS_DESKTOP_TARGET
        #ifdef SDL_USE
        #undef main
        #pragma comment(lib, "SDL2.lib")
        #pragma comment(lib, "SDL2main.lib")
        #pragma comment(lib, "SDL2_image.lib")
/* #pragma comment(lib, "SDL2_ttf.lib") */
        #pragma comment(lib, "SDL2_mixer.lib")
        #endif
#endif

/*OS SPECIFIC CODE*/
#if defined(SDL_USE)

/* #undef main */
        #include <MT2D/SDL/MT2D_SDL_main.h>
        #define MT2D_Init MT2D_SDL_Init
#elif defined(_WIN32)
        #include "_WINDOWS/MT2D_Win_Init.h"
        #define MT2D_Init MT2D_Win_Init
#elif defined(MSDOS_USE_GRAPHICS) 
	#include <MT2D/_MSDOS/Render/MT2D_VGA_Init.h>
	#define MT2D_Init MT2D_Dos_VGA_Init 
#elif defined(__MSDOS__) || defined(__DOS__)
        #include <MT2D/_MSDOS/MT2D_Dos_Init.h>
        #define MT2D_Init MT2D_Dos_Init
#elif defined(linux)
        #include "_LINUX/MT2D_Linux_Init.h"
        #define MT2D_Init MT2D_Linux_Init
#elif defined(__EMSCRIPTEN__) && !defined(SDL_USE)
#error "MT2D must use SDL in order to work with EMSCRIPTEN, please enable it in MT2D_Terminal_Define.h"
#else
#error "MT2D doesn't have code to support this operational system."
#endif

#endif
