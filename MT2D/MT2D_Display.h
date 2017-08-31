#include "MT2D_Terminal_Define.h"

#if defined(SDL_USE)
#include "SDL/Render/MT2D_SDL_Render.h"
#define MT2D_Clear_Main_Window MT2D_SDL_Clear_Main_Window
#define MT2D_Draw_Window MT2D_SDL_Draw_Window
#elif defined(_WIN32)
#include "_WINDOWS\Render\MT2D_Win_Render.h"
#define MT2D_Clear_Main_Window MT2D_Win_Draw_Window
#define MT2D_Draw_Window MT2D_Win_Clear_Main_Window
#elif defined(__MSDOS__)
#include "_MSDOS\Render\MT2D_Dos_Render.h"
#define MT2D_Clear_Main_Window MT2D_Dos_Draw_Window
#define MT2D_Draw_Window MT2D_Dos_Clear_Main_Window
#elif defined(linux) && !defined(__ANDROID__)
#include "_LINUX/Render/MT2D_Linux_Render.h"
#define MT2D_Clear_Main_Window MT2D_Linux_Draw_Window
#define MT2D_Draw_Window MT2D_Linux_Clear_Main_Window
#else
void MT2D_Clear_Main_Window();
void MT2D_Draw_Window(int which);
#endif

