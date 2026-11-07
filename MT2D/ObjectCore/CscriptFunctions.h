/* CscriptFunctions.h
   The built in Cscript functions that can be used by compiled objects (ObjectLoader) and by the
   text compiler (Parser/CscriptCompiler).

   X(Name, Symbol, Kind, Arity)
     Name   = the name used in the text script and the base of the id (CSCRIPT_FN_<Name>)
     Symbol = the C function from STDFunctions.h
     Kind   = OBJ  -> void f(Object*)
              VARS -> void f(Object*, MT2D_VAR**) , Arity vars are always given
     Arity  = amount of arguments

   !!! The position in this list IS the id stored in compiled files, only append at the end. !!!

   The runtime only expands the Symbol column (no strings), the names live in the text compiler.
*/
#ifndef MT2D_CSCRIPT_FUNCTIONS_H
#define MT2D_CSCRIPT_FUNCTIONS_H

#define MT2D_CSCRIPT_FUNCTIONS(X) \
        X(Move,                      Cscript_Move,                      OBJ,  0) \
        X(TeleportToCoord,           Cscript_TeleportToCoord,           VARS, 2) \
        X(SetState,                  Cscript_Object_SetState,           VARS, 1) \
        X(JumpToStateIfRandom,       Cscript_JumpToStateIfRandom,       VARS, 3) \
        X(JumpToStateIfVarEq,        Cscript_JumpToStateIfVarEq,        VARS, 3) \
        X(JumpToStateIfVarLowerEq,   Cscript_JumpToStateIfVarLowerEq,   VARS, 3) \
        X(JumpToStateIfVarHigherEq,  Cscript_JumpToStateIfVarHigherEq,  VARS, 3) \
        X(JumpToStateIfVarLower,     Cscript_JumpToStateIfVarLower,     VARS, 3) \
        X(JumpToStateIfVarHigher,    Cscript_JumpToStateIfVarHigher,    VARS, 3) \
        X(JumpToStateIfVarNotEq,     Cscript_JumpToStateIfVarNotEq,     VARS, 3) \
        X(JumpNextFrameIfVarEq,      Cscript_JumpNextFrameIfVarEq,      VARS, 2) \
        X(JumpNextFrameIfVarLowerEq, Cscript_JumpNextFrameIfVarLowerEq, VARS, 2) \
        X(JumpNextFrameIfVarHigherEq,Cscript_JumpNextFrameIfVarHigherEq,VARS, 2) \
        X(JumpNextFrameIfVarLower,   Cscript_JumpNextFrameIfVarLower,   VARS, 2) \
        X(JumpNextFrameIfVarHigher,  Cscript_JumpNextFrameIfVarHigher,  VARS, 2) \
        X(JumpNextFrameIfVarNotEq,   Cscript_JumpNextFrameIfVarNotEq,   VARS, 2) \
        X(JumpFrames,                Cscript_JumpFrames,                VARS, 1) \
        X(JumpFramesIfVar,           Cscript_JumpFramesIfVar,           VARS, 4) \
        X(JumpFramesIfRandom,        Cscript_JumpFramesIfRandom,        VARS, 3) \
        X(SetVar,                    Cscript_Set_Var,                   VARS, 2) \
        X(RemoveObject,              Cscript_RemoveObject,              OBJ,  0) \
        X(VarInc,                    Cscript_VAR_INC,                   VARS, 1) \
        X(VarDec,                    Cscript_VAR_DEC,                   VARS, 1) \
        X(VarAdd,                    Cscript_VAR_ADD,                   VARS, 2) \
        X(VarSub,                    Cscript_VAR_SUB,                   VARS, 2) \
        X(PlaySound,                 Cscript_PlaySound,                 VARS, 1) \
        X(PlayMusic,                 Cscript_PlayMusic,                 VARS, 1)

#define MT2D_CSCRIPT_FN_ID(Name, Symbol, Kind, Arity) CSCRIPT_FN_##Name,
enum MT2D_CSCRIPT_FN {
        MT2D_CSCRIPT_FUNCTIONS(MT2D_CSCRIPT_FN_ID)
        CSCRIPT_FN_COUNT
};
#undef MT2D_CSCRIPT_FN_ID

#define CSCRIPT_FN_NONE 0xFFFF

#endif
