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

bool MT2D_Win_Init2() {// Return: true - it started without any kind of problem, false : there were a problem when MT2D was started
	bool output = true;
	#ifdef SDL_USE
		MT2D_SDL_Init();//Start SDL files and other things
	#else
		#ifndef _DEBUG
			#ifdef _WIN32// hides the console on release projects
				#ifdef SDL_USE
					HWND windowHandle = GetConsoleWindow();
					output = ShowWindow(windowHandle, SW_HIDE);
				#endif
			#endif//if there's a way of doing the same thing, I'm going to make it
		#endif
	#endif
	return output;
}
#endif