/*==============================================================================================
SDL_MT2D_Keyboard.cpp / SDL_MT2D_Keyboard.h

Created By Lucas Zimerman Fraulob
With this code you'll be able to make use of the Keyboard for your software on an universal way
For now MS-DOS, Windows and linux had their keyboard code standardized, for key codes, check getch output
on Windows or the ansi struct.

List of functions and what they do and their returns:

-Keynboard_touched()
Description: Check if the keyboard was touched (good for loops not getting stucked on keyboard_keytouched.
Return: 1 if the keyboard was touched, 0 if the keyboard wasn't touched.

-Keyboard_keytouched()
Description:	Return what key was pressed, if none was pressed, it'll wait for a key,
also, if a special key was pressed like _key, this function will make two returns, one indicating that
_key was pressed and then witch _key was pressed.
Return: Key pressed (ansi char)
==============================================================================================*/

int SDL_getkeyboard();
int SDL_Keyboard_touched();