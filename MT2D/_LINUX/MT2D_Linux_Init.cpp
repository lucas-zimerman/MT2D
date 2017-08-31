// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.


#if defined(linux)
#include "..//MT2D_Terminal_Define.h"
#include <stdio.h>
#include <cstdlib>
#include <curses.h>
#include <signal.h>
#include <locale.h>
#include <string.h>
#include "../MT2D.h"
bool MT2D_Linux_Init() {// Return: true - it started without any kind of problem, false : there were a problem when MT2D was started
	bool output = true;
	return output;
}