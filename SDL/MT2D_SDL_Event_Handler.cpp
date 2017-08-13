#include "MT2D_SDL_Event_Handler.h"
#include <MT2D/MT2D_System_Calls.h>
#include <MT2D/SDL/Render/MT2D_SDL_Render.h>
#include <MT2D/MT2D_Debug.h>


MT2D_SDL_Events MainEvents;
SDL_Event Events;

void MT2D_SDL_Event_Handler() {
	while(SDL_PollEvent(&Events) != 0) {//As this function implicitly calls SDL_PumpEvents(), you can only call this function in the thread that set the video mode. 
		if (Events.type == SDL_KEYDOWN) {
#ifdef _DEBUG
			MT2D_Ide_Printf("MT2D: SDL Events - Keyboard Touched\n");
#endif
			//clone the event data
			MainEvents.Keyboard_Pressed = true;
			MainEvents.Keyboard.keysym = Events.key.keysym;
			MainEvents.Keyboard.padding2 = Events.key.padding2;
			MainEvents.Keyboard.padding3 = Events.key.padding3;
			MainEvents.Keyboard.repeat = Events.key.repeat;
			MainEvents.Keyboard.state = Events.key.state;
			MainEvents.Keyboard.timestamp = Events.key.timestamp;
			MainEvents.Keyboard.type = Events.key.type;
			MainEvents.Keyboard.windowID = Events.key.windowID;
		}
		else if (Events.type == SDL_WINDOWEVENT) {
			if (Events.window.event == SDL_WINDOWEVENT_RESIZED || Events.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
#ifdef _DEBUG
				MT2D_Ide_Printf("MT2D: SDL Events - Window Resized\n");
#endif
				MainEvents.Window.data1 = Events.window.data1;
				MainEvents.Window.data2 = Events.window.data2;
				MainEvents.Window_Resized = true;
				MainEvents.Window_Started = true;
			}
		}
		else if (Events.type == SDL_QUIT) {
#ifdef _DEBUG
			MT2D_Ide_Printf("MT2D: SDL Event Thread Quit\n");
#endif
			SDL_Quit();
			exit(0);
		}
	}
}