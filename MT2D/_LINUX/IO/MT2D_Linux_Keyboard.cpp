#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
#include "../../MT2D_Terminal_Define.h"
#include "../../MT2D_Keyboard.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <curses.h>


int kbhit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}


bool MT2D_Linux_Keyboard_touched() {//Generic function, easy to be replaced for porting
	bool i = kbhit();
	return i;
}
int	MT2D_Linux_Keyboard_keytouched() {//Generic function, easy to be replaced for porting
								/*
								borland and visual studio getch works equal to almost all keys.
								But, some keys required to be adapted to the visual studio type to maintain compatible with the old softwares
								*/
	int x = 0;
	x = getch();
	return x;
}
#endif