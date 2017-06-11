#ifndef MT2D_SDL_EH
#define MT2D_SDL_EH

#include <SDL.h>
#include <MT2D/SDL/MT2D_SDL_Redefine.h>

struct MT2D_SDL_Events {
	SDL_KeyboardEvent Keyboard;
	bool Keyboard_Pressed;
	bool Keyboard_Consumed;
	SDL_WindowEvent Window;
	bool Window_Resized;
	bool Window_Started;
	bool Window_Consumed;
	bool Close_Program;

	MT2D_SDL_Renderer *Render;
	bool RequestRender;
};
void MT2D_SDL_Event_Handler();
#endif

