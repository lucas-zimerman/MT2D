#include <MT2D\ObjectCore\Functions.h>
#include <cstdlib>

MT2D_OBJECT_FUNCTION *Object_Function_Create_Empty()
{
	MT2D_OBJECT_FUNCTION *F = (MT2D_OBJECT_FUNCTION*)malloc(sizeof(MT2D_OBJECT_FUNCTION));
	F->vPtr1 = 0;
	F->vPtr2 = 0;
	F->vPtr3 = 0;
	F->vPtr4 = 0;
	F->vPtr5 = 0;
	F->vPtr6 = 0;
	F->ReturnVAR_ID = 0;
	F->TotalVars = 0;
	F->TempVars = 0;
	return F;
}

MT2D_OBJECT_FUNCTION *Object_Function_Create_New1(void (*Function)(Object *)) {
	MT2D_OBJECT_FUNCTION *F = Object_Function_Create_Empty();
	F->vPtr1 = Function;
	return F;
}

MT2D_OBJECT_FUNCTION *Object_Function_Create_New2(char *ReturnVAR_ID,void(*Function)(char *, Object *)) {
	MT2D_OBJECT_FUNCTION *F = Object_Function_Create_Empty();
	F->vPtr2 = Function;
	F->ReturnVAR_ID = ReturnVAR_ID;
	return F;

}

MT2D_OBJECT_FUNCTION *Object_Function_Create_New3(MT2D_VAR **ObjVarPointer,void(*Function)(Object *, MT2D_VAR **)) {
	MT2D_OBJECT_FUNCTION *F = Object_Function_Create_Empty();
	F->vPtr3 = Function;
	F->TempVars = ObjVarPointer;
	F->TotalVars = 1;
	return F;
}

MT2D_OBJECT_FUNCTION *Object_Function_Create_New4(MT2D_VAR **ObjVarPointers,int TotalPointers,void(*Function)(Object *, MT2D_VAR **, int )) {
	MT2D_OBJECT_FUNCTION *F = Object_Function_Create_Empty();
	F->vPtr4 = Function;
	F->TempVars = ObjVarPointers;
	F->TotalVars = TotalPointers;
	return F;

}

MT2D_OBJECT_FUNCTION *Object_Function_Create_New5(char *ReturnVAR_ID, MT2D_VAR **ObjVarPointers, int TotalPointers,void(*Function)(char *, Object *, MT2D_VAR **, int)) {
	MT2D_OBJECT_FUNCTION *F = Object_Function_Create_Empty();
	F->vPtr5 = Function;
	F->ReturnVAR_ID = ReturnVAR_ID;
	F->TempVars = ObjVarPointers;
	F->TotalVars = TotalPointers;
	return F;
}

MT2D_OBJECT_FUNCTION *Object_Function_Create_New6(Object *ModelObj, MT2D_VAR **ObjVarPointers, void(*Function)(Object *, Object *, MT2D_VAR **)) {
	MT2D_OBJECT_FUNCTION *F = Object_Function_Create_Empty();
	F->vPtr6 = Function;
	F->TempObj = ModelObj;
	F->TempVars = ObjVarPointers;
	return F;
}



void Object_Function_Call(Object *Caller, MT2D_OBJECT_FUNCTION *Function) {
	if (Function) {
		if (Caller) {
			if (Function->vPtr1) {
				Function->vPtr1(Caller);
			}
			else if (Function->vPtr2) {
				Function->vPtr2(Function->ReturnVAR_ID, Caller);
			}
			else if (Function->vPtr3) {
				Function->vPtr3(Caller,Function->TempVars);
			}
			else if (Function->vPtr4) {
				Function->vPtr4(Caller, Function->TempVars,Function->TotalVars);
			}
			else if (Function->vPtr5) {
				Function->vPtr5(Function->ReturnVAR_ID, Caller, Function->TempVars,Function->TotalVars);
			}
			else if (Function->vPtr6) {
				Function->vPtr6(Caller,Function->TempObj, Function->TempVars);
			}

		}
	}
}