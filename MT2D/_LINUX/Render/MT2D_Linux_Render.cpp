// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.

//#include <stdio.h>

#if defined(linux) && !defined(__ANDROID__)
#include "MT2D_Terminal_Define.h"
#include <cstdlib>
#include <curses.h>
#include <signal.h>
#include <locale.h>
#include "../../MT2D.h"


static void finish(int sig);


void MT2D_Linux_Clear_Main_Window() {
	int i = 0, j = 0;
	while (i <= MAX_VER) {
		while (j<MAX_HOR) {
			WINDOW1[i][j] = ' ';
			j++;
		}
		i++;
		j = 0;
	}
}


void MT2D_Linux_Draw_Window(int which) {
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
}

static void finish(int sig) {
	endwin();
	exit(0);
}
#endif
