/* ObjectLoader.h
   Loads an Object from a compiled object file (.mto), made by the text compiler
   (MT2D/ObjectCore/Parser) or by your own tool.

        Object *movingDot = ObjectCore_Load("movingdot.mto");

   The loader is small on purpose, it has no parser and no function name table, so it can be
   used on slow/low memory targets. The file is read in one pass, every array is allocated with
   its final size (the counts are stored in the file) and each frame is a single allocation.

   The states are shared data: to create more objects with the same behaviour use
        Object_Create(solid, renderOnly, sx, sy, x, y, movingDot->State, movingDot->States_Count)
   and release the model with ObjectCore_Unload(model) only when no other object uses its states.
   Sprites are loaded once per file name and shared by the frames that use it.

   File format (all numbers little endian):

        "MTO1"
        u8  flags                bit 0 = solid, bit 1 = renderOnly
        u16 spriteCount          then spriteCount x { u8 kind, [u16 scaleX, u16 scaleY if kind > 0], u8 len, path }
          kind 0 = ASCII file (Load_Sprite), 1 = image file (Load_Sprite_Image),
          kind 2 = image from the container (Load_Sprite_Image_From_Container)
        u16 stateCount (>= 1)    then stateCount x state
          state = { u8 nameLen (1..18), name, u16 frameCount (>= 1), frameCount x frame }
          frame = { u16 sprite (0xFFFF = none), u16 wait, u16 function, [function parts] }
            function 0xFFFF = no function, 0..N = the position in CscriptFunctions.h,
            0xFFFE = a global function: { u8 len, name } follows the number.
            With a function: u8 argc, u16 stringBytes, argc x arg
          arg   = { u8 0, i32 value }                  an integer constant
                | { u8 1, u8 len, name }               an object var or state name (len 1..18)
                | { u8 2, u8 len, text }               a text, counts len + 1 in stringBytes
                | { u8 3, u8 len, name }               a global var, or a global object when it is
                                                       the first argument of a function that takes one
        u8 newVars, u8 records   then records x { u8 slot, [u8 len, name if slot is 0xFF], i32 value }
          slot 0..5 = Pos_X, Pos_Y, Size_X, Size_Y, Acel_X, Acel_Y (set the value)
          slot 0xFF = a new int var called name (newVars of the records are new)

   Global vars, objects and functions are looked up by name (ObjectGlobals.h) when the file is
   loaded, a missing one makes the load fail, see ObjectCore_GetLastError().
*/
#pragma once
#ifndef MT2D_OBJECT_LOADER_H
#define MT2D_OBJECT_LOADER_H

#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/ObjectGlobals.h>

/* Returns NULL when the file is missing, truncated or invalid (nothing is leaked). */
Object *ObjectCore_Load(const char *file);
Object *ObjectCore_LoadFromMemory(const unsigned char *data, unsigned long size);

/* Releases an object made by ObjectCore_Load: the object, its states (frames, functions, names),
   the array that holds the states and the sprites nobody else uses. */
void ObjectCore_Unload(Object *Model);

/* A new object that shares the states (and so the behaviour) of Model: same flags, size,
   acceleration and user vars (with their current values), placed at x, y. Delete it with
   Object_Delete(o, false), the states belong to the model. */
Object *ObjectCore_Clone(Object *Model, int x, int y);

#endif
