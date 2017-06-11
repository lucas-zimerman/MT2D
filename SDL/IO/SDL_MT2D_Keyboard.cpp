#include <MT2D/MT2D_Terminal_Define.h>

#ifdef SDL_USE

#include <MT2D/SDL/MT2D_SDL_Event_Handler.h>
#include <MT2D/MT2D_Keyboard.h>
int SDL_Keyboard_touched();

extern MT2D_SDL_Events MainEvents;

int key_SDL = 0;
bool SHIFT_ON = false;

bool SDL_SpecialKeyHit = false;
unsigned char SDL_EmulatedKeySpecial = 0;
unsigned char SDL_EmulatedKey = 0;
bool SDL_KeyboardHit = false;

int SDL_getkeyboard() {//keyboardhit is the function that gets the keycode, this function only sends the right key code to the progrma
	int key_code = 0;
	if (SDL_KeyboardHit == false) {// if not touched, lock the software till some key is touched
		while (SDL_Keyboard_touched() == false);
	}
	if (SDL_SpecialKeyHit) {
		SDL_SpecialKeyHit = false;
		key_code = SDL_EmulatedKeySpecial;
	}
	else {
		SDL_EmulatedKeySpecial = 0;
		key_code = SDL_EmulatedKey;
		SDL_KeyboardHit = false;
	}
	//
	return key_code;
}


int SDL_Keyboard_touched() {//check if a key was pressed, and if it was, get the key and store all the information so SDL_getkeyboard can handle what key code should MT2D send
	int output = 0;
	MT2D_SDL_Event_Handler();
	if (MainEvents.Keyboard_Pressed == true) {
		MainEvents.Keyboard_Pressed = false;
		output = true;
		SDL_KeyboardHit = true;
		//printf("tecla pra baixo,");
		//char 8 - 122 are equals, no need for conversion;
		//there's only lower case alphabet, so convert if shift was pressed..
		if (MainEvents.Keyboard.keysym.sym >= 97 && MainEvents.Keyboard.keysym.sym <= 122) {
			//printf(" letras...\n");
			if (MainEvents.Keyboard.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
			{
				SDL_EmulatedKey = MainEvents.Keyboard.keysym.sym - 32;//transforma em maiuscula
			}
			else {
				SDL_EmulatedKey = MainEvents.Keyboard.keysym.sym;
			}
		}
		else if (MainEvents.Keyboard.keysym.sym <97) {//espero ser sempre maior que zero
			//printf(" caracter de valor menor a 97\n");

			SDL_EmulatedKey = MainEvents.Keyboard.keysym.sym;
		}
		else if (MainEvents.Keyboard.keysym.sym >= SDLK_F1 && MainEvents.Keyboard.keysym.sym <= SDLK_F12) {//converte as teclas F12
			//printf("Tecla do tipo Fx\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = MainEvents.Keyboard.keysym.sym - 1073741823; // SDLK_F1 - 59
		}
		else if (MainEvents.Keyboard.keysym.sym == SDLK_UP) {
			//printf(" seta cima\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = arrow_up_pressed;
		}
		else if (MainEvents.Keyboard.keysym.sym == SDLK_DOWN) {
			//printf(" seta baixo\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = arrow_down_pressed;
		}
		else if (MainEvents.Keyboard.keysym.sym == SDLK_LEFT) {
			//printf(" seta esquerda\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = arrow_left_pressed;
		}
		else if (MainEvents.Keyboard.keysym.sym == SDLK_RIGHT) {
			//printf(" seta direita\n");
			SDL_EmulatedKeySpecial = 224;
			SDL_EmulatedKey = arrow_right_pressed;
		}
		if (SDL_EmulatedKeySpecial == 224) {
			SDL_SpecialKeyHit = true;
		}
	}
	return output;
}

#else
	#warning define "SDL_USE" must be created to use the SDL keyboard code

#endif