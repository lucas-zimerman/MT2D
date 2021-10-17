#ifndef SDL_MT2DFil
#define SDL_MT2DFil

#include "../../MT2D_Terminal_Define.h"
#ifdef SDL_USE
#include <SDL.h>

#define MT2D_FILE SDL_RWops

int MT2D_SDL_FILE_EOF(MT2D_FILE *file);
MT2D_FILE *MT2D_SDL_FILE_OPEN(const char*filePath, const char *mode);
int MT2D_SDL_FILE_CLOSE(MT2D_FILE *file);
int MT2D_SDL_FILE_SEEK(MT2D_FILE *file, int offset, int whence);
size_t MT2D_SDL_FILE_READ(MT2D_FILE *file, void *inBufferPtr, size_t size, size_t maxNum);
unsigned char MT2D_SDL_FILE_READ_BYTE(MT2D_FILE *file);
size_t MT2D_SDL_FILE_WRITE(MT2D_FILE * file, void *outBufferPtr, size_t size, size_t count);
size_t MT2D_SDL_FILE_WRITE_BYTE(MT2D_FILE * file, unsigned char data);
long int MT2D_SDL_FILE_TELL(MT2D_FILE *file);
#endif
#endif