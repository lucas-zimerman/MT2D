/*===================================================================
	Audio_core.h Audio_core.h created by: Lucas Zimerman Fraulob
	
	Here's where you're going to play complex music files 
	(.wav,.mp3,...), you have two options of playing sounds.
	1) Music playing: the sound is always played into a loop.
	2) Audio playing: the sound is only played once.
	
	MT2D Start Audio System
		Description: Start the audio system
	MT2D Close Audio System
		Description: Closes the audio system
	MT2D Play Audio
		Description: Plays a sound file from a given addres
	MT2D Play Music
		Description: Plays a sound file in a loop.
	
===================================================================*/
void MT2D_Audio_Init();
void MT2D_Audio_Close();
void MT2D_Play_Audio(char *Name);
void MT2D_Play_Music(char *Name);
