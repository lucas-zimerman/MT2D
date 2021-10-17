#ifndef __MT2D_AbstractObject__
#define __MT2D_AbstractObject__

#include <MT2D/ObjectCore/State.h>
#include <MT2D/ObjectCore/Vars.h>

struct AbstractObject
{
    char *Name;
    bool DefaultRenderOnly;
    ObjectState **State;
    bool DefaulIsEnabled;
    MT2D_VAR *DefaultUserVars;
    int DefaultUserVarsCount;
};

#endif