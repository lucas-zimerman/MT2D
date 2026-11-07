/* ObjectGlobals.cpp, see ObjectGlobals.h */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <MT2D/ObjectCore/ObjectGlobals.h>

#define GLOBAL_NAME_MAX (MT2D_VAR_NAME_MAX - 1)

struct ObjSlot { char Name[MT2D_VAR_NAME_MAX]; Object *Obj; };
struct FnSlot { char Name[MT2D_VAR_NAME_MAX]; ObjectCore_CscriptEntry Entry; };

static MT2D_VAR **Global_Vars = 0;
static int Global_VarCount = 0;
static ObjSlot *Global_Objects = 0;
static int Global_ObjectCount = 0;
static FnSlot *Global_Functions = 0;
static int Global_FunctionCount = 0;
static char Global_LastError[96] = "";

void ObjectCore_ClearLastError() { Global_LastError[0] = 0; }

void ObjectCore_SetLastError(const char *What, const char *Name) {
        snprintf(Global_LastError, sizeof(Global_LastError), "%s '%s'", What, Name ? Name : "");
}

const char *ObjectCore_GetLastError() { return Global_LastError; }

static bool Global_NameOk(const char *Name) {
        size_t n = Name ? strlen(Name) : 0;
        return n >= 1 && n <= GLOBAL_NAME_MAX;
}

static void Global_CopyName(char *dst, const char *src) {
        strncpy(dst, src, MT2D_VAR_NAME_MAX - 1);
        dst[MT2D_VAR_NAME_MAX - 1] = 0;
}

/* ---- vars ---------------------------------------------------------------- */
MT2D_VAR *ObjectCore_FindGlobalVar(const char *Name) {
        for (int i = 0; i < Global_VarCount; i++) {
                if (strcmp(Global_Vars[i]->Name, Name) == 0) return Global_Vars[i];
        }
        return 0;
}

bool ObjectCore_AddGlobalVar(MT2D_VAR *Var) {
        if (!Var || !Global_NameOk(Var->Name) || ObjectCore_FindGlobalVar(Var->Name)) return false;
        MT2D_VAR **grown = (MT2D_VAR**)realloc(Global_Vars, (Global_VarCount + 1) * sizeof(MT2D_VAR*));
        if (!grown) return false;
        Global_Vars = grown;
        Global_Vars[Global_VarCount++] = Var;
        return true;
}

bool ObjectCore_RemoveGlobalVar(const char *Name) {
        if (!Name) return false;
        for (int i = 0; i < Global_VarCount; i++) {
                if (strcmp(Global_Vars[i]->Name, Name) == 0) {
                        for (int j = i; j < Global_VarCount - 1; j++) Global_Vars[j] = Global_Vars[j + 1];
                        Global_VarCount--;
                        if (Global_VarCount == 0) { free(Global_Vars); Global_Vars = 0; }
                        return true;
                }
        }
        return false;
}

/* ---- objects ------------------------------------------------------------- */
Object *ObjectCore_FindGlobalObject(const char *Name) {
        for (int i = 0; i < Global_ObjectCount; i++) {
                if (strcmp(Global_Objects[i].Name, Name) == 0) return Global_Objects[i].Obj;
        }
        return 0;
}

bool ObjectCore_AddGlobalObject(const char *Name, Object *Obj) {
        if (!Obj || !Global_NameOk(Name) || ObjectCore_FindGlobalObject(Name)) return false;
        ObjSlot *grown = (ObjSlot*)realloc(Global_Objects, (Global_ObjectCount + 1) * sizeof(ObjSlot));
        if (!grown) return false;
        Global_Objects = grown;
        Global_CopyName(Global_Objects[Global_ObjectCount].Name, Name);
        Global_Objects[Global_ObjectCount].Obj = Obj;
        Global_ObjectCount++;
        return true;
}

bool ObjectCore_RemoveGlobalObject(const char *Name) {
        if (!Name) return false;
        for (int i = 0; i < Global_ObjectCount; i++) {
                if (strcmp(Global_Objects[i].Name, Name) == 0) {
                        for (int j = i; j < Global_ObjectCount - 1; j++) Global_Objects[j] = Global_Objects[j + 1];
                        Global_ObjectCount--;
                        if (Global_ObjectCount == 0) { free(Global_Objects); Global_Objects = 0; }
                        return true;
                }
        }
        return false;
}

/* ---- functions ----------------------------------------------------------- */
bool ObjectCore_FindGlobalCScript(const char *Name, ObjectCore_CscriptEntry *Out) {
        for (int i = 0; i < Global_FunctionCount; i++) {
                if (strcmp(Global_Functions[i].Name, Name) == 0) {
                        if (Out) *Out = Global_Functions[i].Entry;
                        return true;
                }
        }
        return false;
}

static bool Global_AddFunction(const char *Name, const ObjectCore_CscriptEntry &Entry) {
        if (!Global_NameOk(Name) || ObjectCore_FindGlobalCScript(Name, 0)) return false;
        FnSlot *grown = (FnSlot*)realloc(Global_Functions, (Global_FunctionCount + 1) * sizeof(FnSlot));
        if (!grown) return false;
        Global_Functions = grown;
        Global_CopyName(Global_Functions[Global_FunctionCount].Name, Name);
        Global_Functions[Global_FunctionCount].Entry = Entry;
        Global_FunctionCount++;
        return true;
}

bool ObjectCore_AddGlobalCScript(const char *Name, void (*Fn)(Object *)) {
        ObjectCore_CscriptEntry e = { CSCRIPT_KIND_OBJ, 0, Fn, 0, 0 };
        return Fn != 0 && Global_AddFunction(Name, e);
}

bool ObjectCore_AddGlobalCScript(const char *Name, void (*Fn)(Object *, MT2D_VAR **), int ArgCount) {
        ObjectCore_CscriptEntry e = { CSCRIPT_KIND_VARS, ArgCount, 0, Fn, 0 };
        return Fn != 0 && Global_AddFunction(Name, e);
}

bool ObjectCore_AddGlobalCScript(const char *Name, void (*Fn)(Object *, Object *, MT2D_VAR **), int ArgCount) {
        ObjectCore_CscriptEntry e = { CSCRIPT_KIND_OBJ_OBJ_VARS, ArgCount, 0, 0, Fn };
        return Fn != 0 && Global_AddFunction(Name, e);
}

bool ObjectCore_RemoveGlobalCScript(const char *Name) {
        if (!Name) return false;
        for (int i = 0; i < Global_FunctionCount; i++) {
                if (strcmp(Global_Functions[i].Name, Name) == 0) {
                        for (int j = i; j < Global_FunctionCount - 1; j++) Global_Functions[j] = Global_Functions[j + 1];
                        Global_FunctionCount--;
                        if (Global_FunctionCount == 0) { free(Global_Functions); Global_Functions = 0; }
                        return true;
                }
        }
        return false;
}
