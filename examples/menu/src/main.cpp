/* Menu: MT2D_Menu draws a window with up to 18 options and returns the chosen one (1..n). */
#include <stdio.h>
#include "example.h"

static void screen(const char *message)
{
    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"MT2D menu example", 1, 31, DISPLAY_WINDOW1);
    insert_string_on_display((char *)message, 22, 3, DISPLAY_WINDOW1);
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

int main(int argc, char **argv)
{
    int option = 0;
    char message[80];

    (void)argc;
    (void)argv;
    MT2D_Init();

    screen("Pick something from the menu.");
    while (option != 4) {
        /* arrows + ENTER or the option number selects, the menu draws on top of WINDOW1 */
        option = MT2D_Short_Menu((char *)"MAIN MENU", 20, 10, 5, true, ' ', '>',
                                 (char *)"Start game", (char *)"Options", (char *)"Credits", (char *)"Exit");
        switch (option) {
        case 1: screen("You chose: Start game"); break;
        case 2: screen("You chose: Options"); break;
        case 3: screen("You chose: Credits (MT2D by Lucas Zimerman Fraulob)"); break;
        default: break;
        }
        if (option != 4) {
            sprintf(message, "Option %d selected, press any key to open the menu again.", option);
            insert_string_on_display(message, 20, 3, DISPLAY_WINDOW1);
            MT2D_Draw_Window(DISPLAY_WINDOW1);
            MT2D_Keyboard_keytouched();
        }
    }
    return 0;
}
