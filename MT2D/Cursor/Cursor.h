// Cursor.h created by: Lucas Zimerman Fraulob
// This file includes the basic controls to interact with the cursor (normally used by other MT2D functions)
//

#ifndef MT2D_CURSOR_H
#define MT2D_CURSOR_H

#ifndef __MSDOS__
#include "MT2D_Terminal_Define.h"
#else
#include "../../MT2D/MT2D_Terminal_Define.h"
#endif

#if defined(SDL_USE)
#define MT2D_Cursor_goto_topscreen 0
#define MT2D_Cursor_Set_Position 0
#elif defined(WINDOWS_TARGET)
#include "..\_WINDOWS\MT2D_Win_Cursor.h"
#define MT2D_Cursor_goto_topscreen MT2D_Win_Cursor_goto_topscreen
#define MT2D_Cursor_Set_Position MT2D_Win_Cursor_Set_Position
#elif defined(__MSDOS__)
#include "..\..\MT2D\_MSDOS\MT2D_Dos_Cursor.h"
#define MT2D_Cursor_goto_topscreen MT2D_Dos_Cursor_goto_topscreen
#define MT2D_Cursor_Set_Position MT2D_Dos_Cursor_Set_Position
#elif defined(linux)
#include "../_LINUX/MT2D_Linux_Cursor.h"
#define MT2D_Cursor_goto_topscreen MT2D_Linux_Cursor_goto_topscreen
#define MT2D_Cursor_Set_Position MT2D_Linux_Cursor_Set_Position
#else
void MT2D_Cursor_goto_topscreen();
void MT2D_Cursor_Set_Position(int line, int column);
#endif

#endif