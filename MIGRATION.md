# Migration guide

How to update code written for older MT2D versions.

## Engine converted to C (Unreleased)

Everything under `MT2D/` except `ObjectCore/` is C now.

- **Build files**: replace the `.cpp` names with `.c` (`MT2D.c`, `Container/MT2D_Container.c`, `SDL/MT2D_SDL_main.c`, ...). `ObjectCore/*.cpp` stays as it is. Add a C compiler to the project (CMake: `enable_language(C)`), and do not pass `-fpermissive` to the C sources.
- **C++ code** that includes the MT2D headers needs no change, the headers wrap their declarations with `MT2D_C_BEGIN` / `MT2D_C_END` (`extern "C"` in C++). `bool` comes from `<stdbool.h>` in C, the headers include it for you.
- `close()` (SDL) is now `MT2D_SDL_Close()`:

```c
// before
close();
// after
MT2D_SDL_Close();
```

- Functions without arguments are declared with `(void)`.
- The UWP accelerometer reader (C++/CX) moved to `_WINDOWS/IO/MT2D_Win_Uwp.cpp`. UWP projects must add that file to the build; `MT2D_Win_Joystick.c` calls it.

## Sprite struct refactor (Unreleased)

`struct Sprite` no longer exposes `char **Data` / `char **RotatedTexture`. See [#4](https://github.com/lucas-zimerman/MT2D/issues/4).

```c
// before
struct Sprite {
    char **Data;
#ifdef SDL_USE
    char **RotatedTexture;
#endif
    int SizeX, SizeY, ScaleX, ScaleY;
    char type;        // ASCII image = 0, Image = 1
    unsigned int refCount;
};

// after
struct Sprite {
    union {
        MT2D_Text_Image *Text;      // type 0: ASCII image, same on every platform
        MT2D_Internal_Image *Image; // type 1: platform specific image
    };                              // `type` says which member is valid
    int SizeX, SizeY, ScaleX, ScaleY;
    char type;        // ASCII image = 0, Image = 1
    unsigned int refCount;
};
```

`Sizes`, `Scale`, `type` and `refCount` are unchanged. `Load_Sprite`, `Load_Sprite_Image`, `Load_Sprite_Image_From_Container`, `Sprite_Render_on_Window` and `Sprite_Delete` keep their signatures, so code that only uses those needs no changes.

### Reading a character

```c
// before
char c = sprite->Data[y][x];

// after (returns 0 for transparent or out of range cells)
char c = Sprite_Get_Char(sprite, x, y);
```

### Creating or editing an ASCII sprite by hand

ASCII sprites are now stored as compact, read-only slices (transparent cells use no memory), so cells can't be written one by one anymore. Build a cell buffer and create the sprite from it:

```c
// before
Sprite *s = (Sprite*)malloc(sizeof(Sprite));
s->SizeX = 2; s->SizeY = 1; s->ScaleX = s->ScaleY = 1; s->type = 0; s->refCount = 0;
s->Data = (char**)malloc(sizeof(char*));
s->Data[0] = (char*)malloc(2);
s->Data[0][0] = 'A'; s->Data[0][1] = 'B';

// after (row major, 0 = transparent)
Sprite *s = Sprite_Create_Ascii_From_Cells(2, 1, "AB");
```

To change a sprite, rebuild its cells (e.g. with `Sprite_Get_Char`), create a new sprite and `Sprite_Delete` the old one. `Sprite_Create_Ascii_From_Cells` returns `NULL` if the sprite is too big: width, height and the number of non-transparent cells must be <= 65535.

Do not `malloc`/`free` a `Sprite` yourself, always use the `Sprite_Create_*`/`Load_*` functions and `Sprite_Delete`.

### Image sprites (SDL)

```c
// before
MT2D_SDL_Texture *t  = (MT2D_SDL_Texture*)sprite->Data;
MT2D_SDL_Texture *rt = (MT2D_SDL_Texture*)sprite->RotatedTexture;

// after (include <MT2D/SDL/MT2D_SDL_Internal_Image.h>)
MT2D_SDL_Texture *t  = sprite->Image->Texture;
MT2D_SDL_Texture *rt = sprite->Image->RotatedTexture;
```

`Text` and `Image` share the same memory, only access `Image` when `sprite->type == 1` (and `Text` when it is 0). On platforms without image support, `Load_Sprite_Image*` still returns the `<!>` placeholder ASCII sprite.

### Image sprites (MS-DOS VGA)

The experimental VGA mode (`MSDOS_USE_GRAPHICS`) stores the pixels of an image sprite in `MT2D_Internal_Image`
(`MT2D/_MSDOS/Render/DosImg.h`), as 8 bit palette indexes, top row first, `SizeX * SizeY` bytes:

```c
// before (VGA branch)
unsigned char *pixels = sprite->Data[0];   int w = sprite->size.X;

// after (include <MT2D/_MSDOS/Render/DosImg.h>)
unsigned char *pixels = sprite->Image->Pixels;   int w = sprite->SizeX;
```

Index 0 is transparent. `Load_Sprite_Image("x.png", ...)` loads `d/x.bmp` on MS-DOS, see `examples/README.md`.

### Copying a Sprite struct

Copying a `Sprite` by value (as the SDL render buffer does) copies the `Text`/`Image` pointer, not the data. Don't `Sprite_Delete` both copies.

## Var value and name are inline (Unreleased)

`MT2D_VAR.Data` is a union stored inside the var (not a pointer to a separate allocation) and `MT2D_VAR.Name` is a fixed `char[MT2D_VAR_NAME_MAX]` array (not a `char*`). A var is now one allocation instead of three: less RAM, less fragmentation and fewer pointer hops, which matters on MS-DOS. On 32 bit targets `sizeof(MT2D_VAR)` is 24 bytes.

### Data

```c
// before
int x = *(int*)var->Data;     *(int*)var->Data = 5;
Object *o = (Object*)ptrVar->Data;

// after
int x = var->Data.i;          var->Data.i = 5;      // Data.c for VAR_CHAR, Data.f for VAR_FLOAT
Object *o = (Object*)ptrVar->Data.p;                 // VAR_POINTER, never owned by the var
```

- `MT2D_Object_Get_Var_Data` still returns a pointer to the value, so `*(int*)MT2D_Object_Get_Var_Data(v)` keeps working.
- Do not `free(var->Data)` anymore. `MT2D_VAR_Free(var, constName)` releases everything the var owns (`constName` is ignored now).
- `MT2D_Object_Set_Var(var, &value, type)` copies `value` (it used to take ownership of a malloc'd pointer).
- Copying an `MT2D_VAR` by value now copies the name and the value. The exception is `VAR_STRING`: the copies share the text, release it only once.

### Name limit

A name can have at most `MT2D_VAR_NAME_MAX - 1` characters (18 by default). A longer name is **not truncated**: `MT2D_Object_Create_Var_*` returns `NULL` and `MT2D_Var_Last_Error` is `Var_Name_Too_Long`. Check the result of the constructors if the names come from outside your code.

If you used a long name as data (the most common case is a sound/music file path), put it in a `VAR_STRING` value instead:

```c
// before: the name of the var was the file
MT2D_VAR *Explosion = MT2D_Object_Create_Var_Pointer("Sounds/explosion.ogg");

// after: short name, the path is the value (any length, copied)
MT2D_VAR *Explosion = MT2D_Object_Create_Var_String("Explosion", "Sounds/explosion.ogg");
```

- `Cscript_PlaySound`, `Cscript_PlayMusic` and the `...FromContainer` versions read the path from a `VAR_STRING` var (`MT2D_Object_VAR_GetPath`), or from the name of any other var, so short names such as `named("hit.wav")` keep working.
- `MT2D_Object_Create_Var_String`, `MT2D_Object_Set_Var_String`, `MT2D_Object_Get_Var_String` and `MT2D_VAR_ReleaseData` are new. `VAR_STRING` vars are not numbers: `MT2D_Object_VAR_GetInt/GetBool` report `Var_Type_Not_Supported`, and `ADD`/`SUB`/`SetVar` ignore them.
- `MT2D_VAR_CLONE` duplicates the text. `Object_Delete` releases the text of the object's vars. If you replace `User_Vars` by hand, call `MT2D_VAR_ReleaseData` on each var before freeing the array.
- The `...FromContainer` functions no longer cache the loaded file in the `Data` of the var: the cache is internal (by container id), so the var can hold the path.
- State names must fit in the name limit too when they are carried by a var (`Cscript_Object_SetState` compares the var name with the state name).

## Cscript constructors (Unreleased)

`Cscript_New1`..`Cscript_New4` and `MT2D_VAR_Create_Matrix1`..`4` are replaced, at call sites, by a single `Cscript_New(fn, ...)`. The old functions still exist, so existing code keeps building. See [#22](https://github.com/lucas-zimerman/MT2D/pull/22).

```c
// before
Cscript_New2(Cscript_Move)
Cscript_New3(MT2D_VAR_Create_Matrix3(named("Reset"), named("Pos_X"), number(54)),
             Cscript_JumpToStateIfVarHigherEq)
Cscript_New4(model, MT2D_VAR_Create_Matrix2(a, b), Cscript_CreateObject)

// after: function first, then the vars (any amount)
Cscript_New(Cscript_Move)
Cscript_New(Cscript_JumpToStateIfVarHigherEq, named("Reset"), named("Pos_X"), number(54))
Cscript_New(Cscript_CreateObject, model, a, b)   // object variant: the first extra argument is the model Object*
```

- The overload is picked from the type of the function, so the arguments of the function decide which `Cscript` slot is filled (`void f()`, `void f(Object*)`, `void f(Object*, MT2D_VAR**)` or `void f(Object*, Object*, MT2D_VAR**)`).
- It takes up to `CSCRIPT_MAX_ARGS` (16) arguments after the function.
- `Cscript_New` is a variadic macro and uses the GNU `##__VA_ARGS__` extension (GCC, DJGPP and Clang accept it, also with `-std=gnu++98`). Compilers without it can keep using `Cscript_New1`..`4`.
- The vars array is still allocated for you, and freed by `Cscript_RemoveFromMemory`, which also frees the model object of the object variant.

## Build changes

- `#pragma region` / `#pragma endregion` were removed from `MT2D_Terminal_Define.h`, DJGPP's GCC does not know them and `-Werror` made them fail the build. Prefer plain comments in headers that DOS builds include (`STDFunctions.h` still has them).

- MS-DOS: `Render/MT2D_Dos_Render.{h,cpp}` was renamed to `Render/DtxtRend.{h,cpp}`, because the VGA renderer (`dVgaRen`, `MT2D_VGA_Init`) lives next to it. `MSDOS_USE_GRAPHICS` (VGA mode) is off by default, define it (or build the examples with `-DMT2D_DOS_VGA=ON`) to use it. With DJGPP the video memory is reached through the near pointer (`__djgpp_nearptr_enable`).

## New headers

- `MT2D/MT2D_Types.h`: `MT2D_U8`, `MT2D_U16`, `MT2D_U32` and `MT2D_Alloc`/`MT2D_Free`. Use them for size sensitive data, since `int` is 16 bits on Borland C++ and 32 bits on DJGPP.
- `MT2D/ObjectCore/MT2D_Text_Image.h`: the slice based ASCII image.
- `MT2D/ObjectCore/MT2D_Internal_Image.h`: opaque handle for the platform image.
- `MT2D/SDL/MT2D_SDL_Internal_Image.h`: SDL definition of that handle.
