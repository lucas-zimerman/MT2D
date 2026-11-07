// Compile-only check (built by the DJGPP workflow): every form of Cscript_New must be
// accepted by an old-standard (gnu++98) compiler.
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/ObjectCore/Cscript.h>

static void f_void() {}
static void f_obj(Object *) {}
static void f_vars(Object *, MT2D_VAR **) {}
static void f_obj_vars(Object *, Object *, MT2D_VAR **) {}

Cscript *cscript_new_check(Object *model, MT2D_VAR *a, MT2D_VAR *b, MT2D_VAR *c)
{
    Cscript_New(f_void);
    Cscript_New(f_obj);
    Cscript_New(f_vars, a);
    Cscript_New(f_vars, a, b, c);
    Cscript_New(f_obj_vars, model, a, b);
    return Cscript_New(f_vars, a, b);
}
