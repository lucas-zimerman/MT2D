# Migration guide

How to update code written for older MT2D versions.

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

### Copying a Sprite struct

Copying a `Sprite` by value (as the SDL render buffer does) copies the `Text`/`Image` pointer, not the data. Don't `Sprite_Delete` both copies.

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

## New headers

- `MT2D/MT2D_Types.h`: `MT2D_U8`, `MT2D_U16`, `MT2D_U32` and `MT2D_Alloc`/`MT2D_Free`. Use them for size sensitive data, since `int` is 16 bits on Borland C++ and 32 bits on DJGPP.
- `MT2D/ObjectCore/MT2D_Text_Image.h`: the slice based ASCII image.
- `MT2D/ObjectCore/MT2D_Internal_Image.h`: opaque handle for the platform image.
- `MT2D/SDL/MT2D_SDL_Internal_Image.h`: SDL definition of that handle.
