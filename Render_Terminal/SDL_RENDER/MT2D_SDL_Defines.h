/*===================================================================
	SDL_Defines.h Created by: Lucas Zimerman Fraulob

	Here we have some of the definitions from SDL, these parameters
	are required for a proper functioning.
	Here we define the screen size, font(char) size and also what
	type of the audio SDL is going to use (unfortunately, with the
	actual code, we can only use SDL Beep emulated pc speaker or
	normal audio output, we cannot use one of each time or first one
	and then other, it will only use the first output that is going
	to be requested on the code)

===================================================================*/

#ifndef SDL_DEFINES
#define SDL_DEFINES

//Screen dimension
int SCREEN_WIDTH = 640;//640
int SCREEN_HEIGHT = 200;//200
int SCREEN_MISSING_WIDTH = 0;
int SCREEN_MISSING_HEIGHT = 0;

int FONT_SIZEX = 8;//the size of the font image
int FONT_SIZEY = 8;
int FONT_RESIZEX = 0;//how many pixels it'll increase/decrease
int FONT_RESIZEY = 0;

char SDL_SOUND_TYPE = -1;// -1 not defined, 0 BEEPER, 1 AUDIO FILE
#endif
