/* Cscript, created by Lucas Zimerman Fraulob.
        With Cscript, you can make the ObjectCore much more powerfull, it allows you to
        add any kind of function into your object, those includes accessing other objects data.

        The VARS stored in Cscript must be "const" (by that, their data value shall not be changed, so
        you cannot reuse the same VAR stored in your object directly but you can create a VAR pointer
        by storing a const pointer to your object's VAR or even other objects.

        Cscript can only interact with const Vars given in his creation or to Vars referenced in Objects,
        you can extend that limit but you'll need to code your own solution to your problem.

        Also, Cscript never returns anything, but it uses the Vars arg to store their data, so you can
        give a pointer to a var where you want the function to store the returned value.

        for saving memory, you can share the same function with many objects because freeing the object
        from the memory are not going to clear the Cscript.
        -----------------------------------------------------------------------------------------------

        This File will only include the minimal struct for using Cscript, you can build your own functions
        and link it on Cscript or you could also use the build in functions in the file STDFunctions

*/

/* HOW TO USE A FUNCTION WITH Cscript

You can link any kind of c/c++ function with Cscript, you just need to create a function where the input args are one of the following ones:
void ...();
void ...(Object *Name);
void ...(Object *Name, MT2D_VAR **Array of const Vars);

You can also use the build in functions inside STDFunctions.h

*/

#pragma once
#if !defined(MT2D_OBJFUN_H)
#define MT2D_OBJFUN_H

#include <MT2D/ObjectCore/Vars.h>
#include <MT2D/ObjectCore/Object.h>
//extern struct Object;// this guy is here because we have a circular include.

/**
        This function only store pointers, no malloc involved.
        So in order to clean the memory only free the MT2D_OBECT_FUNCTION
        and the other pointers you free externally by their original pointers.
        f>>>>>>>>>>>>>>>>>>>>>>>>>
        The TempVars are not going to be pointed directly to the vars itself but rather
        to a string that has the same name from a var inside the object, so the function
        can be reused by other objects without affecting the logic.
**/


struct Cscript
{
        void(*_void)(); //a function without args
        void(*_Obj)(struct Object*);//a function with only a single object as arg, normally the caller object
        void(*_Obj_Vars)(struct Object*, MT2D_VAR **);//a function with only a single object and one or more Vars as arg, normally the caller as an object
        void(*_Obj_Obj_Vars)(struct Object*, struct Object*, MT2D_VAR**);//a function with only a single object, one or more objects in the second arg and lastly one or more Vars as arg, normally the caller as the first object
        MT2D_VAR **TempVars;
        struct Object *TempObj;
};

Cscript *Cscript_Empty();
Cscript *Cscript_New1(void(*Function)());
Cscript *Cscript_New2(void(*Function)(struct Object *));
Cscript *Cscript_New3(MT2D_VAR **ObjVarPointer, void(*Function)(struct Object*, MT2D_VAR **));
Cscript *Cscript_New4(struct Object *ModelObj, MT2D_VAR **ObjVarPointers, void(*Function)(struct Object *, struct Object*, MT2D_VAR **));

void Cscript_RemoveFromMemory(Cscript *Me);
void Cscript_Execute(Object *caller, Cscript *f);
#endif
