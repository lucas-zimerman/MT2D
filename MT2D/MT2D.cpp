// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.

#include <stdio.h>

#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D.h>

#ifdef SDL_USE
    #if !defined(__ANDROID__)
        #include <MT2D/SDL/MT2D_SDL_main.h>
        #include <thread>
    #else
    #include <MT2D/SDL/MT2D_SDL_main.h>
    #endif
#elif defined _WIN32
        #include <Windows.h>
#elif defined(__MSDOS__) || defined(__DOS__)
        #include <dos.h>
#elif defined(linux) && !defined(__ANDROID__)
    #include <cstdlib>
    #include <curses.h>
    #include <signal.h>
    #include <locale.h>
#endif
#include <string.h>

unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
unsigned char WINDOW2[MAX_VER + 1][MAX_HOR];

void transfer_window1_to_window2(){
    int i=0;
    int j=0;
    while (i < MAX_VER - 1)
        {
        while (j < MAX_HOR){
              WINDOW2[i][j] = WINDOW1[i][j];
              j++;
         }
         i++;
         j=0;
    }
}

void transfer_window2_to_window1(){
    int i=0;
    int j=0;
    while (i < MAX_VER - 1){
          while ( j < MAX_HOR){
                WINDOW1[i][j] = WINDOW2[i][j];
                j++;
          }
          i++;
          j=0;
    }
}

void insert_string_on_display(char *x, int pos_ver, int pos_hor, bool which){
        int j=0;
        while (x[j] != NULL){
                if(pos_hor>MAX_HOR)
                    break;
                if(which==DISPLAY_WINDOW1)
                    WINDOW1[pos_ver][pos_hor]=x[j];
                else
                    WINDOW2[pos_ver][pos_hor]=x[j];
                j++;
                pos_hor++;
        }
}


void insert_number_on_display(int number, int pos_y,int pos_x, bool display){
        // Based on a generic itoa code without the null termination
        int base=10;
        unsigned char *str=0;
        unsigned char buffer=0;
        int i = 0;
        int j=0;
    bool isNegative = false;
        if(display)
                str=&WINDOW1[pos_y][pos_x];
        else
                str=&WINDOW2[pos_y][pos_x];


    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (number == 0)
    {
        str[i++] = '0';
        return ;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (number < 0 && base == 10)
    {
        isNegative = true;
        number = -number;
    }

    // Process individual digits
    while (number != 0)
    {
        int rem = number % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        number = number/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    // Reverse the string
        i--;
        while(j<i){
                buffer=str[j];
                str[j]=str[i];
                str[i]=buffer;
                j++;
                i--;
        }
}
