#if !defined(__ANDROID__)
#include <MT2D/MT2D.h>
#elif defined(__MSDOS__)
#include <MT2D/MT2D.h>
#else
#include "MT2D.h"
#endif

void MT2D_Create_Window_Layout(int pos_x, int pos_y, int tam_x, int tam_y, bool shadow, bool title, bool option, char background, unsigned char (*window)[MAX_HOR]) {
	int post_x = pos_x + 1;
	int post_y = pos_y;
	window = MT2D_WINDOW_OR_MAIN(window);
	while (post_x <tam_x + pos_x && post_x<MAX_HOR) {
		window[post_y][post_x] = 205;
		window[post_y + tam_y][post_x] = 205;
		post_x++;
	}
	post_x = pos_x;
	post_y = pos_y + 1;
	while (post_y <tam_y + pos_y && post_y<MAX_VER) { /* Ok */
		window[post_y][post_x] = 186;
		window[post_y][post_x + tam_x] = 186;
		post_y++;
	}
	window[pos_y][pos_x] = 201; window[pos_y][pos_x + tam_x] = 187;
	window[pos_y + tam_y][pos_x] = 200; window[pos_y + tam_y][pos_x + tam_x] = 188;
	if (shadow) {
		post_x = pos_x + tam_x + 1;
		post_y = pos_y + 1;

		if (post_x<MAX_HOR) {
			while (post_y <= pos_y + tam_y + 1 && post_y<MAX_VER) {
				window[post_y][post_x] = 176;
				post_y++;
			}
		}
		post_x = pos_x + 1;
		post_y = pos_y + tam_y + 1;
		if (post_y<MAX_VER) {
			while (post_x <= pos_x + tam_x + 1 && post_x<MAX_HOR) {
				window[post_y][post_x] = 176;
				post_x++;
			}
		}
	}
	post_x = pos_x + 1;
	post_y = pos_y + 1;
	while (post_y<pos_y + tam_y && post_y <= MAX_VER) {
		while (post_x<pos_x + tam_x && post_x<MAX_HOR) {
			window[post_y][post_x] = background;
			post_x++;
		}
		post_y++;
		post_x = pos_x + 1;
	}
	if (title) {
		post_y = pos_y + 2;
		post_x = pos_x + 1;
		while (post_x<pos_x + tam_x && post_x<MAX_HOR) {
			window[post_y][post_x] = 205;
			post_x++;
		}
	}
	if (option) {
		post_y = pos_y + tam_y - 2;
		post_x = pos_x + 1;
		while (post_x<pos_x + tam_x && post_x<MAX_HOR) {
			window[post_y][post_x] = 205;
			post_x++;
		}
	}

}
