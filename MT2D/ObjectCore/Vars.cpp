// Vars.cpp, created by: Lucas Zimerman Fraulob
// This file contains the basic functions to interact with a Var struct.
//
// How to set data into the var?
// A:
// int X = 5;
// -MT2D_Object_Set_Var(Var, X);
// -MT2D_Object_Set_Var(Var, 5);
// -MT2D_Object_Set_Var(Var, 1.01);
// -MT2D_Object_Set_Var(Var, (void*)&X, VAR_INT);   (the value is copied)
//
// How to get data from a var?
//
// The value is stored inline in the var (union Data), pick the member by Type:
// int X = var->Data.i;   // VAR_INT      (Data.c for VAR_CHAR, Data.f for VAR_FLOAT)
// void *P = var->Data.p; // VAR_POINTER  (not owned by the var)
//
// Check the supported var types under the MT2D_VAR_TYPE enum
//
// to know if there were some kind of error while interacting with the var functions
// check the return from the function MT2D_Object_Get_Last_Var_Error, if null, there were no errors
// else, check the MT2D_VAR_ERROR enum for more information.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <MT2D/ObjectCore/Vars.h>


MT2D_VAR_ERROR MT2D_Var_Last_Error = Var_No_Error;

static void Private_Set_Error(MT2D_VAR_ERROR e, const char *msg)
{
        MT2D_Var_Last_Error = e;
#ifdef _DEBUG
        printf("MT2D: %s\n", msg);
#else
        (void)msg;
#endif
}

/* Numeric view of any numeric var. */
static inline float Private_To_Float(const MT2D_VAR *v)
{
        return v->Type == VAR_FLOAT ? v->Data.f : (v->Type == VAR_CHAR ? (float)v->Data.c : (float)v->Data.i);
}

static char *Private_Dup(const char *Text)
{
        size_t len = strlen(Text) + 1;
        char *out = (char*)malloc(len);
        if (out) memcpy(out, Text, len);
        return out;
}

static MT2D_VAR *Private_Alloc(const char *Name, MT2D_VAR_TYPE Type)
{
        MT2D_VAR *V;
        int i;
        if (Name == NULL) {
                Private_Set_Error(Var_Null_Name, "NULL NAME RECEIVED IN VAR");
                return NULL;
        }
        V = (MT2D_VAR*)malloc(sizeof(MT2D_VAR));
        if (V == NULL) {
                Private_Set_Error(Var_Out_Of_Memory_To_Create_New_Var, "Out of Memory to create a new var");
                return NULL;
        }
        /* One bounded pass both copies the name and detects names that don't fit. */
        for (i = 0; i < MT2D_VAR_NAME_MAX; i++) {
                V->Name[i] = Name[i];
                if (Name[i] == 0) break;
        }
        if (i == MT2D_VAR_NAME_MAX) {
                free(V);
                Private_Set_Error(Var_Name_Too_Long, "VAR NAME IS TOO LONG");
                return NULL;
        }
        V->Type = (signed char)Type;
        memset(&V->Data, 0, sizeof(V->Data));
        MT2D_Var_Last_Error = Var_No_Error;
        return V;
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, bool Data)
{
        int v = Data ? 1 : 0;
        MT2D_Object_Set_Var(Var, (void*)&v, VAR_INT);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, int Data)
{
        MT2D_Object_Set_Var(Var, (void*)&Data, VAR_INT);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, float Data)
{
        MT2D_Object_Set_Var(Var, (void*)&Data, VAR_FLOAT);
}

/*
        Generic function used by the others
*/
void MT2D_Object_Set_Var(MT2D_VAR *Var, void * Data, MT2D_VAR_TYPE Type)
{
        MT2D_Var_Last_Error = Var_No_Error;
        if (Var == NULL) {
                Private_Set_Error(Var_Not_Found, "NULL VAR");
        }
        else if (Var->Type != Type) {
                Private_Set_Error(Var_Wrong_Type, "WRONG TYPE IN VAR");
        }
        else if (Type == VAR_POINTER) {
                Var->Data.p = Data;
        }
        else if (Type == VAR_STRING) {
                MT2D_Object_Set_Var_String(Var, (const char*)Data);
        }
        else if (Data == NULL) {
                Private_Set_Error(Var_Null_Data, "NULL DATA RECEIVED IN VAR");
        }
        else if (Type == VAR_CHAR) {
                Var->Data.c = *(char*)Data;
        }
        else if (Type == VAR_INT) {
                Var->Data.i = *(int*)Data;
        }
        else if (Type == VAR_FLOAT) {
                Var->Data.f = *(float*)Data;
        }
        else {
                Private_Set_Error(Var_Type_Not_Supported, "TYPE NOT SUPPORTED");
        }
}

void * MT2D_Object_Get_Var_Data(MT2D_VAR * Var)
{
        MT2D_Var_Last_Error = Var_No_Error;
        if (Var)
        {
                return (Var->Type == VAR_POINTER || Var->Type == VAR_STRING) ? Var->Data.p : (void*)&Var->Data;
        }
        Private_Set_Error(Var_Not_Found, "NULL VAR");
        return NULL;
}

MT2D_VAR * MT2D_Object_Get_Var_Pointer(MT2D_VAR * ArrayVARS, char * Name)
{
        int i = 0;
        MT2D_VAR *ptr = NULL;
        MT2D_Var_Last_Error = Var_No_Error;
        if (Name)
        {
                while (ptr == NULL)
                {
                        if (strcmp(ArrayVARS[i].Name, Name) == 0)
                        {
                                ptr = &ArrayVARS[i];
                        }
                        ///yeah, there will be an error here  if the var name isnt on the list.
                        //and it's case sensitive, so "aaa" != "AAA"
                        i++;
                }
        }
        else
        {
                Private_Set_Error(Var_Null_Name, "NULL NAME RECEIVED IN VAR");
        }

        return ptr;
}

MT2D_VAR_ERROR MT2D_Object_Get_Last_Var_Error()
{
        return MT2D_Var_Last_Error;
}

MT2D_VAR * MT2D_Object_Create_Var_Int(const char * Name, int InitialData)
{
        MT2D_VAR *V = Private_Alloc(Name, VAR_INT);
        if (V) V->Data.i = InitialData;
        return V;
}

MT2D_VAR * MT2D_Object_Create_Var_Char(const char * Name, char InitialData)
{
        MT2D_VAR *V = Private_Alloc(Name, VAR_CHAR);
        if (V) V->Data.c = InitialData;
        return V;
}

MT2D_VAR * MT2D_Object_Create_Var_Float(const char * Name, float InitialData)
{
        MT2D_VAR *V = Private_Alloc(Name, VAR_FLOAT);
        if (V) V->Data.f = InitialData;
        return V;
}

MT2D_VAR * MT2D_Object_Create_Var_Pointer(const char * Name)
{
        //the only important data is the Name
        //Check ObjectCore_Get_Object_Internal_Data for looking to what Names are allowed to use.
        return Private_Alloc(Name, VAR_POINTER);
}

MT2D_VAR * MT2D_Object_Create_Var_String(const char * Name, const char * Text)
{
        MT2D_VAR *V;
        if (Text == NULL) {
                Private_Set_Error(Var_Null_Data, "NULL DATA RECEIVED IN VAR");
                return NULL;
        }
        V = Private_Alloc(Name, VAR_STRING);
        if (V) {
                V->Data.p = Private_Dup(Text);
                if (V->Data.p == NULL) {
                        free(V);
                        Private_Set_Error(Var_Out_Of_Memory_To_Create_New_Var, "Out of Memory to create a new var");
                        return NULL;
                }
        }
        return V;
}

void MT2D_Object_Set_Var_String(MT2D_VAR *Var, const char *Text)
{
        char *copy;
        MT2D_Var_Last_Error = Var_No_Error;
        if (Var == NULL) {
                Private_Set_Error(Var_Not_Found, "NULL VAR");
        }
        else if (Var->Type != VAR_STRING) {
                Private_Set_Error(Var_Wrong_Type, "WRONG TYPE IN VAR");
        }
        else if (Text == NULL) {
                Private_Set_Error(Var_Null_Data, "NULL DATA RECEIVED IN VAR");
        }
        else {
                copy = Private_Dup(Text); /* before the release: Text may point inside the old text */
                if (copy == NULL) {
                        Private_Set_Error(Var_Out_Of_Memory_To_Create_New_Var, "Out of Memory to create a new var");
                } else {
                        free(Var->Data.p);
                        Var->Data.p = copy;
                }
        }
}

const char *MT2D_Object_Get_Var_String(MT2D_VAR *Var)
{
        MT2D_Var_Last_Error = Var_No_Error;
        if (Var == NULL) {
                Private_Set_Error(Var_Not_Found, "NULL VAR");
                return NULL;
        }
        if (Var->Type != VAR_STRING) {
                Private_Set_Error(Var_Wrong_Type, "WRONG TYPE IN VAR");
                return NULL;
        }
        return (const char*)Var->Data.p;
}

/*
        Generic Function, InitialData is read (copied) according to Type: a char*, int*, float*,
        or a const char* text for VAR_STRING. It is ignored for VAR_POINTER.
*/
MT2D_VAR * MT2D_Object_Create_Var(const char * Name, MT2D_VAR_TYPE Type, void * InitialData)
{
        MT2D_VAR *V;
        if (Type == VAR_STRING) {
                return MT2D_Object_Create_Var_String(Name, (const char*)InitialData);
        }
        if (Type != VAR_POINTER) {
                if (Type != VAR_CHAR && Type != VAR_INT && Type != VAR_FLOAT) {
                        Private_Set_Error(Var_Type_Not_Supported, "TYPE NOT SUPPORTED");
                        return NULL;
                }
                if (InitialData == NULL) {
                        Private_Set_Error(Var_Null_Data, "NULL DATA RECEIVED IN VAR");
                        return NULL;
                }
        }
        V = Private_Alloc(Name, Type);
        if (V) {
                if (Type == VAR_CHAR)       V->Data.c = *(char*)InitialData;
                else if (Type == VAR_INT)   V->Data.i = *(int*)InitialData;
                else if (Type == VAR_FLOAT) V->Data.f = *(float*)InitialData;
        }
        return V;
}


/**
        does not change the original object type
**/
void MT2D_Object_VAR_SetVar(MT2D_VAR *Var, MT2D_VAR *New) {
        if (New->Type != VAR_CHAR && New->Type != VAR_INT && New->Type != VAR_FLOAT) {
                if (Var->Type != VAR_CHAR && Var->Type != VAR_INT && Var->Type != VAR_FLOAT)
                        MT2D_Var_Last_Error = Var_Type_Not_Supported;
                return;
        }
        switch (Var->Type) {
        case VAR_CHAR:  Var->Data.c = New->Type == VAR_FLOAT ? (char)New->Data.f  : (char)MT2D_Object_VAR_GetInt(New); break;
        case VAR_INT:   Var->Data.i = MT2D_Object_VAR_GetInt(New); break;
        case VAR_FLOAT: Var->Data.f = Private_To_Float(New); break;
        default:        MT2D_Var_Last_Error = Var_Type_Not_Supported; break;
        }
}

static void Private_Arith(MT2D_VAR *Store, MT2D_VAR *Other, int sign) {
        if (Other->Type != VAR_CHAR && Other->Type != VAR_INT && Other->Type != VAR_FLOAT) return;
        switch (Store->Type) {
        case VAR_INT:
                Store->Data.i += sign * MT2D_Object_VAR_GetInt(Other);
                break;
        case VAR_CHAR:
                if (Other->Type == VAR_FLOAT)
                        Store->Data.c = (char)((float)Store->Data.c + sign * Other->Data.f);
                else
                        Store->Data.c = (char)(Store->Data.c + sign * MT2D_Object_VAR_GetInt(Other));
                break;
        case VAR_FLOAT:
                Store->Data.f += sign * Private_To_Float(Other);
                break;
        default:
                break;
        }
}

void  MT2D_Object_ADD(MT2D_VAR *Store, MT2D_VAR *ToAdd) {
        Private_Arith(Store, ToAdd, 1);
}

void  MT2D_Object_SUB(MT2D_VAR *Store, MT2D_VAR *ToAdd) {
        Private_Arith(Store, ToAdd, -1);
}

MT2D_VAR *MT2D_VAR_CLONE(MT2D_VAR *var) {
        MT2D_VAR *NewVAR = 0;
        if (var->Type == VAR_STRING) {
                NewVAR = MT2D_Object_Create_Var_String(var->Name, (const char*)var->Data.p);
        }
        else if (var->Type == VAR_INT || var->Type == VAR_CHAR || var->Type == VAR_FLOAT) {
                NewVAR = Private_Alloc(var->Name, (MT2D_VAR_TYPE)var->Type);
                if (NewVAR) NewVAR->Data = var->Data;
        }
        return NewVAR;
}



MT2D_VAR **MT2D_VAR_Create_Matrix1(MT2D_VAR *first) {
        MT2D_VAR **M = (MT2D_VAR**)malloc(sizeof(MT2D_VAR*));
        M[0] = first;
        return M;
}

MT2D_VAR **MT2D_VAR_Create_Matrix2(MT2D_VAR *first, MT2D_VAR *second) {
        MT2D_VAR **M = (MT2D_VAR**)malloc(2*sizeof(MT2D_VAR*));
        M[0] = first;
        M[1] = second;
        return M;
}

MT2D_VAR **MT2D_VAR_Create_Matrix3(MT2D_VAR *first, MT2D_VAR *second, MT2D_VAR *third) {
        MT2D_VAR **M = (MT2D_VAR**)malloc(3*sizeof(MT2D_VAR*));
        M[0] = first;
        M[1] = second;
        M[2] = third;
        return M;
}


MT2D_VAR **MT2D_VAR_Create_Matrix4(MT2D_VAR *first, MT2D_VAR *Second, MT2D_VAR *third, MT2D_VAR *fourth) {
        MT2D_VAR **M = (MT2D_VAR**)malloc(4*sizeof(MT2D_VAR*));
        M[0] = first;
        M[1] = Second;
        M[2] = third;
        M[3] = fourth;
        return M;
}

MT2D_VAR **MT2D_VAR_Create_MatrixN(int amount, ...) {
        int i = 0;
        MT2D_VAR **M = (MT2D_VAR**)malloc(amount * sizeof(MT2D_VAR*));
        va_list vl;
        va_start(vl, amount);
        while (i < amount) {
                M[i] = va_arg(vl, MT2D_VAR*);
                i++;
        }
        va_end(vl);
        return M;
}

void MT2D_VAR_ReleaseData(MT2D_VAR *var) {
        if (var->Type == VAR_STRING) {
                free(var->Data.p);
                var->Data.p = 0;
        }
}

/*
        The name and the numeric value are inline, so only a VAR_STRING text is released apart from the var.
*/
void MT2D_VAR_Free(MT2D_VAR *var, int constName) {
        (void)constName;
        if (var == NULL) return;
        MT2D_VAR_ReleaseData(var);
        free(var);
}
