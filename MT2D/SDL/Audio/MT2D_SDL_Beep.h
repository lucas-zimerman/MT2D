#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
SDL_MT2DBeep.h SDL_MT2DBeep.cpp created by: Lucas Zimerman Fraulob

	Here, you'll see the implementation of the PC SPEAKER code 
	in SDL, the only function required is SDL_Beep, and the usage
	is similar to the normal MT2D Beep code, you need a frequency 
	and a delayed timing that the beep will occur
===================================================================*/

MT2D_C_BEGIN
void SDL_Beep(int frequency, int TimeMS);
MT2D_C_END