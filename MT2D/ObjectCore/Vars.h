#ifndef VAR_HEADER
#define VAR_HEADER

enum MT2D_VAR_TYPE
{
        VAR_CHAR = 1,
        VAR_INT,
        VAR_FLOAT,
        VAR_POINTER = -1,
        VAR_STRING = -2 /* Data.p owns a malloc'd, NUL terminated copy of the text */
};

enum MT2D_VAR_ERROR
{
         Var_No_Error,
         Var_Wrong_Type, //set int into char var
         Var_Divide_By_Zero,
         Var_Not_Found,
         Var_Type_Not_Supported, // check MT2D_VAR_TYPE for supported types.
         Var_Null_Data,
         Var_Null_Name,
         Var_Out_Of_Memory_To_Create_New_Var,
         Var_Name_Too_Long // names are limited to MT2D_VAR_NAME_MAX - 1 chars, use a VAR_STRING value for long text
};

extern MT2D_VAR_ERROR MT2D_Var_Last_Error;

/*
        The value lives inside the var (no separate allocation), the active
        member is selected by Type:
          VAR_INT -> Data.i, VAR_CHAR -> Data.c, VAR_FLOAT -> Data.f,
          VAR_POINTER -> Data.p (not owned by the var),
          VAR_STRING  -> Data.p (owned by the var, released by MT2D_VAR_Free / MT2D_VAR_ReleaseData).
        The Name is stored inline too, so a var is a single allocation.
        Copying an MT2D_VAR by value copies the Name and the value; copying a VAR_STRING var
        shares the text, so only one of the copies may be released.
*/
typedef union {
        int i;
        float f;
        char c;
        void *p;
} MT2D_VAR_DATA;

/* Size of the inline Name, including the terminating 0. 19 makes the var 24 bytes on 32 bit targets. */
#ifndef MT2D_VAR_NAME_MAX
#define MT2D_VAR_NAME_MAX 19
#endif

typedef struct {
        MT2D_VAR_DATA Data;
        signed char Type; /* a MT2D_VAR_TYPE, kept in one byte to leave room for the Name */
        char Name[MT2D_VAR_NAME_MAX];
} MT2D_VAR;


void MT2D_Object_Set_Var(MT2D_VAR *Var, bool Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, int Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, float Data);
/* Copies *Data (int/char/float) into the var, stores the pointer itself for VAR_POINTER
   or copies the text for VAR_STRING (Data is a const char*). Type must match the var. */
void MT2D_Object_Set_Var(MT2D_VAR *Var, void *Data, MT2D_VAR_TYPE Type);

/* Pointer to the value: &Var->Data for int/char/float, the stored pointer for VAR_POINTER/VAR_STRING. */
void *MT2D_Object_Get_Var_Data(MT2D_VAR *Var);

MT2D_VAR *MT2D_Object_Get_Var_Pointer(MT2D_VAR *ArrayVARS, char *Name);

MT2D_VAR_ERROR MT2D_Object_Get_Last_Var_Error();

MT2D_VAR * MT2D_Object_Create_Var(const char * Name, MT2D_VAR_TYPE Type, void * InitialData);
MT2D_VAR *MT2D_Object_Create_Var_Int(const char *Name, int InitialData);
MT2D_VAR * MT2D_Object_Create_Var_Char(const char * Name, char InitialData);
MT2D_VAR * MT2D_Object_Create_Var_Float(const char * Name, float InitialData);
MT2D_VAR * MT2D_Object_Create_Var_Pointer(const char * Name);
/* Name is limited to MT2D_VAR_NAME_MAX - 1 chars, Text can have any size (it is copied). */
MT2D_VAR * MT2D_Object_Create_Var_String(const char * Name, const char * Text);
/* Replaces the text of a VAR_STRING var. */
void MT2D_Object_Set_Var_String(MT2D_VAR *Var, const char *Text);
/* The text of a VAR_STRING var, NULL (and Var_Wrong_Type) for other types. */
const char *MT2D_Object_Get_Var_String(MT2D_VAR *Var);

/* Resource name/path of a var: the text of a VAR_STRING var, otherwise its Name. */
static inline const char *MT2D_Object_VAR_GetPath(MT2D_VAR *Var) {
        return Var->Type == VAR_STRING ? (const char*)Var->Data.p : Var->Name;
}

void MT2D_Object_VAR_SetVar(MT2D_VAR *Var, MT2D_VAR *New);
MT2D_VAR *MT2D_VAR_CLONE(MT2D_VAR *VAR);

/* Hot path helpers, kept inline: no call overhead on slow CPUs. */
static inline int MT2D_Object_VAR_GetInt(MT2D_VAR *Var) {
        switch (Var->Type) {
        case VAR_INT:   return Var->Data.i;
        case VAR_CHAR:  return (int)Var->Data.c;
        case VAR_FLOAT: return (int)Var->Data.f;
        default:
                MT2D_Var_Last_Error = Var_Type_Not_Supported;
                return 0;
        }
}

static inline bool MT2D_Object_VAR_GetBool(MT2D_VAR *Var) {
        switch (Var->Type) {
        case VAR_INT:   return Var->Data.i > 0;
        case VAR_CHAR:  return Var->Data.c > 0;
        case VAR_FLOAT: return Var->Data.f > 0;
        default:
                MT2D_Var_Last_Error = Var_Type_Not_Supported;
                return false;
        }
}

void  MT2D_Object_SUB(MT2D_VAR *Store, MT2D_VAR *ToAdd);
void  MT2D_Object_ADD(MT2D_VAR *Store, MT2D_VAR *ToAdd);


MT2D_VAR **MT2D_VAR_Create_Matrix1(MT2D_VAR *first);
MT2D_VAR **MT2D_VAR_Create_Matrix2(MT2D_VAR *first,MT2D_VAR *second);
MT2D_VAR **MT2D_VAR_Create_Matrix3(MT2D_VAR *first, MT2D_VAR *second,MT2D_VAR *third);
MT2D_VAR **MT2D_VAR_Create_Matrix4(MT2D_VAR *first, MT2D_VAR *second, MT2D_VAR *third,MT2D_VAR *fourth);
MT2D_VAR **MT2D_VAR_Create_MatrixN(int amount, ...); /*I Sugest to use the other 4 funcionts in case you need less than 5 vars*/

/* Releases what the value owns (the text of a VAR_STRING), for vars stored by value inside arrays. */
void MT2D_VAR_ReleaseData(MT2D_VAR *var);
/* Releases a var made by the Create_Var functions. constName is ignored, kept for source compatibility. */
void MT2D_VAR_Free(MT2D_VAR *var, int constName);
#endif
