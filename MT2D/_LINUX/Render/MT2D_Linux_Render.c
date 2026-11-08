// the "linux" macro (also used for macOS) is defined by this header, include it before the #if below
#include "../../MT2D_Terminal_Define.h"
// MT2D terminal (curses) renderer, used on Linux and macOS when SDL is not enabled.
#include "../../MT2D_Terminal_Define.h"

#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
// wide character curses (ncursesw) is needed to draw the CP850 characters as Unicode
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#include <stdlib.h>
#include <curses.h>
#include <locale.h>
#include "../../MT2D.h"
#include "../MT2D_Linux_CP850.h"

static bool ncurses_started = false;

static void MT2D_Linux_Stop_Curses(void) {
	if (ncurses_started) {
		endwin();
		ncurses_started = false;
	}
}

void MT2D_Linux_Start_Curses(void) {
	if (!ncurses_started) {
		setlocale(LC_ALL, "");
		initscr();
		keypad(stdscr, TRUE);
		nonl();
		cbreak();
		noecho();
		curs_set(0);
		ncurses_started = true;
		atexit(MT2D_Linux_Stop_Curses);
	}
}

void MT2D_Linux_Clear_Main_Window(void) {
	int i = 0, j = 0;
	while (i <= MAX_VER) {
		while (j < MAX_HOR) {
			WINDOW1[i][j] = ' ';
			j++;
		}
		i++;
		j = 0;
	}
}

/* Draws a CP850 byte (the encoding of the display buffers) as its Unicode character. */
static void MT2D_Linux_Add_Char(unsigned char c) {
	wchar_t wide[2];
	cchar_t cell;
	wide[0] = (wchar_t)MT2D_CP850_To_Unicode(c);
	wide[1] = 0;
	if (setcchar(&cell, wide, A_NORMAL, 0, NULL) == OK && add_wch(&cell) != ERR) return;
	addch(' ');
}

void MT2D_Linux_Draw_Window(int which) {
	int x, y;
	unsigned char (*buffer)[MAX_HOR] = (which == DISPLAY_WINDOW1) ? WINDOW1 : WINDOW2;
	MT2D_Linux_Start_Curses();
	for (y = 0; y < 24; y++) {
		move(y, 0);
		for (x = 0; x < MAX_HOR; x++) {
			MT2D_Linux_Add_Char(buffer[y][x]);
		}
	}
	refresh();
}
#endif
