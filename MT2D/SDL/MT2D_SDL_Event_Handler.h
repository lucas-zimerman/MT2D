#ifndef MT2D_SDL_EH
#define MT2D_SDL_EH

#include "../MT2D_Terminal_Define.h"
#ifdef SDL_USE

#include <SDL.h>
#include "MT2D_SDL_Redefine.h"
#include "../ObjectCore/Sprites.h"

struct MT2D_SDL_Events {
	SDL_KeyboardEvent Keyboard;
	bool Keyboard_Pressed;
	bool Keyboard_Consumed;
	int32_t  *Keyboard_Buffer;//used to store all the keys that are active or not
	unsigned int Keyboard_Buffer_Count;
	Sprite *SpriteBuffer;//array of pointers where you know what sprites are requested to render.
	int SpriteBuffer_Count;
	int *SpriteBufferX;
	int *SpriteBufferY;
	SDL_WindowEvent Window;
	bool Window_Resized;
	bool Window_Started;
	bool Window_Consumed;
	bool Close_Program;

	MT2D_SDL_Renderer *Render;
	bool RequestRender;
};

void MT2D_SDL_Event_Handler();
void MT2D_SDL_Event_Handler_NoLoop();
#endif

#endif

