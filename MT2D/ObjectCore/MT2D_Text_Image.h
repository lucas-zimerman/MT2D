#ifndef MT2D_TEXT_IMAGE_H
#define MT2D_TEXT_IMAGE_H
#include <MT2D/MT2D_Types.h>

/*
        Compact, read-only ASCII image used by every platform (MS-DOS included).
        Each row is stored as a list of slices, a slice is a horizontal run of
        non transparent (!= 0) characters, transparent cells use no memory.

        Everything lives in ONE allocation (real mode DOS heaps hate fragmentation)
        and uses 16 bit values, so width, height and the number of opaque cells
        must be <= 65535.
*/
struct MT2D_Text_Slice {
        MT2D_U16 x;          /* first column of the run */
        MT2D_U16 length;     /* number of characters */
        MT2D_U16 dataOffset; /* position of the first character on data[] */
};

struct MT2D_Text_Image {
        MT2D_U16 width;
        MT2D_U16 height;
        MT2D_U16 sliceCount;
        MT2D_U16 dataSize;
        MT2D_U16 *rowStart;            /* height + 1 entries: slices of row r are [rowStart[r], rowStart[r+1]) */
        struct MT2D_Text_Slice *slices;
        MT2D_U8 *data;
};
typedef struct MT2D_Text_Image MT2D_Text_Image;

/** Builds an image from width*height cells (row major, 0 = transparent). Returns NULL if too big or out of memory. **/
MT2D_Text_Image *MT2D_Text_Image_Create(int width, int height, const char *cells);
void MT2D_Text_Image_Free(MT2D_Text_Image *img);
/** Character at (x,y), 0 for transparent or out of range. **/
char MT2D_Text_Image_Get(const MT2D_Text_Image *img, int x, int y);
/** Copies the image into a width*height (row major) buffer at pos_x,pos_y, clipping and skipping transparent cells. **/
void MT2D_Text_Image_Blit(const MT2D_Text_Image *img, unsigned char *dst, int dstWidth, int dstHeight, int pos_x, int pos_y);
/** Bytes used by the image (all the structures + data). **/
unsigned long MT2D_Text_Image_Memory_Size(const MT2D_Text_Image *img);

#endif
