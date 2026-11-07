/* Input popups: message box, text input, number input and a two option question. */
#include <stdio.h>
#include <stdlib.h>
#include "example.h"
#include <MT2D/MessageBox/MT2D_MessageBox.h>
#include <MT2D/MessageBox/MT2D_MessageBox_With_Result.h>
#include <MT2D/InputBox/MT2D_InputBox_String.h>
#include <MT2D/InputBox/MT2D_InputBox_Integer.h>

/* Clears the screen, prints the lines and shows it. */
static void show(const char *line1, const char *line2)
{
    MT2D_Clear_Main_Window();
    insert_string_on_display((char *)"MT2D input popups", 1, 31, DISPLAY_WINDOW1);
    insert_string_on_display((char *)line1, 4, 4, DISPLAY_WINDOW1);
    insert_string_on_display((char *)line2, 6, 4, DISPLAY_WINDOW1);
    MT2D_Draw_Window(DISPLAY_WINDOW1);
}

int main(int argc, char **argv)
{
    char text[80];
    char *name;
    int age;
    int answer;

    (void)argc;
    (void)argv;
    MT2D_Init();

    show("Welcome! A few popups are coming.", "");
    MT2D_MessageBox((char *)"Hello! Press any key to close this message box.");

    show("Type your name and press ENTER (ESC cancels).", "");
    name = MT2D_InputBox_String((char *)"What is your name?");

    show("Type a number and press ENTER.", "");
    age = MT2D_InputBox_Integer((char *)"How old are you?");

    answer = MT2D_MessageBox_With_Result((char *)"QUESTION", (char *)"Do you like MT2D?", (char *)"YES", (char *)"NO");

    if (name != NULL && name[0] != '\0')
        sprintf(text, "Name  : %s", name);
    else
        sprintf(text, "Name  : (nothing typed)");
    show(text, "");
    sprintf(text, "Age   : %d", age);
    insert_string_on_display(text, 6, 4, DISPLAY_WINDOW1);
    sprintf(text, "Likes : %s", answer == 1 ? "yes!" : "not yet");
    insert_string_on_display(text, 8, 4, DISPLAY_WINDOW1);
    insert_string_on_display((char *)"Press any key to exit", 12, 4, DISPLAY_WINDOW1);
    MT2D_Draw_Window(DISPLAY_WINDOW1);

    free(name);
    MT2D_Keyboard_keytouched();
    return 0;
}
