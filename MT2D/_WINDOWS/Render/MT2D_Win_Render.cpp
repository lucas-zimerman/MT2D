
// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.
#include "../../MT2D_Terminal_Define.h"

#if defined(_WIN32) && !defined(SDL_USE)

//#include <stdio.h>

#include "../../MT2D.h"
#include <Windows.h>

SMALL_RECT windowSize = { 0, 0, MAX_HOR - 1, MAX_VER - 1 };
COORD bufferSize = { MAX_HOR, MAX_VER };
COORD characterBufferSize = { MAX_HOR, MAX_VER };
COORD characterPosition = { 0, 0 };
SMALL_RECT consoleWriteArea = { 0, 0, MAX_HOR - 1, MAX_VER - 1 };
CHAR_INFO consoleBuffer[MAX_HOR * MAX_VER];
HANDLE wHnd; /* write (output) handle */
HANDLE rHnd; /* read (input handle */


void MT2D_Win_Clear_Main_Window() {
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
}


void MT2D_Win_Draw_Window(int which) {
	//transfer window1 or 2 memory to video memory, also include other systems that have video memory acess here too
	//"ansi" function that will work with any operational system
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
}

#endif
