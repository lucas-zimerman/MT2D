// Vars.cpp, created by: Lucas Zimerman Fraulob
// This file contains the basic functions to interact with a Var struct.
//
// How to set data into the var?
// A:
// int X = 5;
// -MT2D_Object_Set_Var(Var, X);
// -MT2D_Object_Set_Var(Var, 5);
// -MT2D_Object_Set_Var(Var, "Dog");
// -MT2D_Object_Set_Var(Var, 1.01);
// int *X2 = (int*)malloc(sizeof(int));
// X2[0] = 6;
// -MT2D_Object_Set_Var(Var, X2[0]); or MT2D_Object_Set_Var(Var, (void*)X2,VAR_INT);
//
// How to get data from a var?
//
// int X;
// X = *(int*)MT2D_Object_Get_Var_Data(var);
//
// on a char* case, you just need to call MT2D_Object_Get_Var_Data(var); without the *(char*) part.
// like: 
// printf("%s\n",MT2D_Object_Get_Var_Data(var));
//
// Check the supported var types under the MT2D_VAR_TYPE enum
//
// to know if there were some kind of error while interacting with the var functions
// check the return from the function MT2D_Object_Get_Last_Var_Error, if null, there were no errors
// else, check the MT2D_VAR_ERROR enum for more information.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __ANDROID__
#include <MT2D/Objectcore/Vars.h>
#else
#include "../Objectcore/Vars.h"
#endif


MT2D_VAR_ERROR MT2D_Var_Last_Error = Var_No_Error;


void Private_Free_Data(void *Data,MT2D_VAR_TYPE Type)
{
		if (Type == VAR_BOOL)
	{
		free((bool*)Data);
	}
	else if (Type == VAR_CHAR)
	{
		//free((char*)Var->Data);
	}
	else if (Type == VAR_INT || Type == VAR_UNSIGNED_INT)
	{
		free((int*)Data);
	}
	else if (Type == VAR_FLOAT)
	{
		free((float*)Data);
	}

}


/*
	Should only be called by Set and Delete calls from this file.
*/
void Private_Free_Var_Data(MT2D_VAR *Var)
{
	Private_Free_Data(Var->Data,Var->Type);
	Var->Data = NULL;
}


void MT2D_Object_Set_Var(MT2D_VAR *Var, bool Data)
{
	bool *ptr = (bool*)malloc(sizeof(bool));
	ptr[0] = Data;
	//MT2D_Object_Set_Var(Var, (void*)ptr);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, int Data)
{
	int *ptr = (int*)malloc(sizeof(int));
	ptr[0] = Data;
	//MT2D_Object_Set_Var(Var, (void*)ptr,VAR_INT);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, unsigned int Data)
{
	unsigned int *ptr = (unsigned int*)malloc(sizeof(unsigned int));
	ptr[0] = Data;
	//MT2D_Object_Set_Var(Var, (void*)ptr,VAR_UNSIGNED_INT);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, char *Data)
{
//	char *Copy = (char*)malloc(strlen(Data)*sizeof(char));
//	strcpy(Copy,Data);
	//MT2D_Object_Set_Var(Var, (void*)Data,VAR_CHAR);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, float Data)
{
	float *ptr = (float*)malloc(sizeof(float));
	ptr[0] = Data;
	MT2D_Object_Set_Var(Var, (void*)ptr,VAR_FLOAT);
}

/*
	Generic function used by the others
*/
void MT2D_Object_Set_Var(MT2D_VAR *Var, void * Data, MT2D_VAR_TYPE Type)
{
	MT2D_Var_Last_Error = Var_No_Error;
	if (Var)
	{
		if (Var->Type == Type)
		{
			Private_Free_Var_Data(Var);
			Var->Data = Data;
		}
		else
		{
			MT2D_Var_Last_Error = Var_Wrong_Type;
#ifdef _DEBUG 
			printf("MT2D: WRONG TYPE IN VAR %s\n",Var->Name);
#endif
			Private_Free_Data(Data,Type);
		}
	}
	else
	{
		MT2D_Var_Last_Error = Var_Not_Found;
#ifdef _DEBUG 
		printf("MT2D: NULL VAR\n");
#endif
	}
}

void * MT2D_Object_Get_Var_Data(MT2D_VAR * Var)
{
	MT2D_Var_Last_Error = Var_No_Error;
	if (Var)
	{
		return Var->Data;
	}
	else
	{ 
		MT2D_Var_Last_Error = Var_Not_Found;
#ifdef _DEBUG 
		printf("MT2D: NULL VAR\n");
#endif
	}
	return NULL;
}

MT2D_VAR * MT2D_Object_Get_Var_Pointer(MT2D_VAR * ArrayVARS, char * Name)
{
	int i = 0;
	MT2D_VAR *ptr = NULL;
	MT2D_Var_Last_Error = Var_No_Error;
	if (Name)
	{
		while (ptr == NULL)
		{
			if (strcmp(ArrayVARS[i].Name, Name) == 0)
			{
				ptr = &ArrayVARS[i];
			}
			///yeah, there will be an error here  if the var name isnt on the list.
			//and it's case sensitive, so "aaa" != "AAA"
			i++;
		}
	}
	else
	{
		MT2D_Var_Last_Error = Var_Null_Name;
#ifdef _DEBUG 
		printf("MT2D: NULL NAME RECEIVED IN VAR\n");
#endif
	}

	return ptr;
}

MT2D_VAR_ERROR MT2D_Object_Get_Last_Var_Error()
{
	return MT2D_VAR_ERROR();
}

MT2D_VAR * MT2D_Object_Create_Var_Int(char * Name, int InitialData)
{
	int *Data = (int*)malloc(sizeof(int));
	Data[0] = InitialData;
	MT2D_VAR *out =  MT2D_Object_Create_Var(Name, VAR_INT, (void*)Data);
	free(Data);
	return out;
}
MT2D_VAR * MT2D_Object_Create_Var_UInt(char * Name, unsigned int InitialData)
{
	unsigned int *Data = (unsigned int*)malloc(sizeof(unsigned int));
	Data[0] = InitialData;
	MT2D_VAR *out = MT2D_Object_Create_Var(Name, VAR_UNSIGNED_INT, (void*)Data);
	free(Data);
	return out;
}

MT2D_VAR * MT2D_Object_Create_Var_Char(char * Name, char InitialData)
{
	char *Data = (char*)malloc(sizeof(char));
	Data[0] = InitialData;
	MT2D_VAR *out = MT2D_Object_Create_Var(Name, VAR_CHAR, (void*)Data);
	free(Data);
	return out;
}
MT2D_VAR * MT2D_Object_Create_Var_UChar(char * Name, unsigned char InitialData)
{
	unsigned char *Data = (unsigned char*)malloc(sizeof(unsigned char));
	Data[0] = InitialData;
	MT2D_VAR *out = MT2D_Object_Create_Var(Name, VAR_UNSIGNED_CHAR, (void*)Data);
	free(Data);
	return out;
}

MT2D_VAR * MT2D_Object_Create_Var_Pointer(char * Name)
{
	//the only important data is the Name
	//Check ObjectCore_Get_Object_Internal_Data for looking to what Names are allowed to use.
	unsigned char *Data = (unsigned char*)malloc(sizeof(unsigned char));
	Data[0] = 0;
	MT2D_VAR *out = MT2D_Object_Create_Var(Name, VAR_POINTER, (void*)Data);
	free(Data);
	return out;
}

/*
	Generic Function
*/
MT2D_VAR * MT2D_Object_Create_Var(char * Name, MT2D_VAR_TYPE Type, void * InitialData)
{
	MT2D_VAR *V = (MT2D_VAR*)malloc(sizeof(MT2D_VAR));
	if (V)
	{
		if (InitialData == NULL)
		{
			MT2D_Var_Last_Error = Var_Null_Data;
#ifdef _DEBUG 
			printf("MT2D: NULL DATA RECEIVED IN VAR\n");
#endif
		}
		else
		{
			V->Name = Name;
			if (Name == NULL)
			{
				MT2D_Var_Last_Error = Var_Null_Name;
#ifdef _DEBUG 
				printf("MT2D: NULL NAME RECEIVED IN VAR\n");
#endif
			}
			else
			{
				V->Type = Type;
				if (Type < VAR_BOOL || Type > VAR_STRING)
				{
					MT2D_Var_Last_Error = Var_Type_Not_Supported;
#ifdef _DEBUG 
					printf("MT2D: TYPE NOT SUPPORTED IN VAR %s \n",Name);
#endif
				}
				else
				{
					bool *Nb;
					char *Nc;
					int *Ni;
					float *Nf;

					switch (Type)
					{
					case VAR_BOOL:
						Nb = (bool*)malloc(sizeof(bool));
						Nb[0] = *(bool*)InitialData;
						V->Data = Nb;
						break;
					case VAR_INT:
					case VAR_UNSIGNED_INT:
						Ni = (int*)malloc(sizeof(int));
						Ni[0] = *(int*)InitialData;
						V->Data = Ni;
						break;
					case VAR_FLOAT:
					case VAR_UNSIGNED_FLOAT:
						Nf = (float*)malloc(sizeof(float));
						Nf[0] = *(float*)InitialData;
						V->Data = Nf;
					case VAR_UNSIGNED_CHAR:
					case VAR_CHAR:
						Nc = (char*)malloc(sizeof(char));
						Nc[0] = *(char*)InitialData;
						V->Data = Nc;
						break;
					case VAR_POINTER:
						V->Data = 0;
						break;
					}
				}
			}
		}
	}
	else
	{
		MT2D_Var_Last_Error = Var_Out_Of_Memory_To_Create_New_Var;
#ifdef _DEBUG 
		printf("MT2D: Out of Memory to create a new var\n");
#endif
	}
	return V;
}


/**
	does not change the original object type
**/
void MT2D_Object_VAR_SetVar(MT2D_VAR *Var, MT2D_VAR *New) {
	int output = 0;
	if (Var->Type == VAR_BOOL) {
		switch (New->Type) {
		case VAR_BOOL:
			*(bool*)Var->Data = *(bool*)New->Data;
			break;
		case VAR_CHAR:
			*(bool*)Var->Data = *(char*)New->Data;
			break;
		case VAR_UNSIGNED_CHAR:
			*(bool*)Var->Data = *(unsigned char*)New->Data;
			break;
		case VAR_INT:
			*(bool*)Var->Data = *(int*)New->Data;
			break;
		case VAR_UNSIGNED_INT:
			*(bool*)Var->Data = *(unsigned int*)New->Data;
			break;
		case VAR_FLOAT:
			*(bool*)Var->Data = *(float*)New->Data;
			break;
		}
	}
	else if (Var->Type == VAR_UNSIGNED_CHAR) {
		switch (New->Type) {
		case VAR_BOOL:
			*(unsigned char*)Var->Data = *(bool*)New->Data;
			break;
		case VAR_CHAR:
			*(unsigned char*)Var->Data = *(char*)New->Data;
			break;
		case VAR_UNSIGNED_CHAR:
			*(unsigned char*)Var->Data = *(unsigned char*)New->Data;
			break;
		case VAR_INT:
			*(unsigned char*)Var->Data = *(int*)New->Data;
			break;
		case VAR_UNSIGNED_INT:
			*(unsigned char*)Var->Data = *(unsigned int*)New->Data;
			break;
		case VAR_FLOAT:
			*(unsigned char*)Var->Data = *(float*)New->Data;
			break;
		}
	}
	else if (Var->Type == VAR_CHAR) {
		switch (New->Type) {
		case VAR_BOOL:
			*(char*)Var->Data = *(bool*)New->Data;
			break;
		case VAR_CHAR:
			*(char*)Var->Data = *(char*)New->Data;
			break;
		case VAR_UNSIGNED_CHAR:
			*(char*)Var->Data = *(unsigned char*)New->Data;
			break;
		case VAR_INT:
			*(char*)Var->Data = *(int*)New->Data;
			break;
		case VAR_UNSIGNED_INT:
			*(char*)Var->Data = *(unsigned int*)New->Data;
			break;
		case VAR_FLOAT:
			*(char*)Var->Data = *(float*)New->Data;
			break;
		}
	}
	else if (Var->Type == VAR_INT) {
		switch (New->Type) {
		case VAR_BOOL:
			*(int*)Var->Data = *(bool*)New->Data;
			break;
		case VAR_CHAR:
			*(int*)Var->Data = *(char*)New->Data;
			break;
		case VAR_UNSIGNED_CHAR:
			*(int*)Var->Data = *(unsigned char*)New->Data;
			break;
		case VAR_INT:
			*(int*)Var->Data = *(int*)New->Data;
			break;
		case VAR_UNSIGNED_INT:
			*(int*)Var->Data = *(unsigned int*)New->Data;
			break;
		case VAR_FLOAT:
			*(int*)Var->Data = *(float*)New->Data;
			break;
		}
	}
	else if (Var->Type == VAR_UNSIGNED_INT) {
		switch (New->Type) {
		case VAR_BOOL:
			*(unsigned int*)Var->Data = *(bool*)New->Data;
			break;
		case VAR_CHAR:
			*(unsigned int*)Var->Data = *(char*)New->Data;
			break;
		case VAR_UNSIGNED_CHAR:
			*(unsigned int*)Var->Data = *(unsigned char*)New->Data;
			break;
		case VAR_INT:
			*(unsigned int*)Var->Data = *(int*)New->Data;
			break;
		case VAR_UNSIGNED_INT:
			*(unsigned int*)Var->Data = *(unsigned int*)New->Data;
			break;
		case VAR_FLOAT:
			*(unsigned int*)Var->Data = *(float*)New->Data;
			break;
		}
	}
	else if (Var->Type == VAR_FLOAT) {
		switch (New->Type) {
		case VAR_BOOL:
			*(float*)Var->Data = *(bool*)New->Data;
			break;
		case VAR_CHAR:
			*(float*)Var->Data = *(char*)New->Data;
			break;
		case VAR_UNSIGNED_CHAR:
			*(float*)Var->Data = *(unsigned char*)New->Data;
			break;
		case VAR_INT:
			*(float*)Var->Data = *(int*)New->Data;
			break;
		case VAR_UNSIGNED_INT:
			*(float*)Var->Data = *(unsigned int*)New->Data;
			break;
		case VAR_FLOAT:
			*(float*)Var->Data = *(float*)New->Data;
			break;
		}
	}
	else {
		MT2D_Var_Last_Error = Var_Type_Not_Supported;
	}

}

int MT2D_Object_VAR_GetInt(MT2D_VAR *Var) {
	int output = 0;
	if (Var->Type == VAR_BOOL) {
		if (*(bool*)Var->Data == 1) {
			output = 1;
		}
	}
	else if (Var->Type == VAR_UNSIGNED_CHAR) {
		output = (int)(*(unsigned char*)Var->Data);
	}
	else if (Var->Type == VAR_CHAR) {
		output = (int)(*(char*)Var->Data);
	}
	else if (Var->Type == VAR_INT) {
		output = *(int*)Var->Data;
	}
	else if (Var->Type == VAR_UNSIGNED_INT) {
		output = *(unsigned int*)Var->Data;
	}
	else if (Var->Type == VAR_FLOAT) {
		output = *(float*)Var->Data;

	}
	else {
		MT2D_Var_Last_Error = Var_Type_Not_Supported;
	}
	return output;

}



void  MT2D_Object_ADD(MT2D_VAR *Store, MT2D_VAR *ToAdd) {
	if (Store->Type == VAR_INT) {
		if (ToAdd->Type == VAR_INT) {
			*(int*)Store->Data = *(int*)Store->Data + *(int*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_CHAR) {
			*(int*)Store->Data = *(int*)Store->Data + *(char*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_FLOAT) {
			*(int*)Store->Data = *(int*)Store->Data + *(float*)ToAdd->Data;
		}
	}
	else if (Store->Type == VAR_CHAR) {
		if (ToAdd->Type == VAR_INT) {
			*(char*)Store->Data = *(char*)Store->Data + *(int*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_CHAR) {
			*(char*)Store->Data = *(char*)Store->Data + *(char*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_FLOAT) {
			*(char*)Store->Data = *(char*)Store->Data + *(float*)ToAdd->Data;
		}
	}

}


void  MT2D_Object_SUB(MT2D_VAR *Store, MT2D_VAR *ToAdd) {
	if (Store->Type == VAR_INT) {
		if (ToAdd->Type == VAR_INT) {
			*(int*)Store->Data = *(int*)Store->Data - *(int*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_CHAR) {
			*(int*)Store->Data = *(int*)Store->Data - *(char*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_FLOAT) {
			*(int*)Store->Data = *(int*)Store->Data - *(float*)ToAdd->Data;
		}
	}
	else if (Store->Type == VAR_CHAR) {
		if (ToAdd->Type == VAR_INT) {
			*(char*)Store->Data = *(char*)Store->Data - *(int*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_CHAR) {
			*(char*)Store->Data = *(char*)Store->Data - *(char*)ToAdd->Data;
		}
		else if (ToAdd->Type == VAR_FLOAT) {
			*(char*)Store->Data = *(char*)Store->Data - *(float*)ToAdd->Data;
		}

	}
}

MT2D_VAR *MT2D_VAR_CLONE(MT2D_VAR *VAR) {
	MT2D_VAR *NewVAR =0;
	switch (VAR->Type) {
	case VAR_INT:
		NewVAR = MT2D_Object_Create_Var_Int(VAR->Name, *(int*)VAR->Data);
		break;
	case VAR_UNSIGNED_INT:
		NewVAR = MT2D_Object_Create_Var_UInt(VAR->Name, *(unsigned int*)VAR->Data);
		break;
	case VAR_CHAR:
		NewVAR = MT2D_Object_Create_Var_Char(VAR->Name, *(char*)VAR->Data);
		break;
	case VAR_UNSIGNED_CHAR:
		NewVAR = MT2D_Object_Create_Var_UChar(VAR->Name, *(unsigned char*)VAR->Data);
		break;
	}
	return NewVAR;
}


MT2D_VAR **MT2D_VAR_Create_Matrix1(MT2D_VAR *First) {
	MT2D_VAR **M = (MT2D_VAR**)malloc(sizeof(MT2D_VAR*));
	M[0] = First;
	return M;
}

MT2D_VAR **MT2D_VAR_Create_Matrix2(MT2D_VAR *First, MT2D_VAR *Second) {
	MT2D_VAR **M = (MT2D_VAR**)malloc(2*sizeof(MT2D_VAR*));
	M[0] = First;
	M[1] = Second;
	return M;
}

MT2D_VAR **MT2D_VAR_Create_Matrix3(MT2D_VAR *First, MT2D_VAR *Second, MT2D_VAR *Third) {
	MT2D_VAR **M = (MT2D_VAR**)malloc(3*sizeof(MT2D_VAR*));
	M[0] = First;
	M[1] = Second;
	M[2] = Third;
	return M;
}


MT2D_VAR **MT2D_VAR_Create_Matrix4(MT2D_VAR *First, MT2D_VAR *Second, MT2D_VAR *Third, MT2D_VAR *Fourth) {
	MT2D_VAR **M = (MT2D_VAR**)malloc(4*sizeof(MT2D_VAR*));
	M[0] = First;
	M[1] = Second;
	M[2] = Third;
	M[3] = Fourth;
	return M;
}


void MT2D_VAR_Free(MT2D_VAR *var, int constName) {
	if (var->Type != VAR_POINTER) {
		free(var->Data);
	}
	if (constName == 0) {
		free(var->Name);
	}
	free(var);
}
