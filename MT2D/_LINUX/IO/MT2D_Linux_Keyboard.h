#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include "../../MT2D_Terminal_Define.h"
#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)

MT2D_C_BEGIN
bool MT2D_Linux_Keyboard_touched(void);
int	MT2D_Linux_Keyboard_keytouched(void);
MT2D_C_END

#endif