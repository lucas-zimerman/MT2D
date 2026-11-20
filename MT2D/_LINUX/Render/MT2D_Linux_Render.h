#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../../MT2D_Terminal_Define.h"
#if defined(linux) && !defined(__ANDROID__)
MT2D_C_BEGIN
void MT2D_Linux_Draw_Window(int which);
void MT2D_Linux_Clear_Main_Window(void);
/* The terminal was resized (curses KEY_RESIZE): what is on it is lost, the next MT2D_Linux_Draw_Window draws the whole screen again */
void MT2D_Linux_Screen_Resized(void);
#ifdef MT2D_LINUX_SIXEL
struct Sprite;
/* Graphics mode: queues an image sprite (virtual 320x240 screen position) to be drawn over the text by the next MT2D_Linux_Draw_Window */
void MT2D_Linux_Sixel_Queue_Image(struct Sprite *img, int x, int y);
#endif
MT2D_C_END
#endif