#include "../../MT2D_Terminal_Define.h"
#ifdef SDL_USE
#include <SDL.h>
#include "SDL_MT2D_FILE.h"

int MT2D_SDL_FILE_EOF(MT2D_FILE *file){
	return (int)SDL_RWsize(file) - SDL_RWtell(file);
}



MT2D_FILE *MT2D_SDL_FILE_OPEN(const char*filePath,const char *mode){
	return SDL_RWFromFile(filePath, mode);
}

int MT2D_SDL_FILE_CLOSE(MT2D_FILE *file){
	return SDL_RWclose(file);
}

int MT2D_SDL_FILE_SEEK(MT2D_FILE *file, int offset, int whence) {
	/*whence can be the SDL or STD pattern*/
	/*adapted return so we get a FILE like return*/
	return (SDL_RWseek(file, offset, whence) > 0 ? 0 : -1);
}

size_t MT2D_SDL_FILE_READ(MT2D_FILE *file, void *inBufferPtr, size_t size, size_t maxNum) {
	return SDL_RWread(file, inBufferPtr, size, maxNum);
}

unsigned char MT2D_SDL_FILE_READ_BYTE(MT2D_FILE *file) {
	unsigned char buf = 0;
	if (file) {
		SDL_RWread(file, &buf, sizeof(buf), 1);
	}
	return buf;
}

size_t MT2D_SDL_FILE_WRITE(MT2D_FILE * file, void *outBufferPtr, size_t size, size_t count) {
	return SDL_RWwrite(file, outBufferPtr, size, count);
}

size_t MT2D_SDL_FILE_WRITE_BYTE(MT2D_FILE * file, unsigned char data) {
	unsigned char buf = data;
	return (SDL_RWwrite(file, &buf, sizeof(buf), 1) == -1? -1 : data);
}

long int MT2D_SDL_FILE_TELL(MT2D_FILE *file) {
	return (long int)SDL_RWtell(file);
}
#endif