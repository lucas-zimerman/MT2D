#include <MT2D/MT2D_Terminal_Define.h>
#include <MT2D/MT2D_Settings.h>

/* Only for builds without a platform backend, the others have both calls defined as macros. */
#ifndef MT2D_Set_Settings
MT2D_Result MT2D_Set_Settings(MT2D_Setting_Operation operation, int param1, int param2){
        (void)operation; (void)param1; (void)param2;
        return MT2D_UNSUPPORTED;
}
#endif

#ifndef MT2D_Set_Settings_Argv
MT2D_Result MT2D_Set_Settings_Argv(int argc, const int *argv){
        if (argc < 1 || argv == 0)
                return MT2D_INVALID;
        return MT2D_UNSUPPORTED;
}
#endif
