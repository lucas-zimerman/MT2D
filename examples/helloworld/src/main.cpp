/* Hello World: write a string on the screen buffer, show it and wait for a key. */
#include "example.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    MT2D_Init();
    MT2D_Clear_Main_Window();

    insert_string_on_display((char *)"Hello, MT2D!", 10, 34, DISPLAY_WINDOW1);
    insert_string_on_display((char *)"Press any key to exit", 12, 29, DISPLAY_WINDOW1);

    MT2D_Draw_Window(DISPLAY_WINDOW1);
    MT2D_Keyboard_keytouched();
    return 0;
}
