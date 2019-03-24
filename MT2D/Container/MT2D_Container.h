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

#define BYTE unsigned char

typedef struct {
	BYTE **Data;
	char **Names;
	unsigned int *Length; //in bytes
	unsigned char *Xpadding;//the number of bytes that were added to make this file to be multiple of X
	int Files;

	/**/
	unsigned char **FilePath;
	unsigned int FilePtrCount;
	unsigned int *DataOffset;
	unsigned int *DataFileId;
}MT2D_Container;

/**
Return the index of the first file found with the given name.
-If failed the function returns -1
**/
int MT2D_Container_Get_FileId(char *Name);

int MT2D_Container_Get_FileLength(int Id);
int MT2D_Container_Get_FilePadding(int Id);
BYTE *MT2D_Container_Get_FileData(int Id);
/*Return the pointer of the name of a file form the given ID*/
char *MT2D_Container_Get_FileName_By_ID(int ID);

void MT2D_Container_Init();

/**
Free the loaded files
**/
void MT2D_Container_Clear();

/**
Can load one or more Containers under the main container struct
**/
bool  MT2D_Container_Load(char *Path);

/**
Save all the loaded files in the container for an external file
**/

/**
Load a normal file under the container
-WARNING: remember to add the '\0' at the end of a raw text file in case you want to use the plain text file from a file
**/
bool MT2D_Container_Load_File(char *Path);

void MT2D_Container_Load_File_From_Memory(char *Name, BYTE *Data, int Length);

bool MT2D_Container_Save(char *NameAndPath);

/**
-Save the loaded file under a specific file.
-Warning: You may get some trouble saving encrypted files
**/
bool MT2D_Container_Save_File(char *Name, char * NewName, char *Path);


/**
Decode a file inside the Container
-Thread Safe
**/
void MT2D_Container_Decode_File(char *Name);

/**
Encode a file inside the Container
-Thread Safe
**/
#define MT2D_Container_Encode_File  MT2D_Container_Decode_File

void MT2D_Container_Password_Init();

#define  MT2D_Container_Password_Reset MT2D_Container_Password_Init

void MT2D_Container_Password_AddKey(unsigned char Key);


int MT2D_Container_Count_Files();



#endif