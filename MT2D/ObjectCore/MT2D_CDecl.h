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
#define MT2D_C_BEGIN
#define MT2D_C_END
#endif

/* forward declared once here so the headers can include each other (C forbids repeated typedefs) */
struct Sprite;      typedef struct Sprite Sprite;
struct Cscript;     typedef struct Cscript Cscript;
struct ObjectState; typedef struct ObjectState ObjectState;
struct Object;      typedef struct Object Object;
struct ObjectScene; typedef struct ObjectScene ObjectScene;
#endif
