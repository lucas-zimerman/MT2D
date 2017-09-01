/*==============================================================================================
									Keyboard.cpp / Keyboard.h

								Created By Lucas Zimerman Fraulob
	With this code you'll be able to make use of the Keyboard for your software on an universal way
	For now MS-DOS, Windows and linux had their keyboard code standardized, for key codes, check getch output
	on Windows or the ansi struct.
	NOTE: there's also an experimental SDL code for the keyboard part, so it may not propertly work for now

	List of functions and what they do and their returns:

	-Keynboard_touched()
		Description: Check if the keyboard was touched (good for loops not getting stucked on keyboard_keytouched.
		Return: 1 if the keyboard was touched, 0 if the keyboard wasn't touched.

	-Keyboard_keytouched()
		Description:	Return what key was pressed, if none was pressed, it'll wait for a key,
			also, if a special key was pressed like _key, this function will make two returns, one indicating that
			_key was pressed and then witch _key was pressed.
		Return: Key pressed (ansi char)
	NOTE: if you want to interact with multiple keys at the same time, please use the joystick struct
	because it also works with keyboard.
==============================================================================================*/
#define arrow_key_pressed 224
#define arrow_up_pressed 72
#define arrow_down_pressed 80
#define arrow_left_pressed 75
#define arrow_right_pressed 77
#define back_space_pressed 8

#define enter_pressed 13
#define tab_pressed 9
//#ifdef __MSDOS__
#define escape_pressed 27
//#else
//#define esc 1
//#endif

//#define ç 135

#ifndef __MSDOS__
#include "MT2D_Terminal_Define.h"
#else
#include "../../MT2D/MT2D_Terminal_Define.h"
#endif

#if defined(SDL_USE)
#include "SDL/IO/SDL_MT2D_Keyboard.h"
#define MT2D_Keyboard_keytouched SDL_getkeyboard
#define MT2D_Keyboard_touched  SDL_Keyboard_touched
#elif defined(_WIN32)
#include "_WINDOWS\IO\MT2D_Win_Keyboard.h"
#define MT2D_Keyboard_touched  MT2D_Win_Keyboard_touched
#define MT2D_Keyboard_keytouched MT2D_Win_Keyboard_keytouched
#elif defined(__MSDOS__)
#include "..\..\MT2D\_MSDOS\IO\MT2D_Dos_Keyboard.h"
#define MT2D_Keyboard_touched  MT2D_Dos_Keyboard_touched
#define MT2D_Keyboard_keytouched MT2D_Dos_Keyboard_keytouched
#elif defined(linux) && !defined(__ANDROID__)
#include "_LINUX/IO/MT2D_Linux_Keyboard.h"
#define MT2D_Keyboard_touched  MT2D_Linux_Keyboard_touched
#define MT2D_Keyboard_keytouched MT2D_Linux_Keyboard_keytouched
#else
#error "MT2D_Keyboard: There's no code support for your operational system"
#endif

