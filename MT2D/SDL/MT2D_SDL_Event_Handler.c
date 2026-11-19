#include "../MT2D_Terminal_Define.h"
#ifdef SDL_USE
#include <SDL_config.h>
#include "MT2D_SDL_Event_Handler.h"
#include "../Joystick/MT2D_Joystick.h"
#include "../MT2D_Debug.h"
#include "MT2D_SDL_main.h"
#include "../Touch/MT2D_Touch.h"

extern MT2D_Joystick *GlobalJoystickHandler;
extern int MT2D_SDL_Alt_Enter_Fullscreen;
extern SDL_DisplayMode mode; /* the window size, MT2D_SDL_main.c */



MT2D_SDL_Events MainEvents;
SDL_Event Events;


void MT2D_SDL_REMOVE_Key_Buffer(int32_t Key) {
	int i = 0, j = 0;
	if (MainEvents.Keyboard_Buffer_Count > 0) {
		while (i < MainEvents.Keyboard_Buffer_Count && MainEvents.Keyboard_Buffer[i] != Key) {
			i++;
		}
		if (i < MainEvents.Keyboard_Buffer_Count && MainEvents.Keyboard_Buffer[i] == Key) {
			/*We found that key so remove it*/
			while (i < MainEvents.Keyboard_Buffer_Count - 1) {
				/*Pass everything after that key one index before*/
				MainEvents.Keyboard_Buffer[i] = MainEvents.Keyboard_Buffer[i + 1];
				i++;
			}
			/*resize the memory*/
			MainEvents.Keyboard_Buffer = (int32_t*)realloc(MainEvents.Keyboard_Buffer, (MainEvents.Keyboard_Buffer_Count) * sizeof(int32_t));
			MainEvents.Keyboard_Buffer_Count--;
		}
	}
}

/*
	Key releases are not applied right away: a quick tap (press + release inside the same frame) would
	be gone before the joystick code reads the buffer. They are applied by MT2D_SDL_Flush_Key_Releases,
	that the joystick update calls at the start of the next frame.
*/
#define MT2D_SDL_MAX_PENDING_RELEASES 32
static int32_t PendingReleases[MT2D_SDL_MAX_PENDING_RELEASES];
static int PendingReleaseCount = 0;

void MT2D_SDL_Flush_Key_Releases(void) {
	for (int i = 0; i < PendingReleaseCount; i++) {
		MT2D_SDL_REMOVE_Key_Buffer(PendingReleases[i]);
	}
	PendingReleaseCount = 0;
}

static void MT2D_SDL_Defer_Key_Release(int32_t Key) {
	if (PendingReleaseCount < MT2D_SDL_MAX_PENDING_RELEASES) {
		PendingReleases[PendingReleaseCount++] = Key;
	}
	else {
		MT2D_SDL_REMOVE_Key_Buffer(Key);
	}
}

void MT2D_SDL_ADD_Key_Buffer(int32_t Key) {
	int i = 0;
	/*the key was pressed again before its release was applied, so it is still held*/
	for (i = 0; i < PendingReleaseCount; i++) {
		if (PendingReleases[i] == Key) {
			PendingReleases[i] = PendingReleases[--PendingReleaseCount];
			break;
		}
	}
	i = 0;
	while (i < MainEvents.Keyboard_Buffer_Count && MainEvents.Keyboard_Buffer[i] != Key) {
		i++;
	}
	if (i == MainEvents.Keyboard_Buffer_Count) {
		if (MainEvents.Keyboard_Buffer_Count == 0) {
			MainEvents.Keyboard_Buffer = (int32_t*)malloc(sizeof(int32_t));
		}
		else {
			MainEvents.Keyboard_Buffer = (int32_t*)realloc(MainEvents.Keyboard_Buffer, (MainEvents.Keyboard_Buffer_Count + 1) * sizeof(int32_t));
		}
		MainEvents.Keyboard_Buffer[MainEvents.Keyboard_Buffer_Count] = Key;
		MainEvents.Keyboard_Buffer_Count++;
	}
}


/*
	Finger events go to MT2D_Touch. SDL gives 0..1 of the window; a portrait window (height >= width) shows the
	screen rotated by 90 degrees (see Render_New), so the game's x is the window's y and its y is 1 - the window's x.
*/
static int MT2D_SDL_Touch_Clamp(float v) {
	if (v < 0.0f) v = 0.0f;
	if (v > 1.0f) v = 1.0f;
	return (int)(v * MT2D_TOUCH_RANGE);
}

static void MT2D_SDL_Touch_Event(SDL_Event *e) {
	static int SpaceByTouch = 0; /* released on the lift even if controls were bound meanwhile */
	int x, y;
	if (mode.h >= mode.w) {
		x = MT2D_SDL_Touch_Clamp(e->tfinger.y);
		y = MT2D_SDL_Touch_Clamp(1.0f - e->tfinger.x);
	}
	else {
		x = MT2D_SDL_Touch_Clamp(e->tfinger.x);
		y = MT2D_SDL_Touch_Clamp(e->tfinger.y);
	}
	if (e->type == SDL_FINGERDOWN) {
		MT2D_Touch_Finger_Down((long)e->tfinger.fingerId, x, y);
		if (MT2D_Touch_Count() == 0) {
			/* no touch controls: a touch is the space key, as before MT2D_Touch */
			MT2D_SDL_ADD_Key_Buffer(32);
			MainEvents.Keyboard_Pressed = true;
			MainEvents.Keyboard.keysym.sym = 32;
			SpaceByTouch = 1;
		}
	}
	else if (e->type == SDL_FINGERMOTION) {
		MT2D_Touch_Finger_Move((long)e->tfinger.fingerId, x, y);
	}
	else {
		MT2D_Touch_Finger_Up((long)e->tfinger.fingerId, x, y);
		if (SpaceByTouch) {
			MT2D_SDL_Defer_Key_Release(32);
			SpaceByTouch = 0;
		}
	}
}

void MT2D_SDL_Event_Handler(void) {
	while(SDL_PollEvent(&Events) != 0) {//As this function implicitly calls SDL_PumpEvents(), you can only call this function in the thread that set the video mode. 
		if (Events.type == SDL_KEYDOWN && MT2D_SDL_Alt_Enter_Fullscreen &&
			(Events.key.keysym.sym == SDLK_RETURN || Events.key.keysym.sym == SDLK_KP_ENTER) && (Events.key.keysym.mod & KMOD_ALT)) {
			/* MT2D_SDL_ALT_ENTER_TOGGLE_FULLSCREEN: handled here, the game does not see this Enter */
			if (!Events.key.repeat)
				MT2D_SDL_Toggle_Fullscreen();
		}
		else if (Events.type == SDL_KEYDOWN) {
			//clone the event data
			MT2D_SDL_ADD_Key_Buffer(Events.key.keysym.sym);
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
		else if (Events.type == SDL_KEYUP) {
			MT2D_SDL_Defer_Key_Release(Events.key.keysym.sym);
		}
		else if (Events.type == SDL_WINDOWEVENT) {
			if (Events.window.event == SDL_WINDOWEVENT_RESIZED || Events.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				MainEvents.Window.data1 = Events.window.data1;
				MainEvents.Window.data2 = Events.window.data2;
				MainEvents.Window_Resized = true;
				MainEvents.Window_Started = true;
			}
		}
		else if (Events.type == SDL_FINGERDOWN || Events.type == SDL_FINGERMOTION || Events.type == SDL_FINGERUP) {
			MT2D_SDL_Touch_Event(&Events);
		}

		else if (Events.type == SDL_JOYAXISMOTION) {
			MT2D_Ide_Printf((char*)"Joystick Motion");
			if (GlobalJoystickHandler) {
				if (Events.jaxis.axis == 0) {
					// X axis
					GlobalJoystickHandler->Left_X_axis = Events.jaxis.value;
					GlobalJoystickHandler->Left_Modified = true;
					GlobalJoystickHandler->Touched = true;
				}
				else if (Events.jaxis.axis == 1) {
					// Y axis
					GlobalJoystickHandler->Left_Y_axis = Events.jaxis.value;
					GlobalJoystickHandler->Left_Modified = true;
					GlobalJoystickHandler->Touched = true;
				}
			}
		}
		else if (Events.type == SDL_JOYBUTTONDOWN) {
			if (GlobalJoystickHandler) {
				if (Events.jbutton.button < 4) {
					GlobalJoystickHandler->Button_Pressed[Events.jbutton.button] = true;
					GlobalJoystickHandler->Touched = true;
				}
			}
		}
		else if (Events.type == SDL_QUIT) {
			SDL_Quit();
			exit(0);
		}
	}
}

void MT2D_SDL_Event_Handler_NoLoop(void) {
	if(SDL_PollEvent(&Events) != 0) {//As this function implicitly calls SDL_PumpEvents(), you can only call this function in the thread that set the video mode. 
		if (Events.type == SDL_KEYDOWN || Events.type == SDL_KEYUP) {
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
				MainEvents.Window.data1 = Events.window.data1;
				MainEvents.Window.data2 = Events.window.data2;
				MainEvents.Window_Resized = true;
				MainEvents.Window_Started = true;
			}
		}
		else if (Events.type == SDL_FINGERDOWN || Events.type == SDL_FINGERMOTION || Events.type == SDL_FINGERUP) {
			MT2D_SDL_Touch_Event(&Events);
		}
		else if (Events.type == SDL_JOYAXISMOTION) {
			if (Events.jaxis.axis == 0) {
				// X axis
				GlobalJoystickHandler->Left_X_axis = Events.jaxis.value;
				GlobalJoystickHandler->Left_Modified = true;
				GlobalJoystickHandler->Touched = true;
			}
			else if (Events.jaxis.axis == 1) {
				// Y axis
				GlobalJoystickHandler->Left_Y_axis = Events.jaxis.value;
				GlobalJoystickHandler->Left_Modified = true;
				GlobalJoystickHandler->Touched = true;
			}
		}
		else if (Events.type == SDL_JOYBUTTONDOWN) {
			if (Events.jbutton.button < 4) {
				GlobalJoystickHandler->Button_Pressed[Events.jbutton.button] = true;
				GlobalJoystickHandler->Touched = true;
			}
		}
		else if (Events.type == SDL_QUIT) {
			SDL_Quit();
			exit(0);
		}
	}
}

#endif