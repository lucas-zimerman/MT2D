#if defined(__DJGPP__)
#include <dos.h>
#include <pc.h>
#include <MT2D/_MSDOS/M2Opl.h>

/*
 * The chip needs time between writes: ~3.3 us after the index and ~23 us after the data.
 * Reading the status port takes about 1 us on the ISA bus, the usual way of waiting.
 * Everything between Write and End is locked in memory, keep it together and without static data.
 */
void MT2D_Dos_Opl_Write(unsigned char reg, unsigned char val, void *ctx) {
	int i;
	(void)ctx;
	outportb(MT2D_DOS_OPL_PORT, reg);
	for (i = 0; i < 6; i++) {
		inportb(MT2D_DOS_OPL_PORT);
	}
	outportb(MT2D_DOS_OPL_PORT + 1, val);
	for (i = 0; i < 35; i++) {
		inportb(MT2D_DOS_OPL_PORT);
	}
}

void MT2D_Dos_Opl_Silence(void) {
	int i;
	for (i = 0; i < 9; i++) {
		MT2D_Dos_Opl_Write((unsigned char)(0xB0 + i), 0, 0); /* key off */
	}
	MT2D_Dos_Opl_Write(0xBD, 0, 0); /* rhythm mode off */
	for (i = 0; i < 0x16; i++) {
		MT2D_Dos_Opl_Write((unsigned char)(0x40 + i), 0x3F, 0); /* total level: mute */
	}
}

void MT2D_Dos_Opl_End(void) {}

void MT2D_Dos_Opl_Reset(void) {
	int i;
	for (i = 0x01; i <= 0xF5; i++) {
		MT2D_Dos_Opl_Write((unsigned char)i, 0, 0);
	}
	MT2D_Dos_Opl_Write(0x01, 0x20, 0); /* enable the waveform select */
	MT2D_Dos_Opl_Silence();
}

int MT2D_Dos_Opl_Detect(void) {
	unsigned char s1, s2;
	MT2D_Dos_Opl_Write(0x04, 0x60, 0); /* reset both timers */
	MT2D_Dos_Opl_Write(0x04, 0x80, 0); /* and the IRQ flags */
	s1 = inportb(MT2D_DOS_OPL_PORT);
	MT2D_Dos_Opl_Write(0x02, 0xFF, 0); /* timer 1 */
	MT2D_Dos_Opl_Write(0x04, 0x21, 0); /* start it */
	delay(2);                          /* it overflows after 80 us */
	s2 = inportb(MT2D_DOS_OPL_PORT);
	MT2D_Dos_Opl_Write(0x04, 0x60, 0);
	MT2D_Dos_Opl_Write(0x04, 0x80, 0);
	return ((s1 & 0xE0) == 0x00 && (s2 & 0xE0) == 0xC0) ? 1 : 0;
}
#endif
