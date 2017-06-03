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
#include "Vars.h"

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
	MT2D_Object_Set_Var(Var, (void*)ptr);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, int Data)
{
	int *ptr = (int*)malloc(sizeof(int));
	ptr[0] = Data;
	MT2D_Object_Set_Var(Var, (void*)ptr,VAR_INT);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, unsigned int Data)
{
	unsigned int *ptr = (unsigned int*)malloc(sizeof(unsigned int));
	ptr[0] = Data;
	MT2D_Object_Set_Var(Var, (void*)ptr,VAR_UNSIGNED_INT);
}

void MT2D_Object_Set_Var(MT2D_VAR *Var, char *Data)
{
//	char *Copy = (char*)malloc(strlen(Data)*sizeof(char));
//	strcpy(Copy,Data);
	MT2D_Object_Set_Var(Var, (void*)Data,VAR_CHAR);
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

MT2D_VAR * MT2D_Object_Create_Var(char * Name, MT2D_VAR_TYPE Type, int InitialData)
{
	int *Data = (int*)malloc(sizeof(int));
	Data[0] = InitialData;
	MT2D_VAR *out =  MT2D_Object_Create_Var(Name, Type, (void*)Data);
	free(Data);
	return out;
}
MT2D_VAR * MT2D_Object_Create_Var(char * Name, MT2D_VAR_TYPE Type, unsigned int InitialData)
{
	return MT2D_Object_Create_Var(Name, Type, (int)InitialData);
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
				if (Type < VAR_BOOL || Type > VAR_UNSIGNED_FLOAT)
				{
					MT2D_Var_Last_Error = Var_Type_Not_Supported;
#ifdef _DEBUG 
					printf("MT2D: TYPE NOT SUPPORTED IN VAR %s \n",Name);
#endif
				}
				else
				{
					bool *Nb;
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
					case VAR_CHAR:
						V->Data = InitialData;
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
