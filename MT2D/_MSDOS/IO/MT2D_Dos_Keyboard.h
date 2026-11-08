#include <MT2D/ObjectCore/MT2D_CDecl.h>
#if defined(__MSDOS__) || defined(__DOS__) && !defined(MTDOS_KEYH)
#define MTDOS_KEYH
#include <MT2D/MT2D_Terminal_Define.h>
MT2D_C_BEGIN
int MT2D_Dos_Keyboard_keytouched(void);
bool MT2D_Dos_Keyboard_touched(void);
MT2D_C_END

#endif
