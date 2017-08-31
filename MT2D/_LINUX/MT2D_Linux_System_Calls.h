#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE) && !defined(MTLIN_SYSCALL_H)
#define MTLIN_SYSCALL_H
void MT2D_Linux_System_Delay(int time_millisecond);
void MT2D_Linux_System_Beep(int frequency, int time_milisecond);
#endif