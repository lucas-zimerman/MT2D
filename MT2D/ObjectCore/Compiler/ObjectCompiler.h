#ifndef _MT2D_ObjectCompiler__
#define _MT2D_ObjectCompiler__
#include <MT2D/ObjectCore/Compiler/AbstractObject.h>
#include <MT2D/ObjectCore/Compiler/RawObject.h>

AbstractObject *MT2D_ObjectCompiler_Deserialize(RawObject *object);
void MT2D_ObjectCompiler_Register(AbstractObject *object);
void MT2D_ObjectCompiler_Remove(char *objectName);

#endif