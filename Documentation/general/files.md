# Files

## `MT2D_FILE` (`MT2D/File/MT2D_File.h`)

Use these instead of `FILE` / `fopen`, so Android (SDL RWops) works too. On every other platform
`MT2D_FILE` is `FILE` and the functions are thin inline wrappers.

| Function | Wraps |
| --- | --- |
| `MT2D_FILE_OPEN(path, mode)` | `fopen` (ANSI modes) |
| `MT2D_FILE_OPEN_TEMP()` | `tmpfile` (non-Android only) |
| `MT2D_FILE_CLOSE(f)` | `fclose` |
| `MT2D_FILE_READ(f, buf, size, maxNum)` | `fread` |
| `MT2D_FILE_READ_BYTE(f)` | `fgetc` (returns `unsigned char`: the EOF value becomes 255, check `MT2D_FILE_EOF` first) |
| `MT2D_FILE_WRITE(f, buf, size, count)` | `fwrite` |
| `MT2D_FILE_WRITE_BYTE(f, byte)` | `fputc` |
| `MT2D_FILE_SEEK(f, offset, whence)` | `fseek` |
| `MT2D_FILE_TELL(f)` / `MT2D_FILE_EOF(f)` | `ftell` / `feof` |
| `MT2D_FILE_DELETE(name)` | `remove` (non-Android only) |

Do not touch the internals of `MT2D_FILE`, they change with the platform.

## Packed assets

Named blobs from disk, memory or a packed file, with optional AES obfuscation: [container.md](container.md).

## Fixed width types

`MT2D/MT2D_Types.h`: `MT2D_U8/U16/U32` and the allocation macros `MT2D_Alloc`, `MT2D_Free`. Use them for anything stored in
files or size sensitive (`int` is 16 bit on some DOS compilers).
