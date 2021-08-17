/**
	MT2D_Container, created by: Lucas Zimerman Fraulob

	this function is used to represent a struct that's able to store any given type of file data under a container.
	It also support encryption for protecting the data that you wish to protect.
	You can encrypt file by file using different passwords and also not to encrypt others.
	You can also export the container itself.

	Since there's only one global container you should avoid using multiple files with the same type of name.

	How to get access to the loaded files?
	-use the function "MT2D_Container_Get_FileId", it'll return for you the index where the data is stored.


	The struct of a Container file:
Line 1: name of the file 1 with 1 bit shift to the right, no char limit
Line 2: 8 bits of padding + the rest representing the file length (length is inverted)
Line 3,...X: Data
Line X+1: name of the file 2 with 1 bit shift to the right, no char limit
Line X+2: 8 bits of padding + the rest representing the file length (length is inverted)
Line X+3,...,Y: Data
...

**/

#define MT2D_USING_CONTAINER

#ifndef MT2D_USE_CONTAINER
#define MT2D_USE_CONTAINER

#include <stdio.h>
#include <stdlib.h>
#include "MT2D_ContainerFile.h"
#include "MT2D_ContainerFilePath.h"

#define BYTE unsigned char

#pragma region  Container Struct

enum _DataStore_Type
{
	MT2D_CONT_DATATYPE_MEM = -1,
	MT2D_CONT_DATATYPE_CONT = 0,
	MT2D_CONT_DATATYPE_FILE
};



typedef struct MT2D_ContainerFilePath {
	char* file;//if refCount is -1 it points to a memory address
	int refCount;// if -1 it represents a memory data and not a container
	void* firstData;
	MT2D_ContainerFilePath* prev;
	MT2D_ContainerFilePath* next;
};

typedef struct MT2D_ContainerFile {
	/*minimal struct which represents the a reference from the file to be stored*/
	char* name;
	unsigned int length; //in bytes
	unsigned char xpadding;//the number of bytes that were added to make this file to be multiple of X
	unsigned int fileDataOffset;//coordinate where the data is located in the file, not counting where the name of the file is located at
	MT2D_ContainerFilePath* DataFileId;
	MT2D_ContainerFile* next;
};

typedef struct{ 
	MT2D_ContainerFilePath *start;
	MT2D_ContainerFilePath *end;
}MT2D_ContainerList;

typedef struct{
	MT2D_ContainerFile **hash[15];
	int count[15];
}MT2D_ContainerHash;

#pragma endregion Container Struct 

#pragma region Container User Struct
typedef struct ContainerData
{
	unsigned char* Data;
	int Length;
	int Id;
};
#pragma endregion Container User Struct


int MT2D_Container_Get_FileId(char *name);
int MT2D_Container_Get_FileLength(int id);
int MT2D_Container_Get_FilePadding(int id);
MT2D_Container_DataStore_Type MT2D_GetFile_StoreType(int id);
/*
	Update the container disk file withe the new data.
	WARNING: this operation may take a long time if you have alot of loaded files
	data is not cloned but copied the reference
*/
bool MT2D_Container_Update(int id, BYTE * data, int length, bool encrypt);
/*
 * Delete the referenced id from the disc or memory
*/
void MT2D_Container_Delete_Data(int id);

void MT2D_Container_Init();
/**
Free the loaded files
**/
void MT2D_Container_Clear();
/**
Can load one or more Containers under the main container struct
**/
bool MT2D_Container_Load(char *Path);
/**
Load a normal file under the container | Path char data cloned and not reused.
-the data will be cloned inside of the memory and it'll not support encryption.
-WARNING: remember to add the '\0' at the end of a raw text file in case you want to use the plain text file from a file
**/
bool MT2D_Container_Load_File(char *Path);

void MT2D_Container_Load_File_From_Memory(char *Name, BYTE *Data, int Length);
/**
-Export the loaded data.
-Name: name of the loaded data
-FileName: name of the file
-Path: Where it's going to save the data
-Decrypt: if the file needs to be decrypted
**/
bool MT2D_Container_Export_as_File(int index, char * NewName, char *Path, bool decrypt);
/**
 * Copy the data loaded in MT2D_Container into an existing container.
 * -containerName: the valid name of the container file
 * -dataId: the id to be copied
 * -encrypt: if the copied file is going to be encrypted (mark as false if the copied file is already encrypted)
 * return false if the container wasn't found
 **/
bool MT2D_Container_Export_into_Container(int dataId, char *containerName, bool encrypt);
void MT2D_Container_Password_Init();
void MT2D_Container_Password_AddKey(unsigned char Key);
int MT2D_Container_Count_Files();
/*Return a new filename string*/
char *MT2D_Container_Get_FileName_By_ID(int id);
/*return a new array of bytes containing the decrypted data. return 0 in case of not found*/
BYTE * MT2D_Container_Get_Data(int id, bool decrypt);


#endif
