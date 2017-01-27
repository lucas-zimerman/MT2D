#include "MT2D\MT2D.h"

void MT2D_Create_Window_Layout(int pos_x, int pos_y, int tam_x, int tam_y, bool shadow, bool title, bool option, char background, int window) {
	int post_x = pos_x + 1;
	int post_y = pos_y;
	while (post_x <tam_x + pos_x && post_x<MAX_HOR) {
		if (window == DISPLAY_WINDOW1) {
			WINDOW1[post_y][post_x] = 205;
			WINDOW1[post_y + tam_y][post_x] = 205;
		}
		else {
			WINDOW2[post_y][post_x] = 205;
			WINDOW2[post_y + tam_y][post_x] = 205;
		}post_x++;
	}
	post_x = pos_x;
	post_y = pos_y + 1;
	while (post_y <tam_y + pos_y && post_y<MAX_VER) {//Ok
		if (window == DISPLAY_WINDOW1) {
			WINDOW1[post_y][post_x] = 186;
			WINDOW1[post_y][post_x + tam_x] = 186;
		}
		else {
			WINDOW2[post_y][post_x] = 186;
			WINDOW2[post_y][post_x + tam_x] = 186;
		}post_y++;
	}
	if (window == DISPLAY_WINDOW2) {
		WINDOW2[pos_y][pos_x] = 201; WINDOW2[pos_y][pos_x + tam_x] = 187;
		WINDOW2[pos_y + tam_y][pos_x] = 200; WINDOW2[pos_y + tam_y][pos_x + tam_x] = 188;
	}
	else {
		WINDOW1[pos_y][pos_x] = 201; WINDOW1[pos_y][pos_x + tam_x] = 187;
		WINDOW1[pos_y + tam_y][pos_x] = 200; WINDOW1[pos_y + tam_y][pos_x + tam_x] = 188;
	}
	if (shadow) {
		post_x = pos_x + tam_x + 1;
		post_y = pos_y + 1;

		if (post_x<MAX_HOR) {
			while (post_y <= pos_y + tam_y + 1 && post_y<MAX_VER) {
				if (window == DISPLAY_WINDOW2)	WINDOW2[post_y][post_x] = 176;
				else			WINDOW1[post_y][post_x] = 176;
				post_y++;
			}
		}
		post_x = pos_x + 1;
		post_y = pos_y + tam_y + 1;
		if (post_y<MAX_VER) {
			while (post_x <= pos_x + tam_x + 1 && post_x<MAX_HOR) {
				if (window == DISPLAY_WINDOW2)	WINDOW2[post_y][post_x] = 176;
				else			WINDOW1[post_y][post_x] = 176;
				post_x++;
			}
		}
	}
	post_x = pos_x + 1;
	post_y = pos_y + 1;
	while (post_y<pos_y + tam_y && post_y <= MAX_VER) {//Transfere window1 para window2
		while (post_x<pos_x + tam_x && post_x<MAX_HOR) {
			if (window == DISPLAY_WINDOW2)WINDOW2[post_y][post_x] = background;
			else WINDOW1[post_y][post_x] = background;
			post_x++;
		}
		post_y++;
		post_x = pos_x + 1;
	}
	if (title) {
		post_y = pos_y + 2;
		post_x = pos_x + 1;
		while (post_x<pos_x + tam_x && post_x<MAX_HOR) {
			if (window == DISPLAY_WINDOW2)WINDOW2[post_y][post_x] = 205;
			else WINDOW1[post_y][post_x] = 205;
			post_x++;
		}
	}
	if (option) {
		post_y = pos_y + tam_y - 2;
		post_x = pos_x + 1;
		while (post_x<pos_x + tam_x && post_x<MAX_HOR) {
			if (window == DISPLAY_WINDOW2)WINDOW2[post_y][post_x] = 205;
			else WINDOW1[post_y][post_x] = 205;
			post_x++;
		}
	}

}