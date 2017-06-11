/*===================================================================
	MT2D_SDL_Defines Created by: Lucas Zimerman Fraulob

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
extern int SCREEN_WIDTH;//640
extern int SCREEN_HEIGHT;//200
extern int SCREEN_MISSING_WIDTH;
extern int SCREEN_MISSING_HEIGHT;

extern int FONT_SIZEX;//the size of the font image
extern int FONT_SIZEY;
extern int FONT_RESIZEX;//how many pixels it'll increase/decrease
extern int FONT_RESIZEY;

extern char SDL_SOUND_TYPE;// -1 not defined, 0 BEEPER, 1 AUDIO FILE
#endif
