#ifndef MT2D_CDECL_H
#define MT2D_CDECL_H
/*
        Lets a header be included from C and from C++:
          MT2D_C_BEGIN / MT2D_C_END wrap the declarations (C linkage), and C gets "bool".
*/
#ifdef __cplusplus
#define MT2D_C_BEGIN extern "C" {
#define MT2D_C_END   }
#else
#include <stdbool.h>
/* "bool" is _Bool (curses.h pulls <stdbool.h> too, so it cannot be avoided). Strict C89 has no _Bool and clang
   (Apple clang too) calls it a C99 extension, an error with -pedantic-errors. Every supported compiler accepts it,
   so that warning is switched off. The guards keep DJGPP's -Werror=unknown-pragmas happy. */
#if defined(__clang__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#pragma clang diagnostic ignored "-Wc99-extensions"
#endif
#define MT2D_C_BEGIN
#define MT2D_C_END
#endif

/* "static inline" for headers: C89 has no inline keyword, GNU C (DJGPP, gcc) has __inline__ */
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define MT2D_INLINE static inline
#elif defined(__GNUC__)
#define MT2D_INLINE static __inline__
#else
#define MT2D_INLINE static
#endif

/* forward declared once here so the headers can include each other (C forbids repeated typedefs) */
struct Sprite;      typedef struct Sprite Sprite;
struct Cscript;     typedef struct Cscript Cscript;
struct ObjectState; typedef struct ObjectState ObjectState;
struct Object;      typedef struct Object Object;
struct ObjectScene; typedef struct ObjectScene ObjectScene;
#endif
