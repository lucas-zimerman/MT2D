#if defined(__DOS__) || defined(__MSDOS__)
#include <stdlib.h>
#else
#include <cstdlib>
#endif
#include <stdarg.h>
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/Object.h>

Cscript *Cscript_Empty()
{
        Cscript *F = (Cscript*)malloc(sizeof(Cscript));
        F->_Obj = 0;
        F->_Obj_Vars = 0;
        F->_Obj_Vars = 0;
        F->_Obj_Obj_Vars = 0;
        F->TempObj = 0;
        F->TempVars = 0;
        return F;
}

Cscript *Cscript_New1(void(*Function)()) {
        Cscript *F = Cscript_Empty();
        F->_void = Function;
        return F;
}

Cscript *Cscript_New2(void (*Function)(Object *)) {
        Cscript *F = Cscript_Empty();
        F->_Obj = Function;
        return F;
}

Cscript *Cscript_New3(MT2D_VAR **ObjVarPointer,void(*Function)(Object *, MT2D_VAR **)) {
        Cscript *F = Cscript_Empty();
        F->_Obj_Vars = Function;
        F->TempVars = ObjVarPointer;
        return F;
}

Cscript *Cscript_New4(Object *ModelObj, MT2D_VAR **ObjVarPointers, void(*Function)(Object *, Object *, MT2D_VAR **)) {
        Cscript *F = Cscript_Empty();
        F->_Obj_Obj_Vars = Function;
        F->TempObj = ModelObj;
        F->TempVars = ObjVarPointers;
        return F;
}



Cscript *Cscript_NewV(void(*Function)(), int, ...) {
        return Cscript_New1(Function);
}

Cscript *Cscript_NewV(void(*Function)(Object *), int, ...) {
        return Cscript_New2(Function);
}

static MT2D_VAR **Cscript_ReadVars(int count, va_list vl) {
        MT2D_VAR **Vars = 0;
        if (count > 0) {
                Vars = (MT2D_VAR**)malloc(count * sizeof(MT2D_VAR*));
                for (int i = 0; i < count; i++) {
                        Vars[i] = va_arg(vl, MT2D_VAR*);
                }
        }
        return Vars;
}

Cscript *Cscript_NewV(void(*Function)(Object *, MT2D_VAR **), int count, ...) {
        va_list vl;
        va_start(vl, count);
        MT2D_VAR **Vars = Cscript_ReadVars(count, vl);
        va_end(vl);
        return Cscript_New3(Vars, Function);
}

/* the first extra argument is the model Object*, the rest are the Vars */
Cscript *Cscript_NewV(void(*Function)(Object *, Object *, MT2D_VAR **), int count, ...) {
        va_list vl;
        va_start(vl, count);
        Object *Model = count > 0 ? va_arg(vl, Object*) : 0;
        MT2D_VAR **Vars = Cscript_ReadVars(count > 0 ? count - 1 : 0, vl);
        va_end(vl);
        return Cscript_New4(Model, Vars, Function);
}

void Cscript_Execute(Object *Caller, Cscript *Function) {
        if (Function) {
                if (Caller) {
                        if (Function->_Obj) {
                                Function->_Obj(Caller);
                        }
                        else if (Function->_Obj_Vars) {
                                Function->_Obj_Vars(Caller,Function->TempVars);
                        }
                        else if (Function->_Obj_Obj_Vars) {
                                Function->_Obj_Obj_Vars(Caller,Function->TempObj, Function->TempVars);
                        }

                }
        }
}

void Cscript_RemoveFromMemory(Cscript *Me) {
        if (Me) {
                if (Me->TempObj) {
                        free(Me->TempObj); // we only store a pointer and not the actuall object so we just remover the byte that points for that object.
                }
                if (Me->TempVars) { //again, we only remove the array of pointers and not the actual vars
                        free(Me->TempVars);
                }
                free(Me);
        }
}
