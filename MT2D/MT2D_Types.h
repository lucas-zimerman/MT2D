#ifndef MT2D_TYPES_H
#define MT2D_TYPES_H
/*
        Fixed width types and allocation macros.
        Borland C++ (16 bit, no <stdint.h>) and DJGPP (32 bit) disagree on the size
        of "int", so anything that is stored/serialized or size sensitive should use these.
*/
#include <stdlib.h>

typedef unsigned char  MT2D_U8;
typedef unsigned short MT2D_U16;
#if defined(__BORLANDC__) || defined(__TURBOC__) || (defined(__MSDOS__) && !defined(__DJGPP__))
typedef unsigned long  MT2D_U32; /* int is 16 bits here */
#else
typedef unsigned int   MT2D_U32;
#endif

/* compile time size checks (a negative array size is an error on every compiler) */
typedef char MT2D_Check_U8_Size [sizeof(MT2D_U8)  == 1 ? 1 : -1];
typedef char MT2D_Check_U16_Size[sizeof(MT2D_U16) == 2 ? 1 : -1];
typedef char MT2D_Check_U32_Size[sizeof(MT2D_U32) == 4 ? 1 : -1];

/*
        Single place to change how engine memory is allocated.
        (Borland large/compact models may want farmalloc/farfree here.)
*/
#define MT2D_Alloc(size) malloc(size)
#define MT2D_Free(ptr)   free(ptr)

#endif
