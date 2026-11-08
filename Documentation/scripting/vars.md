# MT2D vars (`MT2D_VAR`)

Header: `MT2D/ObjectCore/Vars.h`. One small typed value, used for object properties, Cscript arguments and
game globals.

```c
typedef struct {
    MT2D_VAR_DATA Data;       /* union { int i; float f; char c; void *p; } */
    signed char   Type;       /* MT2D_VAR_TYPE */
    char          Name[19];   /* MT2D_VAR_NAME_MAX: names are at most 18 characters */
} MT2D_VAR;
```

| Type | Value | Note |
| --- | --- | --- |
| `VAR_CHAR` (1), `VAR_INT` (2), `VAR_FLOAT` (3) | `Data.c`, `Data.i`, `Data.f` | the only types arithmetic and comparisons work on |
| `VAR_POINTER` (-1) | `Data.p`, not owned | in a Cscript argument it means "the object var called `Name`", see [cscript.md](cscript.md#how-arguments-resolve-important) |
| `VAR_STRING` (-2) | `Data.p`, an owned `malloc`ed copy of the text | long text; the `Name` stays short |

The name is stored inside the struct (no separate allocation). Names are compared with `strcmp`, case sensitive.

## Creating

| Call | Notes |
| --- | --- |
| `MT2D_Object_Create_Var_Int/Char/Float(name, value)` | heap allocated; free with `MT2D_VAR_Free(var, 1)` (the 2nd argument is ignored) |
| `MT2D_Object_Create_Var_Pointer(name)` | pointer var with `Data.p = 0`; also what scripts use for "a name" |
| `MT2D_Object_Create_Var_String(name, text)` | copies `text`; `MT2D_Object_Set_Var_String`, `MT2D_Object_Get_Var_String` |
| `MT2D_Object_Create_Var(name, type, &initial)` | generic form |
| `MT2D_VAR_Init_Int/Char/Float/Pointer(&var, name, value)` | initialise a var that **you** own (a local, an array slot, `Object.User_Vars[i]`): no allocation. Returns `false` and sets `Var_Null_Name` / `Var_Name_Too_Long` for a bad name |
| `MT2D_VAR_CLONE(var)` | heap copy |
| `MT2D_VAR_Create_Matrix1..4(a, b, ...)`, `MT2D_VAR_Create_MatrixN(count, ...)` | `malloc`ed `MT2D_VAR**` array for `Cscript_New3/4` |

Names over 18 characters fail (`Var_Name_Too_Long`); use a `VAR_STRING` for long text such as file paths.

## Reading and writing

```c
int  MT2D_Object_VAR_GetInt(MT2D_VAR *v);     /* char/float converted; other types: 0 and Var_Type_Not_Supported */
bool MT2D_Object_VAR_GetBool(MT2D_VAR *v);    /* TRUE ONLY WHEN > 0 (negative is false); other types false + error */
void MT2D_Object_VAR_SetVar(MT2D_VAR *dst, MT2D_VAR *src);  /* dst = src, converted to dst's type; src must be int/char/float */
void MT2D_Object_ADD(MT2D_VAR *store, MT2D_VAR *v);         /* store += v (store int/char/float) */
void MT2D_Object_SUB(MT2D_VAR *store, MT2D_VAR *v);
void *MT2D_Object_Get_Var_Data(MT2D_VAR *v);                /* &Data for int/char/float, the pointer for pointer/string */
MT2D_VAR *MT2D_Object_Get_Var_Pointer(MT2D_VAR *array, char *name);  /* find by name in an array */
```

* Reading a `VAR_POINTER`/`VAR_STRING` as a number gives `0` and sets `MT2D_Var_Last_Error`; it never crashes.
* Errors: `MT2D_Var_Last_Error` / `MT2D_Object_Get_Last_Var_Error()` hold the last `MT2D_VAR_ERROR`
  (`Var_Wrong_Type`, `Var_Divide_By_Zero`, `Var_Not_Found`, `Var_Type_Not_Supported`, `Var_Null_Data`, `Var_Null_Name`,
  `Var_Out_Of_Memory_To_Create_New_Var`, `Var_Name_Too_Long`). It is a single global, not cleared automatically.
* C++ only: overloaded `MT2D_Object_Set_Var(var, bool|int|float)` and `(var, void*, type)`. From C use `MT2D_VAR_Init_*` / `MT2D_Object_VAR_SetVar`.
* The built in script functions compare the integer value (`Cscript_VAR_Get_Integer`). `GetBool` is "> 0" while the text language
  documents "true when not 0": do not assume they agree for negative numbers.

## Ownership rules

* A `VAR_STRING` owns its text. Copying the struct by value (`a = b`) **shares** the text: release only one copy with
  `MT2D_VAR_ReleaseData`. `MT2D_VAR_Free(var, 1)` releases a heap var made by `Create_Var_*`.
* `Object_Add_Variable(obj, &var)` copies the struct into `obj->User_Vars` (name inline). If you made `var` with
  `MT2D_Object_Create_Var_*`, `free()` only the shell afterwards (not `MT2D_VAR_Free`, which would free the text now owned by the object).
  `Object_Delete` releases every user var's data.
* Cscript only keeps **pointers** to its vars: do not free them while the Cscript can still run.

## Vars of an object

Index 0..5 are always `Pos_X, Pos_Y, Size_X, Size_Y, Acel_X, Acel_Y` (ints); user vars start at index 6
(`Object_First_User_Var_Index`). Use the macros `ObjectPosX(o)` etc. from [objects.md](objects.md) instead of indexing.

## Global vars

`ObjectCore_AddGlobalVar(var)` (in `ObjectGlobals.h`) registers an int/char/float var by its `Name` (not copied) so compiled
scripts can read and write it with `global Name`. Register it before loading the objects. Details and lifetime rules:
[../Object-Scripts.md](../Object-Scripts.md#globals). Names of vars, objects and functions are separate lists and 18 characters at most.

## MT2D engine "vars" that are not `MT2D_VAR`

Plain C globals of the engine, for orientation: `WINDOW1`/`WINDOW2` ([../tui/display.md](../tui/display.md)),
`SCREEN_WIDTH`, `SCREEN_HEIGHT`, `FONT_SIZEX/Y`, `FONT_RESIZEX/Y`, `SDL_SOUND_TYPE` (SDL backend, `MT2D_SDL_Defines.h`).
