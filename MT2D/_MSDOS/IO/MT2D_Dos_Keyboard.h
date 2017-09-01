#if defined(__MSDOS__) && !defined(MTDOS_KEYH)
#define MTDOS_KEYH
#include "../../MT2D/MT2D_Terminal_Define.h"
int MT2D_Dos_Keyboard_keytouched();
bool MT2D_Dos_Keyboard_touched();

#endif