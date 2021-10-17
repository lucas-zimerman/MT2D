// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.
#if defined(__MSDOS__) || defined(__DOS__)
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D.h> //for max ver/hor parameter.

extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER + 1][MAX_HOR];

// Return: true - it started without any kind of problem, false : there were a problem when MT2D was started
bool MT2D_Dos_Init()
{
        bool output = true;
        return output;
}
#endif
