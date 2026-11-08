// the "linux" macro (also used for macOS) is defined by this header, include it before the #if below
#include "../MT2D_Terminal_Define.h"
#if defined(linux) && !defined(__ANDROID__) && !defined(SDL_USE)
#include "../MT2D_Terminal_Define.h"
#include <MT2D/ObjectCore/MT2D_CDecl.h> /* bool */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/kd.h>
#endif

void MT2D_Linux_System_Delay(int time_millisecond) {
	usleep((useconds_t)time_millisecond * 1000);
}

/* Writes a mono 8 bit 8000 Hz square wave .wav file. */
static bool MT2D_Linux_Write_Tone(const char *path, int frequency, int time_milisecond) {
	const int rate = 8000;
	unsigned int samples = (unsigned int)rate * time_milisecond / 1000;
	unsigned int dataSize = samples, riffSize = 36 + dataSize, byteRate = rate, fmtSize = 16;
	unsigned short pcm = 1, channels = 1, align = 1, bits = 8;
	unsigned int i, period = frequency > 0 ? rate / frequency : 1;
	FILE *f = fopen(path, "wb");
	if (!f) return false;
	if (period < 2) period = 2;
	fwrite("RIFF", 1, 4, f); fwrite(&riffSize, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f);
	fwrite(&fmtSize, 4, 1, f); fwrite(&pcm, 2, 1, f); fwrite(&channels, 2, 1, f);
	fwrite(&rate, 4, 1, f); fwrite(&byteRate, 4, 1, f); fwrite(&align, 2, 1, f); fwrite(&bits, 2, 1, f);
	fwrite("data", 1, 4, f); fwrite(&dataSize, 4, 1, f);
	for (i = 0; i < samples; i++) {
		/* short fade in/out so notes do not click */
		unsigned int edge = i < samples - i ? i : samples - i;
		int amp = edge < 80 ? (int)(edge * 40 / 80) : 40;
		fputc(128 + ((i % period) < period / 2 ? amp : -amp), f);
	}
	fclose(f);
	return true;
}

/*
	There is no PC speaker on modern unix systems, so:
	1) try the real one (Linux text console only),
	2) otherwise render the note to a temporary .wav and play it with a command line player
	   (afplay on macOS, paplay/aplay/pw-play on Linux),
	3) otherwise just wait, so the timing of the songs stays the same.
*/
void MT2D_Linux_System_Beep(int frequency, int time_milisecond) {
	if (frequency <= 0) {
		usleep((useconds_t)time_milisecond * 1000);
		return;
	}
#ifdef __linux__
	if (ioctl(STDOUT_FILENO, KIOCSOUND, 1193180 / frequency) == 0) {
		usleep((useconds_t)time_milisecond * 1000);
		ioctl(STDOUT_FILENO, KIOCSOUND, 0);
		return;
	}
#endif
	{
		char path[64];
		char cmd[160];
		snprintf(path, sizeof(path), "/tmp/mt2d_beep_%d.wav", (int)getpid());
		if (MT2D_Linux_Write_Tone(path, frequency, time_milisecond)) {
#ifdef __APPLE__
			snprintf(cmd, sizeof(cmd), "afplay %s >/dev/null 2>&1", path);
#else
			snprintf(cmd, sizeof(cmd), "(paplay %s || pw-play %s || aplay -q %s) >/dev/null 2>&1", path, path, path);
#endif
			if (system(cmd) != 0) usleep((useconds_t)time_milisecond * 1000);
			remove(path);
		} else {
			usleep((useconds_t)time_milisecond * 1000);
		}
	}
}
#endif
