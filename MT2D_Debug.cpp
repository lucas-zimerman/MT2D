#include <Windows.h>
#ifdef _DEBUG
void MT2D_Ide_Printf(char *String) {
	OutputDebugStringA(String);
}
#else
void MT2D_Ide_Printf(char *String) {}
#endif