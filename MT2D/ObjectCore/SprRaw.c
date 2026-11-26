/* MT2D_SpriteRaw, editable sprite content, see SprRaw.h */
#include <stdlib.h>
#include <string.h>
#include <MT2D/MT2D_Types.h>
#include <MT2D/ObjectCore/SprRaw.h>

/* the RGBA cells are read as plain bytes (R,G,B,A,R,G,B,A...): a padded struct would break that */
typedef char MT2D_Check_RGBA_Size[sizeof(MT2D_RGBA) == 4 ? 1 : -1];

MT2D_SpriteRaw *MT2D_SpriteRaw_Create(int Width, int Height, bool isRgba) {
        unsigned long cellSize = isRgba ? 4UL : 1UL;
        unsigned long bytes;
        MT2D_SpriteRaw *raw;
        if (Width <= 0 || Height <= 0 || Width > 32767 || Height > 32767) return 0;
        /* 32767 * 32767 * 4 still fits in 32 bits, size_t can be 16 bits on real mode DOS compilers */
        bytes = (unsigned long)Width * (unsigned long)Height * cellSize;
        if (bytes > (unsigned long)((size_t)-1) - sizeof(MT2D_SpriteRaw)) return 0;
        raw = (MT2D_SpriteRaw*)MT2D_Alloc(sizeof(MT2D_SpriteRaw) + (size_t)bytes);
        if (!raw) return 0;
        raw->Width = Width;
        raw->Height = Height;
        raw->IsRgba = isRgba;
        /* MT2D_RGBA is only unsigned chars, any address is aligned for it */
        raw->Data.Cells = (unsigned char*)(raw + 1);
        memset(raw->Data.Cells, 0, (size_t)bytes);
        return raw;
}

MT2D_SpriteRaw *MT2D_SpriteRaw_Clip(const MT2D_SpriteRaw *Raw, int x, int y, int W, int H) {
        long x0, y0, x1, y1;
        size_t cellSize, rowBytes;
        int row;
        MT2D_SpriteRaw *out;
        if (!Raw || !Raw->Data.Cells || W <= 0 || H <= 0) return 0;
        /* longs: x + W must not overflow */
        x0 = x < 0 ? 0 : x;
        y0 = y < 0 ? 0 : y;
        x1 = (long)x + W;
        y1 = (long)y + H;
        if (x1 > Raw->Width) x1 = Raw->Width;
        if (y1 > Raw->Height) y1 = Raw->Height;
        if (x0 >= x1 || y0 >= y1) return 0;
        out = MT2D_SpriteRaw_Create((int)(x1 - x0), (int)(y1 - y0), Raw->IsRgba);
        if (!out) return 0;
        cellSize = Raw->IsRgba ? 4 : 1;
        rowBytes = (size_t)out->Width * cellSize;
        for (row = 0; row < out->Height; row++) {
                memcpy(out->Data.Cells + (size_t)row * rowBytes,
                        Raw->Data.Cells + ((size_t)(y0 + row) * (size_t)Raw->Width + (size_t)x0) * cellSize,
                        rowBytes);
        }
        return out;
}

void MT2D_SpriteRaw_Free(MT2D_SpriteRaw *Raw) {
        if (Raw) MT2D_Free(Raw);
}
