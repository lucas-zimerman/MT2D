#include "../MT2D_Terminal_Define.h"

#if defined(_WIN32)  && !defined(SDL_USE)
#include <stdio.h>
#include "../MT2D.h"
#include <Windows.h>

void MT2D_Win_Cursor_goto_topscreen() {
	COORD coord;
	coord.X = 0;
	coord.Y = 0;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void MT2D_Win_Cursor_Set_Position(int line, int column) {
	COORD coord;
	coord.X = column;
	coord.Y = line;
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
#endif