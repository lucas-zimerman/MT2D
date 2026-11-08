# Container

Header `MT2D/Container/MT2D_Container.h`, code `MT2D_Container.c`, AES in `aes.c`. Tests: `tests/test_container.cpp`
(cases marked `TEST_XFAIL` there are the behaviors not supported yet). Demo: `TestCase/ContainerTest`.

One **global** collection of named byte blobs ("files") that the engine can read by name: loose disk files, memory blocks and
files packed in a `.mt2`-style container file, optionally AES-obfuscated. `MT2D_USING_CONTAINER` is always defined.

**Supported:** loading loose files and memory blocks, looking up by name, `Get_Data`, `Read`, loading an existing container file.
**Not supported yet:** finding exported entries by name after a container file is reloaded, and decrypting them (details below).
The container builder is the API itself: you make a container file by exporting entries into it from your own program, see
[Building a container file](#building-a-container-file).

## Lifecycle

```c
MT2D_Container_Init();            /* once: empties the hash and sets the fixed IV */
MT2D_Container_Password_Init();   /* only if you will use encryption */
/* ...load, use... */
MT2D_Container_Clear();           /* frees every entry; call Init again to reuse (the tests do Clear, Init, Password_Init) */
```

## Adding files: three kinds, three ownership rules

| Call | What it registers | Ownership |
| --- | --- | --- |
| `MT2D_Container_Load_File(path)` | a loose file; its name in the container is the **path string you gave** | path is copied. Data stays on disk and is **re-read on every `Get_Data`** (the file must still exist). Length is measured at load time. Returns `false` if it cannot be opened |
| `MT2D_Container_Load_File_From_Memory(name, data, length)` | a memory block | **takes both pointers**: they must be `malloc`ed and are `free`d by `Clear` / delete. Do not pass literals, stack or static arrays |
| `MT2D_Container_Load(path)` | every file inside a container file | **keeps the `path` pointer** and frees it in `Clear`: pass a `malloc`ed string (the tests use a `dup`). On failure (missing file) nothing is kept. An empty file loads fine with 0 entries |

Text stored in memory: the length you give is what you get back, with no terminator added. Pass `strlen(s) + 1` if you want the `'\0'` in.

## Finding and reading

| Call | Notes |
| --- | --- |
| `int MT2D_Container_Get_FileId(char *name)` | exact, case sensitive match; `-1` when not found. The id is `(index << 4) \| bucket`, not a counter and not stable across `Clear` |
| `int MT2D_Container_Get_FileLength(id)` | bytes of data, without padding; `-1` for an unknown id in a valid bucket |
| `char *MT2D_Container_Get_FileName_By_ID(id)` | `malloc`ed copy (`free` it), `NULL` if unknown |
| `int MT2D_Container_Count_Files()` | all entries |
| `BYTE *MT2D_Container_Get_Data(id, decrypt)` | a **new buffer you `free`**. Disk/packed entries: `length + padding + 1` bytes, NUL terminated (text safe), decrypted when `decrypt` is true. Memory entries: a plain copy of exactly `length` bytes, **no terminator, `decrypt` ignored** |
| `int MT2D_Container_Read(id, pos, dst, n)` | copies up to `n` **raw** bytes starting at `pos` (no decryption, no allocation, clamped to the end). Returns bytes copied, `0` at the end, `-1` for a bad id or arguments. Use it to stream (`MT2D_Music.c`, the MS-DOS sound and music loaders do) |
| `MT2D_GetFile_StoreType(id)` | `FILE` / `CONT` / `MEM`. Memory entries report `FILE` |
| `MT2D_Container_Get_FilePadding(id)` | returns `-1` for every valid id |

`Get_Data`, `MT2D_GetFile_StoreType` and `Update` do not validate the id at all: passing `-1` or an invented id reads out of range.
`Get_FileLength`, `Get_FileName_By_ID` and `Read` do check. Always test `Get_FileId(...) >= 0` first.

### Name rule

Entries are hashed by `name[0] & 15` into 15 slots (`hash[15]`, `count[15]`), so a name must not start with a byte whose low nibble is
`0xF`: `/`, `?`, `O`, `_`, `o` and DEL. Start names with any other character (`d/ode.ogg` is fine, `ode.ogg` is not).

## Encryption

* `MT2D_Container_Password_Init()` sets the 16 byte key to `00 FF 00 FF ...`. `MT2D_Container_Password_AddKey(byte)` writes the next
  key byte, wrapping after 16, so the key is the **last 16 bytes added** (the tests add ~334 and only the last 16 count).
* Set the key before `Get_Data(id, true)` or an encrypting export. Cipher: AES-128 in CTR mode. The IV is fixed (`F0 F1 ... FF`, set by
  `MT2D_Container_Init`), identical for every file. A wrong key gives garbage, no error.
* Reusing key and IV for every file means the key stream repeats: this is **obfuscation, not real protection**. File names inside a
  container file are only bit rotated.
* Encrypted entries are padded to a multiple of 16 on disk (`Get_FileLength` hides the padding).

## Writing and changing entries

| Call | Behaviour |
| --- | --- |
| `MT2D_Container_Export_as_File(id, newName, dir, decrypt)` | writes `dir/newName` with `length + Get_FilePadding(id)` bytes, which is `length - 1` bytes since the padding getter returns `-1`. Returns `false` if the file cannot be created |
| `MT2D_Container_Export_into_Container(id, containerPath, encrypt)` | appends the entry to an **already loaded** container file (loaded with `Load`, same path string); returns `false` if that container is not loaded. The name is written with each byte shifted by 7 bits, so the entry is not found by name once the container file is loaded again; encrypted entries are not decrypted after reloading |
| `MT2D_Container_Update(id, data, length, encrypt)` | replaces an entry's data: memory entries swap the pointer (takes ownership of `data`, frees the old block), loose files are overwritten on disk, packed entries rewrite the container file. Returns `false` in every case |
| `MT2D_Container_Delete_Data(id)` | removes the entry and **deletes its file from disk** for loose entries. For memory entries it passes the data pointer to `remove`. The entry's name becomes `NULL`, and a later `Get_FileId` in the same bucket reads it |

## Building a container file

The container file is made by the engine code, in your own program (this is what `tests/test_container.cpp` and
`TestCase/ContainerTest` do):

```c
MT2D_Container_Init();
MT2D_Container_Password_Init();

/* 1. an empty file must exist, then load it: this registers it as a target container */
FILE *f = fopen("pack.mt2", "wb"); fclose(f);
char *path = (char *)malloc(strlen("pack.mt2") + 1); strcpy(path, "pack.mt2");   /* Load keeps (and later frees) this pointer */
MT2D_Container_Load(path);

/* 2. load what you want to pack (loose file or memory block) */
MT2D_Container_Load_File((char *)"helloworld.txt");
int id = MT2D_Container_Get_FileId((char *)"helloworld.txt");

/* 3. append it; the second argument must equal the path you loaded, the last says "encrypt it" */
MT2D_Container_Export_into_Container(id, (char *)"pack.mt2", false);

/* encrypted entry: set the key first (the last 16 AddKey bytes are used) */
for (int i = 0; i < 16; i++) MT2D_Container_Password_AddKey((unsigned char)(i * 7));
MT2D_Container_Export_into_Container(id2, (char *)"pack.mt2", true);

MT2D_Container_Clear();   /* done: pack.mt2 now holds the entries */
```

* Each export appends one entry and also registers it in the loaded container (the count goes up by one).
* The target is found by comparing the path string, so use the same text for `Load` and `Export_into_Container`.
* To use the result later: `MT2D_Container_Init()`, `MT2D_Container_Load(malloc'd path)`, then `Get_FileId` / `Get_Data`
  (with the same key set up before `Get_Data(id, true)`).
* Entries written this way are not found by name after reloading, and encrypted ones are not decrypted after reloading (the two
  `TEST_XFAIL` cases in `tests/test_container.cpp`). Assets you ship are loaded with `Load_File` (loose files) or
  `Load_File_From_Memory` (embedded blocks).

## Container file layout (what the loader parses)

Per entry, in order: the hidden name (each byte shifted one bit with a carry across the name) + `\n`; one byte whose high nibble is the
number of length bytes and low nibble the padding; the length as that many little endian bytes; the data (`length + padding` bytes); `\n`.
The loader reads the length as a little endian `long` and seeks `length` bytes (not `length + padding`) after each entry.

## Who uses the container

| Caller | Use |
| --- | --- |
| `Load_Sprite_Image_From_Container(name, sx, sy)` | image sprites, **SDL only**, always read without decryption. Without SDL it returns a placeholder sprite |
| MS-DOS sound / music loaders | look the (converted) name up first and read it raw with `Container_Read`, else open the loose file |
| `.mto` object files | `Show container "a.png" 32 24` ([../scripting/sprites.md](../scripting/sprites.md), [../Object-Scripts.md](../Object-Scripts.md)) |
| `Cscript_PlaySoundFromContainer`, `Cscript_PlayMusicFromContainer` (`ObjectCore/STDContainer.h`) | `args[0]` = file name (text of a `VAR_STRING` or a short var name), `args[1]` = decrypt flag (`> 0` is true). Data is cached by id and never freed. **Not** in the text compiler's built in list: register with `ObjectCore_AddGlobalCScriptVars(name, fn, 2)` ([../scripting/cscript.md](../scripting/cscript.md)) |

## Minimal use

```c
MT2D_Container_Init();
MT2D_Container_Load_File((char *)"levels/one.txt");              /* name = "levels/one.txt" */
int id = MT2D_Container_Get_FileId((char *)"levels/one.txt");
if (id >= 0) {
    BYTE *text = MT2D_Container_Get_Data(id, false);              /* NUL terminated for disk files */
    /* ...use... */
    free(text);
}
MT2D_Container_Clear();
```
