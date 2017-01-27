#include <stdio.h>

#ifndef __MSDOS__
	#include "MT2D/MT2D_Terminal_Define.h"
#endif

#ifdef SDL_USE
//   #include "SDL_MT2D/SDL_Defines.h"
#include "SDL_MT2D/SDL_MT2Dmain.h"
#include "MT2D.h"
#elif defined _WIN32
#include <Windows.h>
#elif defined __MSDOS__
#include <dos.h>
#elif defined linux
#include <cstdlib>
#include <curses.h>
#include <signal.h>
#include <locale.h>
#endif


void MT2D_Cursor_goto_topscreen() {
#ifdef SDL_USE
#elif defined _WIN32
	COORD coord;
	coord.X = 0;
	coord.Y = 0;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#elif defined  __MSDOS__
	//no command for that yet
#endif
}

void MT2D_Cursor_Set_Position(int line, int column) {
#ifdef _WIN32//WINDOWS FUNCTION TO CHANGE THE CURSOR POSITION
#ifdef SDL_USE // no need for this code
#else
	COORD coord;
	coord.X = column;
	coord.Y = line;
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE), coord);
#endif
#else
	//no ansi code found...
#endif
}