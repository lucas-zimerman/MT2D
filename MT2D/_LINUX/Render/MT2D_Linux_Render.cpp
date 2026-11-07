// MT2D terminal (curses) renderer, used on Linux and macOS when SDL is not enabled.
#include "../../MT2D_Terminal_Define.h"

#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
#include <cstdlib>
#include <curses.h>
#include <locale.h>
#include "../../MT2D.h"

static bool ncurses_started = false;

static void MT2D_Linux_Stop_Curses() {
	if (ncurses_started) {
		endwin();
		ncurses_started = false;
	}
}

void MT2D_Linux_Start_Curses() {
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

void MT2D_Linux_Clear_Main_Window() {
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

/* Converts a MS-DOS (code page 437/850) character to something curses can draw. */
static void MT2D_Linux_Add_Char(unsigned char c) {
	if (c >= 32 && c <= 126) { addch(c); return; }
	switch (c) {
	case 176: case 177: case 178: case 219: case 220: case 221: case 222: case 223: case 254:
		addch(ACS_CKBOARD); break;
	case 106: case 196: case 205: addch(ACS_HLINE); break;
	case 179: case 186: addch(ACS_VLINE); break;
	case 192: case 200: addch(ACS_LLCORNER); break;
	case 201: case 218: addch(ACS_ULCORNER); break;
	case 187: case 191: addch(ACS_URCORNER); break;
	case 188: case 217: addch(ACS_LRCORNER); break;
	case 204: case 195: addch(ACS_LTEE); break;
	case 180: case 185: addch(ACS_RTEE); break;
	case 193: case 202: addch(ACS_BTEE); break;
	case 194: case 203: addch(ACS_TTEE); break;
	case 197: case 206: addch(ACS_PLUS); break;
	case 167: addch(ACS_DEGREE); break;
	case 241: addch(ACS_PLMINUS); break;
	case 249: case 250: addch(ACS_BULLET); break;
	case 243: addch(ACS_LEQUAL); break;
	case 242: addch(ACS_GEQUAL); break;
	case 227: addch(ACS_PI); break;
	case 24: addch(ACS_UARROW); break;
	case 25: addch(ACS_DARROW); break;
	case 26: addch(ACS_RARROW); break;
	case 27: addch(ACS_LARROW); break;
	default: addch(' '); break;
	}
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
