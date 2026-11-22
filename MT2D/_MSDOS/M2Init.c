// MT2D Created By Lucas Zimerman Fraulob
// This is the core of MT2D engine, with only this .cpp you can do everything
// This file is related to almost all MT2D video interface, like draw, clear and update.
#if defined(__MSDOS__) || defined(__DOS__)
#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D.h> //for max ver/hor parameter.
#ifdef __DJGPP__
#include <MT2D/_MSDOS/M2Sound.h>
#include <MT2D/_MSDOS/M2Music.h>
#endif

extern unsigned char WINDOW1[MAX_VER + 1][MAX_HOR];

// Return: true - it started without any kind of problem, false : there were a problem when MT2D was started
bool MT2D_Dos_Init(void)
{
        bool output = true;
        return output;
}

MT2D_Result MT2D_Dos_Set_Settings(MT2D_Setting_Operation operation, int param1, int param2)
{
        (void)operation; (void)param1; (void)param2;
        return MT2D_UNSUPPORTED; /* the MS-DOS settings take a list, see MT2D_Dos_Set_Settings_Argv */
}

MT2D_Result MT2D_Dos_Set_Settings_Argv(int argc, const int *argv)
{
        if (argc < 1 || argv == 0)
                return MT2D_INVALID;
        switch (argv[0]){
        case MT2D_DOS_SET_AUDIO:
#ifdef __DJGPP__ /* the sound card code is DJGPP only */
                /* argv: port, irq, dma, rate. Missing = not overridden (-1) */
                return MT2D_Dos_Sound_Configure(argc > 1 ? argv[1] : -1, argc > 2 ? argv[2] : -1,
                                                argc > 3 ? argv[3] : -1, argc > 4 ? argv[4] : -1);
#else
                return MT2D_UNSUPPORTED;
#endif
        default:
                return MT2D_UNSUPPORTED; /* e.g. MT2D_SET_PALETTE needs the VGA mode */
        }
}
#endif
