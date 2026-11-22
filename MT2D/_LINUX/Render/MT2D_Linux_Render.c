/* the "linux" macro (also used for macOS) is defined by this header, include it before the #if below */
#include "../../MT2D_Terminal_Define.h"
/* MT2D terminal (curses) renderer, used on Linux and macOS when SDL is not enabled. */
#include "../../MT2D_Terminal_Define.h"

#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
/* wide character curses (ncursesw) is needed to draw the CP850 characters as Unicode */
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#include <stdlib.h>
#include <curses.h>
#include <locale.h>
#include "../../MT2D.h"
#include "../MT2D_Linux_CP850.h"
#ifdef MT2D_LINUX_SIXEL
#include <stdio.h>
#include <string.h>
#include "MT2D_Linux_Sixel.h"
#include "MT2D_Linux_Image.h"
#endif

static bool ncurses_started = false;

#ifdef MT2D_LINUX_SIXEL
#define SIXEL_ROWS 24
#define SIXEL_W (MAX_HOR * MT2D_SIXEL_CELL_W)
#define SIXEL_H (SIXEL_ROWS * MT2D_SIXEL_CELL_H)
#define SIXEL_MAX_IMAGES 256
#define SIXEL_OUT_SIZE (1024 * 1024)
/* The text colors are palette indexes, the same as the MS-DOS VGA mode: 16 is black and 31 is white in the default palette. */
#define SIXEL_TEXT_BG 16
#define SIXEL_TEXT_FG 31
/* The images are placed on a 320x240 virtual screen (like SDL), stretched to the SIXEL_W x SIXEL_H canvas. */
#define SIXEL_VIRTUAL_W 320
#define SIXEL_VIRTUAL_H 240

typedef struct Sixel_Queued {
	Sprite *Img;
	int X;
	int Y;
} Sixel_Queued;

static Sixel_Queued sixel_queue[SIXEL_MAX_IMAGES]; /* images of the frame being built, drawn over the text */
static int sixel_queue_count = 0;
static unsigned char sixel_pixels[SIXEL_W * SIXEL_H];
static unsigned char sixel_last_pixels[SIXEL_W * SIXEL_H]; /* the last frame sent, an equal frame is not sent again */
static MT2D_Sixel_Canvas sixel_canvas;
static MT2D_Sixel_Canvas sixel_last_canvas;
static char sixel_buffer[SIXEL_OUT_SIZE];
static bool sixel_has_last = false;

void MT2D_Linux_Sixel_Queue_Image(Sprite *img, int x, int y) {
	if (!img || !img->Image || sixel_queue_count >= SIXEL_MAX_IMAGES) return;
	sixel_queue[sixel_queue_count].Img = img;
	sixel_queue[sixel_queue_count].X = x;
	sixel_queue[sixel_queue_count].Y = y;
	sixel_queue_count++;
}
#endif

static void MT2D_Linux_Stop_Curses(void) {
	if (ncurses_started) {
#ifdef MT2D_LINUX_SIXEL
		fputs("\033[?80l", stdout); /* sixel scrolling back on */
		fflush(stdout);
#endif
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
#ifdef MT2D_LINUX_SIXEL
		/* curses still reads the keyboard but draws nothing: clear the screen now, a later getch would clear
		   it again and erase the image. ?80h = sixel display mode (the image does not scroll the screen). */
		refresh();
		fputs("\033[?80h", stdout);
		fflush(stdout);
#endif
		ncurses_started = true;
		atexit(MT2D_Linux_Stop_Curses);
	}
}

void MT2D_Linux_Screen_Resized(void) {
#ifdef MT2D_LINUX_SIXEL
	/* the terminal drops or reflows the image and curses clears the screen: an unchanged frame must be sent again */
	sixel_has_last = false;
#endif
	/* text mode: curses redraws by itself, every frame writes every cell */
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

#ifdef MT2D_LINUX_SIXEL
/* Graphics mode: the 80x24 cells are drawn as a Sixel image with a bitmap font, the queued images go over it. */
void MT2D_Linux_Draw_Window(int which) {
	unsigned char (*buffer)[MAX_HOR] = (which == DISPLAY_WINDOW1) ? WINDOW1 : WINDOW2;
	unsigned int len;
	int i, sx, sy, sx1, sy1;
	MT2D_Linux_Start_Curses();
	MT2D_Sixel_Draw_Text(&sixel_canvas, sixel_pixels, &buffer[0][0], MAX_HOR, SIXEL_ROWS, SIXEL_TEXT_BG, SIXEL_TEXT_FG);
	for (i = 0; i < sixel_queue_count; i++) {
		Sprite *s = sixel_queue[i].Img;
		int w = s->ScaleX > 0 ? s->ScaleX : s->SizeX; /* size on the virtual screen */
		int h = s->ScaleY > 0 ? s->ScaleY : s->SizeY;
		sx = (int)((long)sixel_queue[i].X * SIXEL_W / SIXEL_VIRTUAL_W);
		sy = (int)((long)sixel_queue[i].Y * SIXEL_H / SIXEL_VIRTUAL_H);
		sx1 = (int)((long)(sixel_queue[i].X + w) * SIXEL_W / SIXEL_VIRTUAL_W);
		sy1 = (int)((long)(sixel_queue[i].Y + h) * SIXEL_H / SIXEL_VIRTUAL_H);
		MT2D_Sixel_Draw_Image(&sixel_canvas, s->Image->Pixels, s->SizeX, s->SizeY, sx, sy, sx1 - sx, sy1 - sy);
	}
	sixel_queue_count = 0;
	if (sixel_has_last &&
		memcmp(sixel_canvas.Palette, sixel_last_canvas.Palette, sizeof(sixel_canvas.Palette)) == 0 &&
		memcmp(sixel_pixels, sixel_last_pixels, sizeof(sixel_pixels)) == 0) return;
	len = MT2D_Sixel_Encode(&sixel_canvas, sixel_buffer, (unsigned int)sizeof(sixel_buffer));
	if (!len) return;
	memcpy(sixel_last_pixels, sixel_pixels, sizeof(sixel_pixels));
	sixel_last_canvas = sixel_canvas;
	sixel_has_last = true;
	fputs("\033[H", stdout); /* top left corner */
	fwrite(sixel_buffer, 1, len, stdout);
	fflush(stdout);
}
#else
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
#endif /* MT2D_LINUX_SIXEL */
#endif
