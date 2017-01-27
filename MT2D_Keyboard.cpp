#include "MT2D/MT2D_Terminal_Define.h"
#include "MT2D/MT2D_Keyboard.h"

#ifdef SDL_USE
    #include "SDL_MT2D/SDL_MT2Dmain.h"
    #include <stdio.h>
#elif defined __MSDOS__
    #include <conio.h>
#elif defined _WIN32
    #include <stdio.h>
    #include <conio.h>
#elif defined linux //linux without SDL
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <curses.h>


int kbhit(void){
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

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

#endif

bool MT2D_Keyboard_touched(){//Generic function, easy to be replaced for porting
    #ifdef SDL_USE //any system that uses SDL will run only the code below
        bool i = SDL_Keyboard_touched();//cannot use _kbhit on SDL because it'll check the terminal window not the SDL one//
 //       printf("Keyboard touched:%d\n",i);
	#elif defined _WIN32 //without SDL
		bool i=_kbhit();
	#elif defined __MSDOS__
		bool i=kbhit();
	#elif defined linux //linux without SDL
        bool i=kbhit();
	#endif
	return i;
}
int	MT2D_Keyboard_keytouched(){//Generic function, easy to be replaced for porting
/*
  borland and visual studio getch works equal to almost all keys.
  But, some keys required to be adapted to the visual studio type to maintain compatible with the old softwares
*/
	int x=0;
	#ifdef SDL_USE
        x=SDL_getkeyboard();
        printf("Keyboard key touched:%d\n",x);
    #elif defined _WIN32
		x=_getch();
		fflush(stdin);
    #elif defined __MSDOS__
       x=getch();//Borland ms-dos code != windows code, must adapt
       if(x==0)x=224;
    #elif defined linux
        x=getch();
    #endif
	return x;
}
