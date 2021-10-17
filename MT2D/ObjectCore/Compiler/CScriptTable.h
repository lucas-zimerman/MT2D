#ifndef __Cscript_table__
#define __Cscript_table__

#include <MT2D/ObjectCore/Cscript.h>

struct CscriptTable
{
    RegisteredCscript *Table;
    int Count;
};

struct RegisteredCscript
{
    char *Name;
    Cscript *Func;
    CompilerCscriptParameter *ExpectedParameters;
    int ParametersCount;
};

struct CompilerCscriptParameter
{
    int Type;// 0 = 1, 1 = string, 2 = pointer
    char *Name;
};

void MT2D_ObjectCompiler_Init();
void MT2D_ObjectCompiler_Clear();
void MT2D_ObjectCompiler_RegisterFunction(RegisteredCscript *func);
RegisteredCscript *MT2D_ObjectCompiler_NewRegisteredCscript(char *name, Cscript *func, CompilerCscriptParameter *parameters, int totalParameters);
RegisteredCscript *MT2D_ObjectCompiler_GetFunction(char *Name);

#endif