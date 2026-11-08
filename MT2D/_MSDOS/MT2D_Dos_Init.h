#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Win_Init.h/ MT2D_Win_Init.cpp created By Lucas Zimerman Fraulob

This file handles the Windows specific code required to init the MT2D
engine.
===================================================================*/




#if defined(__MSDOS__) || defined(__DOS__) && !defined(MT2DDOS_INIT_H)
#define MT2DDOS_INIT_H

MT2D_C_BEGIN
bool MT2D_Dos_Init(void);
MT2D_C_END

#endif
