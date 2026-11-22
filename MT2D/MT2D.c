/* MT2D Created By Lucas Zimerman Fraulob */
/* This is the core of MT2D engine, with only this .cpp you can do everything */
/* This file is related to almost all MT2D video interface, like draw, clear and update. */

#include <stdio.h>

#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D.h>

#ifdef SDL_USE
    #if !defined(__ANDROID__)
        #include <MT2D/SDL/MT2D_SDL_main.h>
    #else
    #include <MT2D/SDL/MT2D_SDL_main.h>
    #endif
#elif defined _WIN32
        #include <Windows.h>
#elif defined(__MSDOS__) || defined(__DOS__)
        #include <dos.h>
#elif defined(linux) && !defined(__ANDROID__)
    #include <stdlib.h>
    #include <curses.h>
    #include <signal.h>
    #include <locale.h>
#endif
#include <string.h>

unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
/* Copies the cells of window (WINDOW1 when NULL) inside the w x h rectangle at (x, y) into buffer (w * h bytes).
   Cells outside the screen are skipped. Popups use it to put back what they covered. */
void MT2D_Window_Save_Area(unsigned char *buffer, int x, int y, int w, int h, unsigned char (*window)[MAX_HOR]) {
        int i, j;
        window = MT2D_WINDOW_OR_MAIN(window);
        for (i = 0; i < h; i++) {
                if (y + i < 0 || y + i > MAX_VER) continue;
                for (j = 0; j < w; j++) {
                        if (x + j < 0 || x + j >= MAX_HOR) continue;
                        buffer[i * w + j] = window[y + i][x + j];
                }
        }
}

/* Puts back on window (WINDOW1 when NULL) the cells saved by MT2D_Window_Save_Area with the same rectangle. */
void MT2D_Window_Restore_Area(unsigned char *buffer, int x, int y, int w, int h, unsigned char (*window)[MAX_HOR]) {
        int i, j;
        window = MT2D_WINDOW_OR_MAIN(window);
        for (i = 0; i < h; i++) {
                if (y + i < 0 || y + i > MAX_VER) continue;
                for (j = 0; j < w; j++) {
                        if (x + j < 0 || x + j >= MAX_HOR) continue;
                        window[y + i][x + j] = buffer[i * w + j];
                }
        }
}

void insert_string_on_display(char *x, int pos_ver, int pos_hor, unsigned char (*window)[MAX_HOR]){
        int j=0;
        window = MT2D_WINDOW_OR_MAIN(window);
        while (x[j] != '\0'){
                if(pos_hor>MAX_HOR)
                    break;
                window[pos_ver][pos_hor]=x[j];
                j++;
                pos_hor++;
        }
}


void insert_number_on_display(int number, int pos_y,int pos_x, unsigned char (*window)[MAX_HOR]){
/* Based on a generic itoa code without the null termination */
        int base=10;
        unsigned char *str=0;
        unsigned char buffer=0;
        int i = 0;
        int j=0;
    bool isNegative = false;
        str=&MT2D_WINDOW_OR_MAIN(window)[pos_y][pos_x];


    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (number == 0)
    {
        str[i++] = '0';
        return ;
    }

/* In standard itoa(), negative numbers are handled only with */
/* base 10. Otherwise numbers are considered unsigned. */
    if (number < 0 && base == 10)
    {
        isNegative = true;
        number = -number;
    }

/* Process individual digits */
    while (number != 0)
    {
        int rem = number % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        number = number/base;
    }

/* If number is negative, append '-' */
    if (isNegative)
        str[i++] = '-';

/* Reverse the string */
        i--;
        while(j<i){
                buffer=str[j];
                str[j]=str[i];
                str[i]=buffer;
                j++;
                i--;
        }
}
