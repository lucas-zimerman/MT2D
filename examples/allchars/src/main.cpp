/* All characters: shows the 256 CP850 characters of the screen buffer in one 16x16 table.
   The row is the high hex digit and the column the low one, so the character 0xC9 is at row C, column 9. */
#include "example.h"

static const char HEX[] = "0123456789ABCDEF";

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    const int top = 3;   /* first line of the table */
    const int left = 24; /* first column of the table */
    int row, col;

    MT2D_Init();
    MT2D_Clear_Main_Window();

    insert_string_on_display((char *)"All 256 characters (CP850)", 0, 27, DISPLAY_WINDOW1);

    for (col = 0; col < 16; col++) {
        WINDOW1[top - 1][left + 4 + col * 3] = HEX[col];
    }
    for (row = 0; row < 16; row++) {
        WINDOW1[top + row][left + 1] = HEX[row];
        for (col = 0; col < 16; col++) {
            /* 0 and 255 are blank, they are written anyway */
            WINDOW1[top + row][left + 4 + col * 3] = (unsigned char)(row * 16 + col);
        }
    }

    insert_string_on_display((char *)"Press any key to exit", 23, 29, DISPLAY_WINDOW1);

    MT2D_Draw_Window(DISPLAY_WINDOW1);
    MT2D_Keyboard_keytouched();
    return 0;
}
