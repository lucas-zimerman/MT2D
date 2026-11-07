// the "linux" macro (also used for macOS) is defined by this header, include it before the #if below
#include "../../MT2D_Terminal_Define.h"
// MT2D terminal keyboard (curses), used on Linux and macOS when SDL is not enabled.
#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
#include "../../MT2D_Terminal_Define.h"
#include "../../MT2D_Keyboard.h"
// wide character curses (ncursesw) is needed to read the typed characters as Unicode
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#include <curses.h>
#include <wchar.h>
#include "../MT2D_Linux_CP850.h"

void MT2D_Linux_Start_Curses();

static int pending_special = 0; /* second half of a "224, code" arrow key sequence */

/* Converts a key from get_wch to the codes the MS-DOS getch returns, characters are CP850.
   Returns -1 for keys that have no MS-DOS equivalent (they are ignored). */
static int MT2D_Linux_Read_Key(int status, wint_t ch) {
	if (status == KEY_CODE_YES) {
		switch (ch) {
		case KEY_UP: pending_special = arrow_up_pressed; return arrow_key_pressed;
		case KEY_DOWN: pending_special = arrow_down_pressed; return arrow_key_pressed;
		case KEY_LEFT: pending_special = arrow_left_pressed; return arrow_key_pressed;
		case KEY_RIGHT: pending_special = arrow_right_pressed; return arrow_key_pressed;
		case KEY_BACKSPACE: return back_space_pressed;
		case KEY_ENTER: return enter_pressed;
		default: return -1;
		}
	}
	switch (ch) {
	case 127: return back_space_pressed;
	case '\n': case '\r': return enter_pressed;
	}
	if (ch < 32) return (int)ch; /* tab, escape, ctrl+key */
	return MT2D_Unicode_To_CP850((unsigned int)ch);
}

/* A key read by Keyboard_touched that was not consumed yet */
static bool has_peeked = false;
static int peeked_status;
static wint_t peeked_char;

bool MT2D_Linux_Keyboard_touched() {
	if (pending_special || has_peeked) return true;
	MT2D_Linux_Start_Curses();
	nodelay(stdscr, TRUE);
	peeked_status = get_wch(&peeked_char);
	nodelay(stdscr, FALSE);
	has_peeked = (peeked_status != ERR);
	return has_peeked;
}

int MT2D_Linux_Keyboard_keytouched() {
	wint_t ch;
	int status, key;
	if (pending_special) {
		key = pending_special;
		pending_special = 0;
		return key;
	}
	MT2D_Linux_Start_Curses();
	do {
		if (has_peeked) {
			status = peeked_status;
			ch = peeked_char;
			has_peeked = false;
		} else {
			status = get_wch(&ch);
		}
		key = (status == ERR || (status == KEY_CODE_YES && ch == KEY_RESIZE)) ? -1 : MT2D_Linux_Read_Key(status, ch);
	} while (key < 0);
	return key;
}
#endif
