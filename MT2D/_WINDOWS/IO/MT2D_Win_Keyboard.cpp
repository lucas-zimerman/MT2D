#include "../../MT2D_Terminal_Define.h"
#if defined(_WIN32) && !defined(SDL_USE)
#include <stdio.h>
#include <conio.h>


bool MT2D_Win_Keyboard_touched() {//Generic function, easy to be replaced for porting
	bool i = _kbhit();
	return i;
}
int	MT2D_Win_Keyboard_keytouched() {//Generic function, easy to be replaced for porting
								/*
								borland and visual studio getch works equal to almost all keys.
								But, some keys required to be adapted to the visual studio type to maintain compatible with the old softwares
								*/
	int x = 0;
	x = _getch();
	fflush(stdin);
	return x;
}
#endif