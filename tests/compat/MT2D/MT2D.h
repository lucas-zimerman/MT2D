// Test-only replacement for MT2D/MT2D.h.
//
// The real header pulls in the platform init headers through backslash include
// paths (e.g. "_LINUX\MT2D_Linux_Init.h"), which do not resolve on Linux. The
// tests only need the display buffers and their helpers, so this declares just
// that part. It is found before the real header because tests/compat is first
// on the include path.
#ifndef MT2D_TEST_COMPAT_MT2D_H
#define MT2D_TEST_COMPAT_MT2D_H

#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/ObjectCore/MT2D_CDecl.h> // C linkage macros and "bool" (the engine sources are C)

#include <MT2D/MT2D_Window.h> // WINDOW1, MT2D_Window, MAX_HOR, MAX_VER, DISPLAY_WINDOW1

MT2D_C_BEGIN

void MT2D_Window_Save_Area(unsigned char *buffer, int x, int y, int w, int h, unsigned char (*window)[MAX_HOR]);
void MT2D_Window_Restore_Area(unsigned char *buffer, int x, int y, int w, int h, unsigned char (*window)[MAX_HOR]);
void insert_string_on_display(char *x, int pos_ver, int pos_hor, unsigned char (*window)[MAX_HOR]);
void insert_number_on_display(int number, int pos_y, int pos_x, unsigned char (*window)[MAX_HOR]);
MT2D_C_END

#endif
