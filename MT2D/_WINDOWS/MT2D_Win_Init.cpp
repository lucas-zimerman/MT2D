// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.


#include "../MT2D_Terminal_Define.h"
#if defined(_WIN32) && !defined(SDL_USE)

#include <stdio.h>
#include <MT2D/MT2D.h>
#include <MT2D/SDL/MT2D_SDL_main.h>
#include <thread>
#include <Windows.h>
#include "MT2D/MT2D.h"
#include <string.h>

extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER + 1][MAX_HOR];


bool MT2D_Win_Init() {// Return: true - it started without any kind of problem, false : there were a problem when MT2D was started
	bool output = true;
	return output;
}
#endif