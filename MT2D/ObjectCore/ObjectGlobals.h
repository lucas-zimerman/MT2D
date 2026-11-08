/* ObjectGlobals.h
   Names the game registers so that compiled objects (ObjectLoader) can use them. In a script they
   are written with the global prefix:

        global DogHealth--
        Pos_X -= global GameSpeed
        if (global JumpRequest == 1) { Goto doJmp }
        global SmokeSpawnRight()
        GotoStateIfHitObject(global Doggo, hit)

   Register them before loading the objects that use them. A name that is not registered makes
   ObjectCore_Load return NULL, ObjectCore_GetLastError() tells which one. The names of a kind
   (vars, objects, functions) are unique and can have 18 characters (MT2D_VAR_NAME_MAX - 1).

   Sprites are different: nothing is written in the script. Register a sprite under the path a
   script uses in Show ("SPRITEX", image "a.png" 2 2, container "a.png" 2 2) and the loader uses it
   instead of loading that file. The registry holds a reference of its own (see below).

   A loaded object keeps the pointer it found, nothing is looked up while the object runs. The
   Remove functions always work and leave the control to you: unload (or never run again) the
   objects that use a global before you remove it or free what it points to.
*/
#pragma once
#ifndef MT2D_OBJECT_GLOBALS_H
#define MT2D_OBJECT_GLOBALS_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/ObjectCore/Sprites.h>

MT2D_C_BEGIN

/* The var is not copied, the name is Var->Name. Int, char and float vars are read and written by
   the Cscript functions as they are. Returns false for a duplicate or a bad name. */
bool ObjectCore_AddGlobalVar(MT2D_VAR *Var);
bool ObjectCore_RemoveGlobalVar(const char *Name);

/* An object some Cscript functions need as an argument (the model of CreateObject, the target of
   GotoStateIfHitObject). The loaded frames keep the pointer they see when the file is loaded. */
bool ObjectCore_AddGlobalObject(const char *Name, Object *Obj);
bool ObjectCore_RemoveGlobalObject(const char *Name);

/* A sprite the loader uses instead of loading the file called Path (the text a script has in Show,
   up to 255 characters, any kind of Show). The registry takes a reference (refCount++) so the
   objects that unload never free it, do not call Sprite_Delete on it yourself. A sprite that
   was registered keeps the scale it was created with, the w h of the script are ignored.
   Remove releases that reference: the sprite is freed when the last state that uses it is gone.
   Returns false for a NULL sprite, a bad path or a duplicate. */
bool ObjectCore_AddGlobalSprite(const char *Path, Sprite *Spr);
bool ObjectCore_RemoveGlobalSprite(const char *Path);
void ObjectCore_ClearGlobalSprites(void);   /* Remove all of them (shutdown) */
int ObjectCore_CountGlobalSprites(void);    /* How many sprites are registered */

/* Your own Cscript functions. ArgCount is how many arguments a script has to write (for the
   Object* + vars signature the object counts as the first one), -1 accepts any amount. */
bool ObjectCore_AddGlobalCScript(const char *Name, void (*Fn)(Object *));                                    /* void f(Object*) */
bool ObjectCore_AddGlobalCScriptVars(const char *Name, void (*Fn)(Object *, MT2D_VAR **), int ArgCount);     /* void f(Object*, MT2D_VAR**) */
bool ObjectCore_AddGlobalCScriptObjVars(const char *Name, void (*Fn)(Object *, Object *, MT2D_VAR **), int ArgCount); /* void f(Object*, Object*, MT2D_VAR**) */
bool ObjectCore_RemoveGlobalCScript(const char *Name);

/* Why the last ObjectCore_Load/ObjectCore_Clone failed ("" when it worked). */
const char *ObjectCore_GetLastError(void);

/* ---- used by the loader ---------------------------------------------------- */
typedef enum MT2D_CSCRIPT_KIND {
        CSCRIPT_KIND_OBJ = 0,          /* void f(Object*) */
        CSCRIPT_KIND_VARS = 1,         /* void f(Object*, MT2D_VAR**) */
        CSCRIPT_KIND_OBJ_OBJ_VARS = 2  /* void f(Object*, Object*, MT2D_VAR**) */
} MT2D_CSCRIPT_KIND;

typedef struct ObjectCore_CscriptEntry {
        int Kind;
        int ArgCount; /* -1 = any */
        void (*Obj)(Object *);
        void (*Vars)(Object *, MT2D_VAR **);
        void (*ObjObj)(Object *, Object *, MT2D_VAR **);
} ObjectCore_CscriptEntry;

MT2D_VAR *ObjectCore_FindGlobalVar(const char *Name);
Object *ObjectCore_FindGlobalObject(const char *Name);
Sprite *ObjectCore_FindGlobalSprite(const char *Path);
bool ObjectCore_FindGlobalCScript(const char *Name, ObjectCore_CscriptEntry *Out);
void ObjectCore_SetLastError(const char *What, const char *Name);
void ObjectCore_ClearLastError(void);

MT2D_C_END

#endif
