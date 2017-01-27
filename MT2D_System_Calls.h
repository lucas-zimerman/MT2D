/*===================================================================
system_calls.h/ system_calls.cpp created By Lucas Zimerman Fraulob

This file will contain some simple hardware calls

	List of functions and what they do and their returns:

	-Delay()
		Description: freeze the code by x miliseconds,
		NOTE: this will not freeze if the software make use of threads, only the thread with the freeze code
		will be delayed.
		Return: none
	-pc_speaker_beep()
		Description: Make a beep with a given frequency on pc speaker, (MS-DOS and WINDOWS ONLY), or into the sound card (Using SDL [works under windows, linux])
			NOTE: since windows vista, the pc speaker code is emulated, so it'll not sound as good as XP or MS-DOS (So better use the SDL version)
			for fast beeps.
		Return: none

===================================================================*/


void MT2D_System_Delay(int time_milisecond);
void MT2D_System_Beep(int frequency, int time_milisecond);
