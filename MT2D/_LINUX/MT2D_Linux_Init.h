#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Linux_Init.h/ MT2D_Linux_Init.cpp

Init of the terminal (curses) backend used by Linux and macOS.
===================================================================*/
#if defined(linux) && !defined(MT2DLIN_INIT_H)
#define MT2DLIN_INIT_H

MT2D_C_BEGIN
bool MT2D_Linux_Init(void);
MT2D_C_END

#endif
