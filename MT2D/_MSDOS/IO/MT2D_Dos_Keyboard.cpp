#if defined(__MSDOS__)
#include <MT2D/MT2D_Terminal_Define.h>
#include <conio.h>

bool MT2D_Dos_Keyboard_touched() {//Generic function, easy to be replaced for porting
	bool i = kbhit();
	return i;
}
int	MT2D_Dos_Keyboard_keytouched() {//Generic function, easy to be replaced for porting
								/*
								borland and visual studio getch works equal to almost all keys.
								But, some keys required to be adapted to the visual studio type to maintain compatible with the old softwares
								*/
	int x = 0;
	x = getch();//Borland ms-dos code != windows code, must adapt
	if (x == 0)x = 224;
	return x;
}
#endif