/* MT2D audio of the Linux terminal backend made by command line players, see MT2D_Linux_Audio.h. */
#include "../../MT2D_Terminal_Define.h"

#ifdef MT2D_LINUX_AUDIO
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600 /* fork, setpgid, mkstemp (strict -std=c89 builds) */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include "MT2D_Linux_Audio.h"

/* $1 is the file: it never goes through the shell parser, so any name is safe */
#define PLAY_CHAIN "paplay \"$1\" || pw-play \"$1\" || aplay -q \"$1\""
#define MUSIC_CHAIN "paplay \"$1\" || pw-play \"$1\" || ffplay -nodisp -autoexit -loglevel quiet \"$1\" || aplay -q \"$1\""
#ifdef __APPLE__
#undef PLAY_CHAIN
#define PLAY_CHAIN "afplay \"$1\""
#undef MUSIC_CHAIN
#define MUSIC_CHAIN "afplay \"$1\""
#endif

static pid_t music_pid = 0;
static char music_temp[64]; /* temp file of a song played from memory, "" when there is none */
static int atexit_set = 0;

/* The file as given, or in the "d" folder (the converted assets of the MS-DOS build). 0 when there is none. */
static int Find_File(const char *name, char *out, size_t outSize) {
	if (!name || !name[0]) return 0;
	if (access(name, R_OK) == 0 && strlen(name) < outSize) {
		strcpy(out, name);
		return 1;
	}
	if (strlen(name) + 3 <= outSize) {
		strcpy(out, "d/");
		strcat(out, name);
		if (access(out, R_OK) == 0) return 1;
	}
	return 0;
}

/* Runs "sh -c script sh path" with no input and no output. Child side only, never returns. */
static void Exec_Script(const char *script, const char *path) {
	int fd = -1;
	fd = open("/dev/null", O_RDWR);
	if (fd >= 0) {
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
		if (fd > 2) close(fd);
	}
	execl("/bin/sh", "sh", "-c", script, "sh", path, (char *)0);
	_exit(127);
}

/* A sound effect: forked twice so the game never has to wait for it nor leaves a zombie. */
static void Spawn_Effect(const char *script, const char *path) {
	pid_t child = fork();
	if (child < 0) return;
	if (child == 0) {
		pid_t player = fork();
		if (player == 0) Exec_Script(script, path);
		_exit(0);
	}
	waitpid(child, NULL, 0);
}

static void Stop_Music(void) {
	if (music_pid > 0) {
		kill(-music_pid, SIGTERM); /* the shell and the player: the whole process group */
		waitpid(music_pid, NULL, 0);
		music_pid = 0;
	}
	if (music_temp[0]) {
		remove(music_temp);
		music_temp[0] = '\0';
	}
}

static void Start_Music(const char *path) {
	pid_t pid;
	if (!atexit_set) {
		atexit_set = 1;
		atexit(Stop_Music);
	}
	Stop_Music();
	pid = fork();
	if (pid < 0) return;
	if (pid == 0) {
		setpgid(0, 0); /* its own group, so one kill stops the loop and the player */
#ifdef __linux__
		prctl(PR_SET_PDEATHSIG, SIGTERM); /* the loop does not outlive a game that is killed */
#endif
		/* The player runs in the background and the shell waits for it ("wait" is interrupted by a signal at once, a
		   foreground player would delay the trap until the song ends). On SIGTERM, sent by Stop_Music or by the kernel when
		   the game dies (also kill -9, a closed terminal), the trap ends the whole group: the shell and the player. */
		Exec_Script("trap 'trap - TERM; kill 0' TERM; while :; do { " MUSIC_CHAIN "; } & wait $! || exit 1; done", path);
	}
	setpgid(pid, pid); /* also here: the kill must not race the child */
	music_pid = pid;
}

/* Writes a block to a new temp file. Returns 0 when it could not. */
static int Write_Temp(const void *block, unsigned int size, char *path, size_t pathSize) {
	int fd;
	size_t done = 0;
	if (!block || !size || pathSize < 24) return 0;
	strcpy(path, "/tmp/mt2d_audio_XXXXXX");
	fd = mkstemp(path);
	if (fd < 0) return 0;
	while (done < size) {
		ssize_t n = write(fd, (const char *)block + done, size - done);
		if (n <= 0) {
			close(fd);
			remove(path);
			return 0;
		}
		done += (size_t)n;
	}
	close(fd);
	return 1;
}

int MT2D_Linux_Audio_Init(void) {
	return 1;
}

void MT2D_Linux_Audio_Close(void) {
	Stop_Music();
}

void MT2D_Linux_Play_Sound(const char *name) {
	char path[512];
	if (Find_File(name, path, sizeof(path))) Spawn_Effect(PLAY_CHAIN, path);
}

void MT2D_Linux_Play_Music(const char *name) {
	char path[512];
	if (Find_File(name, path, sizeof(path))) Start_Music(path);
}

void MT2D_Linux_Play_Sound_From_Memory(void *block, unsigned int size, bool release) {
	char path[64];
	if (Write_Temp(block, size, path, sizeof(path))) Spawn_Effect(PLAY_CHAIN "; rm -f \"$1\"", path);
	if (release) free(block);
}

void MT2D_Linux_Play_Music_From_Memory(void *block, unsigned int size, bool release) {
	char path[64];
	int ok = Write_Temp(block, size, path, sizeof(path));
	if (release) free(block);
	if (!ok) return;
	Start_Music(path); /* stops the old song, and its temp file */
	strcpy(music_temp, path); /* removed when this song stops */
}
#endif
