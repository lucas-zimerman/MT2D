#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Linux_Init.h/ MT2D_Linux_Init.cpp

Init of the terminal (curses) backend used by Linux and macOS.
===================================================================*/
#if defined(linux) && !defined(MT2DLIN_INIT_H)
#define MT2DLIN_INIT_H

#include <MT2D/MT2D_Settings.h>

MT2D_C_BEGIN
bool MT2D_Linux_Init(void);
MT2D_Result MT2D_Linux_Set_Settings_Argv(int argc, const int *argv);
MT2D_Result MT2D_Linux_Set_Settings(MT2D_Setting_Operation operation, int param1, int param2);
MT2D_C_END

#endif
