// the "linux" macro (also used for macOS) is defined by this header, include it before the #if below
#include "../../MT2D_Terminal_Define.h"
// MT2D terminal keyboard (curses), used on Linux and macOS when SDL is not enabled.
#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
#include "../../MT2D_Terminal_Define.h"
#include "../../MT2D_Keyboard.h"
#include <curses.h>

void MT2D_Linux_Start_Curses();

static int pending_special = 0; /* second half of a "224, code" arrow key sequence */

/* Reads one key and converts it to the same codes the MS-DOS getch returns. */
static int MT2D_Linux_Read_Key(int ch) {
	switch (ch) {
	case KEY_UP: pending_special = arrow_up_pressed; return arrow_key_pressed;
	case KEY_DOWN: pending_special = arrow_down_pressed; return arrow_key_pressed;
	case KEY_LEFT: pending_special = arrow_left_pressed; return arrow_key_pressed;
	case KEY_RIGHT: pending_special = arrow_right_pressed; return arrow_key_pressed;
	case KEY_BACKSPACE: case 127: return back_space_pressed;
	case '\n': case KEY_ENTER: return enter_pressed;
	default: return ch;
	}
}

bool MT2D_Linux_Keyboard_touched() {
	int ch;
	if (pending_special) return true;
	MT2D_Linux_Start_Curses();
	nodelay(stdscr, TRUE);
	ch = getch();
	nodelay(stdscr, FALSE);
	if (ch == ERR) return false;
	ungetch(ch);
	return true;
}

int MT2D_Linux_Keyboard_keytouched() {
	int ch;
	if (pending_special) {
		ch = pending_special;
		pending_special = 0;
		return ch;
	}
	MT2D_Linux_Start_Curses();
	do {
		ch = getch();
	} while (ch == ERR || ch == KEY_RESIZE);
	return MT2D_Linux_Read_Key(ch);
}
#endif
