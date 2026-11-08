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

#define DISPLAY_WINDOW1 1
#define DISPLAY_WINDOW2 0
#define MAX_HOR 80
#define MAX_VER 25

MT2D_C_BEGIN
extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER + 1][MAX_HOR];

void transfer_window1_to_window2(void);
void transfer_window2_to_window1(void);
void insert_string_on_display(char *x, int pos_ver, int pos_hor, bool which);
void insert_number_on_display(int number, int pos_y, int pos_x, bool display);
MT2D_C_END

#endif
