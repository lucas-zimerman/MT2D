#include <stdlib.h>
#include <stdarg.h>
#include <MT2D/ObjectCore/Cscript.h>
#include <MT2D/ObjectCore/Object.h>

Cscript *Cscript_Empty(void)
{
        Cscript *F = (Cscript*)malloc(sizeof(Cscript));
        F->_void = 0;
        F->_Obj = 0;
        F->_Obj_Vars = 0;
        F->_Obj_Obj_Vars = 0;
        F->TempObj = 0;
        F->TempVars = 0;
        F->Packed = 0;
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



/* Reads count Var pointers from the argument list. */
static MT2D_VAR **Cscript_ReadVars(int count, va_list vl) {
        MT2D_VAR **Vars = 0;
        int i;
        if (count > 0) {
                Vars = (MT2D_VAR**)malloc(count * sizeof(MT2D_VAR*));
                for (i = 0; i < count; i++) {
                        Vars[i] = va_arg(vl, MT2D_VAR*);
                }
        }
        return Vars;
}

Cscript *Cscript_New3VA(void(*Function)(Object *, MT2D_VAR **), int count, va_list vl) {
        MT2D_VAR **Vars = Cscript_ReadVars(count, vl);
        return Cscript_New3(Vars, Function);
}

/* the first extra argument is the model Object*, the rest are the Vars */
Cscript *Cscript_New4VA(void(*Function)(Object *, Object *, MT2D_VAR **), int count, va_list vl) {
        Object *Model = count > 0 ? va_arg(vl, Object*) : 0;
        MT2D_VAR **Vars = Cscript_ReadVars(count > 0 ? count - 1 : 0, vl);
        return Cscript_New4(Model, Vars, Function);
}

Cscript *Cscript_New3V(void(*Function)(Object *, MT2D_VAR **), int count, ...) {
        Cscript *F;
        va_list vl;
        va_start(vl, count);
        F = Cscript_New3VA(Function, count, vl);
        va_end(vl);
        return F;
}

Cscript *Cscript_New4V(void(*Function)(Object *, Object *, MT2D_VAR **), int count, ...) {
        Cscript *F;
        va_list vl;
        va_start(vl, count);
        F = Cscript_New4VA(Function, count, vl);
        va_end(vl);
        return F;
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
                if (Me->Packed) {
                        free(Me); /* one block, nothing else to release */
                        return;
                }
                /* TempObj (the model or target object) is not owned by the Cscript, the caller deletes it */
                if (Me->TempVars) { /* we only remove the array of pointers and not the actual vars */
                        free(Me->TempVars);
                }
                free(Me);
        }
}
