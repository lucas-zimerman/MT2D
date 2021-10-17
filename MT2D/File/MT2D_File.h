/*===================================================================
        MT2D_File.h created by: Lucas Zimerman Fraulob
        
        Here's a simple multiplatform file handler using basic functions
        for interacting with local files and container files.
        
        MT2D File Open
                Description: Opens a file
                Args:
                -filePath: the path where MT2D is gonna open your file.
                -mode: an ASCII string representing the mode to be used for 
                opening the file; (the same as the ANSI C modes form the FILE
                struct)
        
        MT2D File Close
                Description: Closes a file
        
        MT2D File Read Byte
                Description: Read a byte from a file, return 0 in case of EOF or error
        
        MT2D File Read
                Description: Read x bytes from a given file, return 0 in case of EOF or error.
                Args:
                -file: the file to be loaded.
                -inBufferPtr: the buffer where the data will be stored after readed.
                -size: size of the buffer in bytes.
                -maxNum: the maximum number of objects to be read.

        MT2D File Seek
                Description: seeks to offset relative to whence whence may 
                be any of the following values:
                        SEEK_SET: seek from the beginning of data
                        SEEK_CUR: seek relative to current read point
                        SEEK_END: seek relative to the end of data
                Return: If successful, the function returns zero.
                Otherwise, it returns -1.
                Args:
                -file: the file to be seeked.
                -offset: an offset in bytes, relative to whence location; 
                can be negative.
                -whence: any of SEEK_SET, SEEK_CUR or SEEK_END
                
        MT2D File Write
                Description: Write data under the given file, return the
                number of the objects that were written.
                Args:
                -file: the file to be written, in the last set offset.
                -outBufferPtr: a pointer to a buffer containing data to write.
                -size: the size of an object to write, in bytes
                -count: the number of objects to write

        MT2D File Write Byte
                Description: Write a byte under the given file.
                Return: the written byte on sucess
                Args:
                -file: the file to be written, in the last set offset.

        MT2D File Tell
                Description: get the current read/write offset in a file
                Return: the current offset, -1 on error.
                Args:
                -file: the file to be checked

        NOTE: Do not try to use the internal data inside the MT2D_FILE
        since it may change acording to the OS or Libraries.
        Intead, use the given functions to do what you need to.
        
===================================================================*/
#include <MT2D/MT2D_Terminal_Define.h>
#if !defined(MT2D_FILEH)
#define MT2D_FILEH 0

#if defined(__ANDROID__)
#include"../SDL/IO/SDL_MT2D_FILE.h"     
#define MT2D_FILE_OPEN MT2D_SDL_FILE_OPEN
#define MT2D_FILE_CLOSE MT2D_SDL_FILE_CLOSE
#define MT2D_FILE_READ MT2D_SDL_FILE_READ
#define MT2D_FILE_READ_BYTE MT2D_SDL_FILE_READ_BYTE
#define MT2D_FILE_WRITE MT2D_SDL_FILE_WRITE
#define MT2D_FILE_WRITE_BYTE MT2D_SDL_FILE_WRITE_BYTE
#define MT2D_FILE_SEEK MT2D_SDL_FILE_SEEK
#define MT2D_FILE_EOF MT2D_SDL_FILE_EOF
#define MT2D_FILE_TELL MT2D_SDL_FILE_TELL

#else
//other platforms are gonna use the FILE struct when they can
#include <stdio.h>
#define MT2D_FILE FILE

 inline MT2D_FILE *MT2D_FILE_OPEN(const char *fileName, const char *mode) {
        return fopen(fileName, mode);
}

 inline MT2D_FILE *MT2D_FILE_OPEN_TEMP() {
        return tmpfile();
}

 inline int MT2D_FILE_CLOSE(MT2D_FILE *file) {
        return fclose(file);
}


 inline int MT2D_FILE_SEEK(MT2D_FILE *file, long offset, int whence) {
        return fseek(file, offset, whence);
}

 inline size_t MT2D_FILE_READ(MT2D_FILE *file, void *inBufferPtr, size_t size, size_t maxNum) {
        return fread(inBufferPtr, size, maxNum, file);
}

 inline unsigned char MT2D_FILE_READ_BYTE(MT2D_FILE *file) {
        return fgetc(file);
}

 inline size_t MT2D_FILE_WRITE(MT2D_FILE * file, void *outBufferPtr, size_t size, size_t count) {
        return fwrite(outBufferPtr, size, count, file);
}

 inline int MT2D_FILE_EOF(MT2D_FILE *file) {
        return feof(file);
}

 inline long int MT2D_FILE_TELL(MT2D_FILE *file) {
        return ftell(file);
}

 inline int MT2D_FILE_WRITE_BYTE(MT2D_FILE * file, unsigned char data) {
        return fputc(data, file);
}

inline int MT2D_FILE_DELETE(char *name){
        return remove(name);
}
#endif
#endif
