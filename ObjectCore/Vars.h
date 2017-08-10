#ifndef VAR_HEADER
#define VAR_HEADER



enum MT2D_VAR_TYPE
{
	VAR_BOOL,
	VAR_CHAR,
	VAR_INT,
	VAR_FLOAT,
	VAR_UNSIGNED_CHAR,
	VAR_UNSIGNED_INT,
	VAR_UNSIGNED_FLOAT,
	VAR_POINTER,
	VAR_STRING
};

enum MT2D_VAR_ERROR
{
	 Var_No_Error,
	 Var_Wrong_Type, //set int into char var
	 Var_Divide_By_Zero,
	 Var_Not_Found,
	 Var_Type_Not_Supported, // check MT2D_VAR_TYPE for supported types.
	 Var_Null_Data,
	 Var_Null_Name,
	 Var_Out_Of_Memory_To_Create_New_Var
};

extern MT2D_VAR_ERROR MT2D_Var_Last_Error;


struct MT2D_VAR {
	void *Data;
	MT2D_VAR_TYPE Type;
	char *Name;
};


void MT2D_Object_Set_Var(MT2D_VAR *Var, bool Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, int Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, unsigned int Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, char *Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, float Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, float Data);
void MT2D_Object_Set_Var(MT2D_VAR *Var, void *Data, MT2D_VAR_TYPE Type);

void *MT2D_Object_Get_Var_Data(MT2D_VAR *Var);

MT2D_VAR *MT2D_Object_Get_Var_Pointer(MT2D_VAR *ArrayVARS, char *Name);

MT2D_VAR_ERROR MT2D_Object_Get_Last_Var_Error();

MT2D_VAR * MT2D_Object_Create_Var(char * Name, MT2D_VAR_TYPE Type, void * InitialData);
MT2D_VAR *MT2D_Object_Create_Var_Int(char *Name, int InitialData);
MT2D_VAR *MT2D_Object_Create_Var_UInt(char *Name, unsigned int InitialData);
MT2D_VAR * MT2D_Object_Create_Var_Char(char * Name, char InitialData);
MT2D_VAR * MT2D_Object_Create_Var_UChar(char * Name, unsigned char InitialData);
MT2D_VAR * MT2D_Object_Create_Var_Pointer(char * Name);

int MT2D_Object_VAR_GetInt(MT2D_VAR *Var);
MT2D_VAR *MT2D_VAR_CLONE(MT2D_VAR *VAR);


void  MT2D_Object_SUB(MT2D_VAR *Store, MT2D_VAR *ToAdd);
void  MT2D_Object_ADD(MT2D_VAR *Store, MT2D_VAR *ToAdd);

#endif