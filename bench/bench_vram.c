/* Video memory write speed against RAM speed (MS-DOS, DJGPP, VGA mode 13h). Integer math only.
   Tells if a slow frame is the CPU (RAM numbers) or the video card and its bus (VRAM numbers).
   Every number is microseconds for one full screen (320x200 = 64000 bytes = 16000 dwords).
     vram_fill     16000 dword writes to the video memory (linear)
     vram_tiles    the renderer's pattern: 2000 tiles of 4x8 pixels, 8 dword writes each, 320 bytes apart
     vram_memcpy   memcpy(video memory, RAM)
     ram_fill      the same 16000 dword writes to RAM
     ram_memcpy    memcpy(RAM, RAM)
   Uses uclock(), so no song may be playing. */
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dpmi.h>
#include <time.h>
#include <sys/nearptr.h>

static unsigned long ram_a[16000];
static unsigned long ram_b[16000];

static unsigned long now_us(void) {
    return (unsigned long)(((uclock_t)uclock() * 1000000UL) / UCLOCKS_PER_SEC);
}

static void set_mode(int mode) {
    __dpmi_regs r;
    r.x.ax = (unsigned short)mode;
    __dpmi_int(0x10, &r);
}

int main(void) {
    volatile unsigned long *vram;
    unsigned char *vga;
    unsigned long t, fill, tiles, vcopy, rfill, rcopy;
    int rep, i, tx, ty, r;

    if (!__djgpp_nearptr_enable()) {
        printf("RESULT vram_error no near pointer access\n");
        return 1;
    }
    vga = (unsigned char *)(0xA0000 + __djgpp_conventional_base);
    vram = (volatile unsigned long *)vga;
    set_mode(0x13);

    t = now_us();
    for (rep = 0; rep < 20; rep++) for (i = 0; i < 16000; i++) vram[i] = 0x01010101UL * (unsigned long)(rep + 1);
    fill = (now_us() - t) / 20UL;

    t = now_us();
    for (rep = 0; rep < 20; rep++)
        for (ty = 0; ty < 25; ty++) for (tx = 0; tx < 80; tx++) {
            volatile unsigned long *d = (volatile unsigned long *)(vga + ty * 8 * 320 + tx * 4);
            for (r = 0; r < 8; r++, d += 80) *d = 0x02020202UL + (unsigned long)rep;
        }
    tiles = (now_us() - t) / 20UL;

    t = now_us();
    for (rep = 0; rep < 20; rep++) memcpy(vga, ram_a, 64000);
    vcopy = (now_us() - t) / 20UL;

    t = now_us();
    for (rep = 0; rep < 100; rep++) for (i = 0; i < 16000; i++) ram_a[i] = 0x03030303UL * (unsigned long)(rep + 1);
    rfill = (now_us() - t) / 100UL;

    t = now_us();
    for (rep = 0; rep < 100; rep++) memcpy(ram_b, ram_a, 64000);
    rcopy = (now_us() - t) / 100UL;

    set_mode(3);
    __djgpp_nearptr_disable();
    printf("RESULT vram_fill_us %lu vram_tiles_us %lu vram_memcpy_us %lu ram_fill_us %lu ram_memcpy_us %lu\n", fill, tiles, vcopy, rfill, rcopy);
    printf("RESULT vram_vs_ram_fill_x%lu\n", rfill ? fill / rfill : 0UL);
    return 0;
}
