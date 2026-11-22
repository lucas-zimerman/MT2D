#include <MT2D/ObjectCore/MT2D_CDecl.h>
/*===================================================================
MT2D_Linux_Audio.h / MT2D_Linux_Audio.c

Sound effects and music for the Linux terminal backend (MT2D_TERMINAL_USE_AUDIO,
see MT2D_Terminal_Define.h), made by command line players: no audio library is linked.

  Sound effects: a short lived "paplay || pw-play || aplay" per sound, so sounds overlap freely.
  Music:         a loop of "paplay || pw-play || ffplay || aplay" in its own process group,
                 one song at a time, a new song replaces the old one.
Files are .wav or .ogg (aplay only plays .wav, paplay and pw-play need libsndfile for .ogg, ffplay does
anything). IMF songs (the MS-DOS music) are not played. A file is looked for as given and then in "d/".
The players run with their input and output on /dev/null, so they never draw on the terminal.
Without any player the sounds are silent. The audio plays on the machine that runs the game (not on the
other side of an ssh connection).
===================================================================*/
#ifndef MT2DLIN_AUDIO_H
#define MT2DLIN_AUDIO_H

#include <MT2D/MT2D_Terminal_Define.h>

#ifdef MT2D_LINUX_AUDIO
MT2D_C_BEGIN
int MT2D_Linux_Audio_Init(void); /* always 1, a missing player is only found when a sound is played */
void MT2D_Linux_Audio_Close(void); /* stops the music */
void MT2D_Linux_Play_Sound(const char *name);
void MT2D_Linux_Play_Music(const char *name);
/* the block is a whole .wav / .ogg file image, it is freed (free()) after use when "release" is true */
void MT2D_Linux_Play_Sound_From_Memory(void *block, unsigned int size, bool release);
void MT2D_Linux_Play_Music_From_Memory(void *block, unsigned int size, bool release);
MT2D_C_END
#endif

#endif
