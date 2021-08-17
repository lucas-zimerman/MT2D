#if  defined(__EMSCRIPTEN__) && !defined(EM_JOY_H)
#define EM_JOY_H
void MT2D_Ems_Joystick_Init();
void MT2D_Ems_Joystick_Close();
void MT2D_Ems_Joystick_Update();
#endif