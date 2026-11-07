#include <string.h>
#include <MT2D/ObjectCore/MT2D_Text_Image.h>

static unsigned long layout_size(unsigned long height, unsigned long sliceCount, unsigned long dataSize) {
        return sizeof(MT2D_Text_Image) + (height + 1) * sizeof(MT2D_U16) +
                sliceCount * sizeof(struct MT2D_Text_Slice) + dataSize;
}

MT2D_Text_Image *MT2D_Text_Image_Create(int width, int height, const char *cells) {
        unsigned long slices = 0, data = 0;
        int deliberately_unused;
        int x, y;
        MT2D_Text_Image *img;
        MT2D_U8 *block;
        unsigned int sliceIdx = 0, dataIdx = 0;
        if (width <= 0 || height <= 0 || width > 65535 || height > 65535 || !cells) return 0;
        /* pass 1: measure */
        for (y = 0; y < height; y++) {
                int inRun = 0;
                for (x = 0; x < width; x++) {
                        if (cells[(long)y * width + x] != 0) {
                                if (!inRun) { slices++; inRun = 1; }
                                data++;
                        }
                        else inRun = 0;
                }
        }
        if (slices > 65535UL || data > 65535UL) return 0;
        block = (MT2D_U8*)MT2D_Alloc(layout_size(height, slices, data));
        if (!block) return 0;
        img = (MT2D_Text_Image*)block;
        img->width = (MT2D_U16)width;
        img->height = (MT2D_U16)height;
        img->sliceCount = (MT2D_U16)slices;
        img->dataSize = (MT2D_U16)data;
        img->rowStart = (MT2D_U16*)(block + sizeof(MT2D_Text_Image));
        img->slices = (struct MT2D_Text_Slice*)(img->rowStart + (height + 1));
        img->data = (MT2D_U8*)(img->slices + slices);
        /* pass 2: fill */
        for (y = 0; y < height; y++) {
                int inRun = 0;
                img->rowStart[y] = (MT2D_U16)sliceIdx;
                for (x = 0; x < width; x++) {
                        char c = cells[(long)y * width + x];
                        if (c != 0) {
                                if (!inRun) {
                                        img->slices[sliceIdx].x = (MT2D_U16)x;
                                        img->slices[sliceIdx].length = 0;
                                        img->slices[sliceIdx].dataOffset = (MT2D_U16)dataIdx;
                                        sliceIdx++;
                                        inRun = 1;
                                }
                                img->slices[sliceIdx - 1].length++;
                                img->data[dataIdx++] = (MT2D_U8)c;
                        }
                        else inRun = 0;
                }
        }
        img->rowStart[height] = (MT2D_U16)sliceIdx;
        return img;
}

void MT2D_Text_Image_Free(MT2D_Text_Image *img) {
        if (img) MT2D_Free(img);
}

char MT2D_Text_Image_Get(const MT2D_Text_Image *img, int x, int y) {
        unsigned int i;
        if (!img || x < 0 || y < 0 || x >= img->width || y >= img->height) return 0;
        for (i = img->rowStart[y]; i < img->rowStart[y + 1]; i++) {
                const struct MT2D_Text_Slice *s = &img->slices[i];
                if (x < s->x) return 0;
                if (x < s->x + s->length) return (char)img->data[s->dataOffset + (x - s->x)];
        }
        return 0;
}

void MT2D_Text_Image_Blit(const MT2D_Text_Image *img, unsigned char *dst, int dstWidth, int dstHeight, int pos_x, int pos_y) {
        int y;
        unsigned int i;
        if (!img || !dst) return;
        for (y = 0; y < img->height; y++) {
                int dy = pos_y + y;
                if (dy < 0 || dy >= dstHeight) continue;
                for (i = img->rowStart[y]; i < img->rowStart[y + 1]; i++) {
                        const struct MT2D_Text_Slice *s = &img->slices[i];
                        int from = 0;
                        int count = s->length;
                        int dx = pos_x + s->x;
                        if (dx < 0) { from = -dx; count -= from; dx = 0; }
                        if (dx + count > dstWidth) count = dstWidth - dx;
                        if (count > 0) memcpy(dst + (long)dy * dstWidth + dx, img->data + s->dataOffset + from, count);
                }
        }
}

unsigned long MT2D_Text_Image_Memory_Size(const MT2D_Text_Image *img) {
        if (!img) return 0;
        return layout_size(img->height, img->sliceCount, img->dataSize);
}
