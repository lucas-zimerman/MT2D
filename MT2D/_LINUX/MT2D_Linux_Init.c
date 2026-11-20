/* the "linux" macro (also used for macOS) is defined by this header, include it before the #if below */
#include "../MT2D_Terminal_Define.h"
/* MT2D Created By Lucas Zimerman Fraulob */
/* This is the core of MT2D engine, with only this .cpp you can do everything */
/* This file is related to almost all MT2D video interface, like draw, clear and update. */


#if defined(linux)
#include "..//MT2D_Terminal_Define.h"
#ifdef MT2D_LINUX_SIXEL
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L /* termios, select (strict -std=c89 builds) */
#endif
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include "Render/MT2D_Linux_Sixel.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <locale.h>
#include <string.h>
#include "../MT2D.h"
#ifdef MT2D_LINUX_SIXEL
#define SIXEL_REPLY_TIMEOUT_MS 2000 /* a slow ssh link needs time for the round trip */

/* Asks the terminal for its device attributes and waits for the answer. Returns 1 when it lists Sixel. */
static int MT2D_Linux_Terminal_Has_Sixel(void) {
	struct termios saved, raw;
	char reply[64];
	int len = 0, ok;
	struct timeval limit;
	fd_set fds;
	char c;
	if (!isatty(0) || !isatty(1) || tcgetattr(0, &saved) != 0) return 0;
	raw = saved;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &raw);
	tcflush(0, TCIFLUSH);
	if (write(1, "\033[c", 3) != 3) {
		tcsetattr(0, TCSANOW, &saved);
		return 0;
	}
	limit.tv_sec = SIXEL_REPLY_TIMEOUT_MS / 1000;
	limit.tv_usec = (SIXEL_REPLY_TIMEOUT_MS % 1000) * 1000;
	while (len < (int)sizeof(reply) - 1) { /* select updates limit on Linux: the timeout is for the whole reply */
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		if (select(1, &fds, NULL, NULL, &limit) <= 0 || read(0, &c, 1) != 1) break;
		reply[len++] = c;
		if (c == 'c') break; /* end of the reply */
	}
	reply[len] = '\0';
	ok = MT2D_Sixel_Reply_Supports_Sixel(reply);
	tcsetattr(0, TCSANOW, &saved);
	return ok;
}
#endif

bool MT2D_Linux_Init(void) { /* Return: true - it started without any kind of problem, false : there were a problem when MT2D was started */
	bool output = true;
#ifdef MT2D_LINUX_SIXEL
	/* MT2D_SIXEL=1 skips the check, for a terminal that draws Sixel but does not say so */
	const char *force = getenv("MT2D_SIXEL");
	if (!(force && force[0] == '1') && !MT2D_Linux_Terminal_Has_Sixel()) {
		fputs("MT2D: this terminal does not support Sixel graphics (built with MT2D_TERMINAL_USE_GRAPHICS).\n"
			"Use a Sixel terminal (foot, wezterm, mlterm, xterm -ti vt340), or set MT2D_SIXEL=1 if yours supports it.\n", stderr);
		exit(1);
	}
#endif
	return output;
}

/* No setting is supported on this backend yet. */
MT2D_Result MT2D_Linux_Set_Settings(MT2D_Setting_Operation operation, int param1, int param2) {
	(void)operation; (void)param1; (void)param2;
	return MT2D_UNSUPPORTED;
}

MT2D_Result MT2D_Linux_Set_Settings_Argv(int argc, const int *argv) {
	if (argc < 1 || argv == NULL)
		return MT2D_INVALID;
	return MT2D_UNSUPPORTED;
}

#endif