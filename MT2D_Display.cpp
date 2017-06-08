// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.

#include <stdio.h>

#ifndef __MSDOS__
#include "MT2D/MT2D_Terminal_Define.h"
#endif

#ifdef SDL_USE
#include <MT2D/Render_Terminal/SDL_RENDER/MT2D_SDL_main.h>
#elif defined _WIN32
#include <Windows.h>
#elif defined __MSDOS__
#ifdef __DJGPP__ // DJGPP uses another method for writing data on screen
#include <sys/farptr.h> /* _farpokeb() */
#include <go32.h> /* _dos_ds */
#else //Turbo C | Borland C
#include <dos.h>
#endif
#elif defined linux
#include <cstdlib>
#include <curses.h>
#include <signal.h>
#include <locale.h>
#endif

#include <MT2D/MT2D.h>

#if defined(_WIN32) && !defined(SDL_USE) //starting data for windows only and, if windows mode, only load the terminal stuff if not using SDL
SMALL_RECT windowSize = { 0, 0, MAX_HOR - 1, MAX_VER - 1 };
COORD bufferSize = { MAX_HOR, MAX_VER };
COORD characterBufferSize = { MAX_HOR, MAX_VER };
COORD characterPosition = { 0, 0 };
SMALL_RECT consoleWriteArea = { 0, 0, MAX_HOR - 1, MAX_VER - 1 };
CHAR_INFO consoleBuffer[MAX_HOR * MAX_VER];
HANDLE wHnd; /* write (output) handle */
HANDLE rHnd; /* read (input handle */
#elif defined linux
static void finish(int sig);
#endif


void MT2D_Clear_Main_Window() {
#ifdef __MSDOS__
	unsigned int x, y, offset;
	for (x = 0; x<80; x++) {
		for (y = 0; y <= 24; y++) {//should be 25 but the windows code isn't ready for that
			WINDOW1[y][x] = ' ';
		}
	}
#elif defined _WIN32
	int i = 0, j = 0;
	while (i <= MAX_VER) {
		while (j<MAX_HOR) {
			WINDOW1[i][j] = ' ';
			j++;
		}
		i++;
		j = 0;
	}
	WINDOW1[MAX_VER][MAX_HOR] = '\0';
	//#endif
#elif defined linux
	int i = 0, j = 0;
	while (i <= MAX_VER) {
		while (j<MAX_HOR) {
			WINDOW1[i][j] = ' ';
			j++;
		}
		i++;
		j = 0;
	}
#endif
}


void MT2D_Draw_Window(int which) {
#ifdef SDL_USE
	int i = 0;
	int j = 0;
	//	Clean_Render();
	if (which == DISPLAY_WINDOW1) {
		Render_New(WINDOW1);
	}
	else {
		Render_New(WINDOW2);
	}
	SDL_Render();

#elif defined __MSDOS__
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
#elif defined  _WIN32
	int x, y;
	bool buffer_started = 0;

	if (!buffer_started) {
		wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
		rHnd = GetStdHandle(STD_INPUT_HANDLE);
		SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
		SetConsoleScreenBufferSize(wHnd, bufferSize);
		buffer_started = true;
	}
	if (which == DISPLAY_WINDOW1) {
		for (y = 0; y < MAX_VER; ++y)
		{
			for (x = 0; x < MAX_HOR; ++x)
			{
				consoleBuffer[x + MAX_HOR * y].Char.AsciiChar = (unsigned char)WINDOW1[y][x];//insere o caracter
				consoleBuffer[x + MAX_HOR * y].Attributes = 7;//rand() % 255;//cor do caracter + cor de fundo
			}
		}
	}
	else {
		for (y = 0; y < MAX_VER; ++y)
		{
			for (x = 0; x < MAX_HOR; ++x)
			{
				consoleBuffer[x + MAX_HOR * y].Char.AsciiChar = (unsigned char)WINDOW2[y][x];//insere o caracter
				consoleBuffer[x + MAX_HOR * y].Attributes = 7;//rand() % 255;//cor do caracter + cor de fundo
			}
		}
	}
	WriteConsoleOutputA(wHnd, consoleBuffer, characterBufferSize, characterPosition, &consoleWriteArea);//troca o buffer do console para o novo buffer
#elif defined  linux
	int x, y;
	if (!ncurses_started) {
		(void)signal(SIGINT, finish);
		(void)initscr();
		keypad(stdscr, TRUE);
		(void)nonl();
		(void)cbreak();
		(void)echo();
		setlocale(LC_ALL, "");
		ncurses_started = true;
	}

	if (which == DISPLAY_WINDOW1) {
		for (y = 0; y<24; y++) {
			for (x = 0; x<80; x++) {
				move(y, x);
				if (WINDOW1[y][x] <= 127)
					addch(WINDOW1[y][x]);
				else {// no codepage 850 ;--;
					if (WINDOW1[y][x] >= 176 && WINDOW1[y][x] <= 178)
						addch(ACS_CKBOARD);
					else if (WINDOW1[y][x] == 106 || WINDOW1[y][x] == 205)
						addch(ACS_HLINE);
					else if (WINDOW1[y][x] == 179 || WINDOW1[y][x] == 186)
						addch(ACS_VLINE);
					else if (WINDOW1[y][x] >= 219 && WINDOW1[y][x] <= 223)
						addch(ACS_CKBOARD);
					else if (WINDOW1[y][x] == 254)
						addch(ACS_CKBOARD);
					else if (WINDOW1[y][x] == 192 || WINDOW1[y][x] == 200)
						addch(ACS_LLCORNER);
					else if (WINDOW1[y][x] == 201 || WINDOW1[y][x] == 218)
						addch(ACS_ULCORNER);
					else if (WINDOW1[y][x] == 187 || WINDOW1[y][x] == 191)
						addch(ACS_URCORNER);
					else if (WINDOW1[y][x] == 188 || WINDOW1[y][x] == 217)
						addch(ACS_LRCORNER);
					else if (WINDOW1[y][x] == 204 || WINDOW1[y][x] == 195)
						addch(ACS_LTEE);
					else if (WINDOW1[y][x] == 180 || WINDOW1[y][x] == 185)
						addch(ACS_RTEE);
					else if (WINDOW1[y][x] == 193 || WINDOW1[y][x] == 202)
						addch(ACS_BTEE);
					else if (WINDOW1[y][x] == 194 || WINDOW1[y][x] == 203)
						addch(ACS_TTEE);
					else if (WINDOW1[y][x] == 197 || WINDOW1[y][x] == 206)
						addch(ACS_PLUS);
					else if (WINDOW1[y][x] == 167)
						addch(ACS_DEGREE);
					else if (WINDOW1[y][x] == 241)
						addch(ACS_PLMINUS);
					else if (WINDOW1[y][x] == 249 || WINDOW1[y][x] == 250)
						addch(ACS_BULLET);
					else if (WINDOW1[y][x] == 243)
						addch(ACS_LEQUAL);
					else if (WINDOW1[y][x] == 242)
						addch(ACS_GEQUAL);
					else if (WINDOW1[y][x] == 227)
						addch(ACS_PI);
					else
						addch(32);
				}
			}
		}
	}
	else {
		for (y = 0; y<24; y++) {
			for (x = 0; x<80; x++) {
				move(y, x);
				if (WINDOW2[y][x] <= 127)
					addch(WINDOW2[y][x]);
				else {// no codepage 850 ;--;
					if (WINDOW2[y][x] >= 176 && WINDOW2[y][x] <= 178)
						addch(ACS_CKBOARD);
					else if (WINDOW2[y][x] == 106 || WINDOW2[y][x] == 205)
						addch(ACS_HLINE);
					else if (WINDOW2[y][x] == 179 || WINDOW2[y][x] == 186)
						addch(ACS_VLINE);
					else if (WINDOW2[y][x] >= 219 && WINDOW2[y][x] <= 223)
						addch(ACS_CKBOARD);
					else if (WINDOW2[y][x] == 254)
						addch(ACS_CKBOARD);
					else if (WINDOW2[y][x] == 192 || WINDOW2[y][x] == 200)
						addch(ACS_LLCORNER);
					else if (WINDOW2[y][x] == 201 || WINDOW2[y][x] == 218)
						addch(ACS_ULCORNER);
					else if (WINDOW2[y][x] == 187 || WINDOW2[y][x] == 191)
						addch(ACS_URCORNER);
					else if (WINDOW2[y][x] == 188 || WINDOW2[y][x] == 217)
						addch(ACS_LRCORNER);
					else if (WINDOW2[y][x] == 204 || WINDOW2[y][x] == 195)
						addch(ACS_LTEE);
					else if (WINDOW2[y][x] == 180 || WINDOW2[y][x] == 185)
						addch(ACS_RTEE);
					else if (WINDOW2[y][x] == 193 || WINDOW2[y][x] == 202)
						addch(ACS_BTEE);
					else if (WINDOW2[y][x] == 194 || WINDOW2[y][x] == 203)
						addch(ACS_TTEE);
					else if (WINDOW2[y][x] == 197 || WINDOW2[y][x] == 206)
						addch(ACS_PLUS);
					else if (WINDOW2[y][x] == 167)
						addch(ACS_DEGREE);
					else if (WINDOW2[y][x] == 241)
						addch(ACS_PLMINUS);
					else if (WINDOW2[y][x] == 249 || WINDOW2[y][x] == 250)
						addch(ACS_BULLET);
					else if (WINDOW2[y][x] == 243)
						addch(ACS_LEQUAL);
					else if (WINDOW2[y][x] == 242)
						addch(ACS_GEQUAL);
					else if (WINDOW2[y][x] == 227)
						addch(ACS_PI);
					else
						addch(32);
				}
			}
		}
	}

#endif
}

#ifdef linux
#if !defined(SDL_USE) &&  !defined(_WIN32)
static void finish(int sig) {
	endwin();
	exit(0);
}
#endif
#endif
