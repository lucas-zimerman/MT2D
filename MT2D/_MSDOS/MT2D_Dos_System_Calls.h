#if defined(__MSDOS__) || defined(__DOS__) && !defined(MTDOS_SYSCALL_H)
#define MTDOS_SYSCALL_H
void MT2D_Dos_System_Delay(int time_millisecond);
void MT2D_Dos_System_Beep(int frequency, int time_milisecond);
#endif
