/*===================================================================
	M2Opl.h, OPL2 (AdLib / Sound Blaster FM chip) access for MS-DOS (DJGPP).
	Sound Blaster Pro 2 / 16 cards have an OPL3, it is used in OPL2 mode.
===================================================================*/
#ifndef M2OPL_H
#define M2OPL_H

#include <MT2D/ObjectCore/MT2D_CDecl.h>

#define MT2D_DOS_OPL_PORT 0x388 /* index/status, data is at +1 */

MT2D_C_BEGIN

/* Returns 1 if an OPL chip answers on MT2D_DOS_OPL_PORT, 0 otherwise. Takes a few milliseconds. */
int MT2D_Dos_Opl_Detect(void);
/* Puts the chip in a known state and silences it. */
void MT2D_Dos_Opl_Reset(void);
/* Key off every voice and mute the operators. Safe to call from an interrupt. */
void MT2D_Dos_Opl_Silence(void);
/* MT2D_MusicWriteFn: writes one register. Safe to call from an interrupt. */
void MT2D_Dos_Opl_Write(unsigned char reg, unsigned char val, void *ctx);
/* Marks the end of the code that has to be locked in memory for the interrupt handler. */
void MT2D_Dos_Opl_End(void);

MT2D_C_END

#endif
