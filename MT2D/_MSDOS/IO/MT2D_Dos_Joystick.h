#if  defined(__MSDOS__)  || defined(__DOS__) && !defined(MTDOS_JOY_H)
#define MTDOS_JOY_H
void MT2D_Dos_Joystick_Init();
void MT2D_Dos_Joystick_Close();
void MT2D_Dos_Joystick_Update();
#endif
