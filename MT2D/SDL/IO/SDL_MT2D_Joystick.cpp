#ifndef __ANDROID__
#include <MT2D/MT2D_Terminal_Define.h>
#else
#include "../../MT2D_Terminal_Define.h"
#endif

#if defined(SDL_USE) && !defined(__ANDROID__)
#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#include <MT2D/Joystick/MT2D_Joystick.h>
#elif defined(SDL_USE) && defined(__ANDROID__)
#include "../../SDL/MT2D_SDL_Event_Handler.h"
#include "../../Joystick/MT2D_Joystick.h"
#endif

#ifdef SDL_USE

extern MT2D_SDL_Events MainEvents;


SDL_Joystick* SDL_LoadJoystick() {
	SDL_Joystick * Joy = 0;
	if (SDL_NumJoysticks() > 0) {
		Joy = SDL_JoystickOpen(0);

	}
	return Joy;
}

void SDL_Joystick_Init() {
	SDL_Init(SDL_INIT_JOYSTICK);
}

void MT2D_SDL_Update_Keys(MT2D_Joystick *MyJosytick) {
	int i = 0,j=0;
	unsigned char SDL_EmulatedKey = 0;
	if (MainEvents.Keyboard_Buffer_Count == 2) {
		i = 0;
	}
	while (i < MainEvents.Keyboard_Buffer_Count) {
		SDL_EmulatedKey = 0;
		/*Step 1, translate the key to something that we can read*/
		if (MainEvents.Keyboard_Buffer[i] >= 97 && MainEvents.Keyboard_Buffer[i] <= 122) {
			SDL_EmulatedKey = MainEvents.Keyboard_Buffer[i];
		}
		else if (MainEvents.Keyboard_Buffer[i] <97) {//espero ser sempre maior que zero
			SDL_EmulatedKey = MainEvents.Keyboard_Buffer[i];
		}
//		else if (MainEvents.Keyboard_Buffer[i] >= SDLK_F1 && MainEvents.Keyboard_Buffer[i] <= SDLK_F12) {//converte as teclas F12
//			SDL_EmulatedKeySpecial = 224;
//			SDL_EmulatedKey = MainEvents.Keyboard_Buffer[i] - 1073741823; // SDLK_F1 - 59
//		}
		if (MainEvents.Keyboard_Buffer[i] == SDLK_UP) {
			//printf(" seta cima\n");
			MyJosytick->Left_Y_axis = -32768;
			MyJosytick->Left_Modified = true;
			MyJosytick->Touched = true;
		}
		else if (MainEvents.Keyboard_Buffer[i] == SDLK_DOWN) {
			MyJosytick->Left_Y_axis = 32767;
			MyJosytick->Left_Modified = true;
			MyJosytick->Touched = true;
		}
		else if (MainEvents.Keyboard_Buffer[i] == SDLK_LEFT) {
			MyJosytick->Left_X_axis = -32768;
			MyJosytick->Left_Modified = true;
			MyJosytick->Touched = true;
		}
		else if (MainEvents.Keyboard_Buffer[i] == SDLK_RIGHT) {
			MyJosytick->Left_X_axis = 32767;
			MyJosytick->Left_Modified = true;
			MyJosytick->Touched = true;
		}
		if (SDL_EmulatedKey > 0) {
			j = 0;
			while (j < 4) {
				if (SDL_EmulatedKey == MyJosytick->Mapped_Button[j]) {
					MyJosytick->Button_Pressed[j] = true;
					MyJosytick->Touched = true;
				}
				j++;
			}
		}
		i++;
	}/*
	if (MainEvents.Keyboard_Buffer_Count > 0) {
		printf("Joystick - clear buffer");
		free(MainEvents.Keyboard_Buffer);
		MainEvents.Keyboard_Buffer_Count = 0;
	}
	*/
}

#endif