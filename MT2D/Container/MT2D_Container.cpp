/*TODO: Refactory plan:
	*Do not load ANY DATA from the file itself, only the padding information, length, the name of the file and also the file pointer where it's located.
	*Create a container struct that's dynamic.
	*Create a container struct that requires a low amount of ram to manage.
	*Create a container model which requires low amount of cpu\ram for dealing with large amount of data pointers.


	PLAN: 
	-Create a hash of 15 Containers
	-The file id is 16 bits min size

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MT2D_Container.h"
#include "../File/MT2D_File.h"
#include "aes.h"

#define xb10000000 128
#define xb00001111 15
#define xb00000001 01


/*REFACTORY VARS*/
#pragma region  Container Struct

typedef struct {
	char *file;
	unsigned int refCount;
	MT2D_ContainerFilePath *prev;
	MT2D_ContainerFilePath *next;
}MT2D_ContainerFilePath;

typedef struct {
	/*minimal struct whitch represents the a reference from the file to be stored*/
	char *name;
	unsigned int length; //in bytes
	unsigned char xpadding;//the number of bytes that were added to make this file to be multiple of X
	unsigned int offset;
	MT2D_ContainerFilePath *DataFileId;
}MT2D_ContainerFile;




typedef struct{ 
	MT2D_ContainerFilePath *start;
	MT2D_ContainerFilePath *end;
}MT2D_ContainerList;

typedef struct{
	MT2D_ContainerFile *hash[15];
	int count[15];
}MT2D_ContainerHash;

#pragma endregion Container Struct 

#pragma region Container Global Vars

MT2D_ContainerHash ContainerHash;
MT2D_ContainerList ListFilePaths;
BYTE MT2D_ContainerKey[16];
int MT2D_ContainerKeyOffset = 0;
BYTE iv[16]; /* { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };*/

#pragma endregion Container Global Vars

/*Refactored*/
void remove_Node(MT2D_ContainerFilePath *node){
		/*Edit the ListFilePaths in case this node is the first or the last node*/
	if(node->prev != NULL){
		node->prev->next = node->next;
	}
	else{
		ListFilePaths.start = node->next;
	}
	if(node->next != NULL){
		node->next->prev = node->prev;
	}
	else{
		ListFilePaths.end = node->prev;
	}
	free(node->file);
	free(node);
}

/*Refactored*/
void add_Node(MT2D_ContainerFilePath *node){
	/*Always add the node to the end*/
	if(ListFilePaths.start == NULL){
		ListFilePaths.start = node;
		ListFilePaths.end = node;		
	}
	else if(ListFilePaths.start == ListFilePaths.end){
		ListFilePaths.end = node;
		node->prev = ListFilePaths.start;
		ListFilePaths.start->next = node;
	}
	else{
		ListFilePaths.end->next = node;
		node->prev = ListFilePaths.end;
		ListFilePaths.end = node;
	}
}

/*Refactored*/
int get_Hash_Index(int id){
	return id & xb00001111;
}

/*Refactored*/
int get_Hash_File_Index(int id){
	return id >> 4;
}

/**
 *  Refactored
 * 	Return the index of the container file found with the given name.
	-If failed the function returns -1
**/
int MT2D_Container_Get_FileId(char *name) {
	int hashIndex = name[0] & xb00001111;
	int j;
	for (int i = 0; i < ContainerHash.count[hashIndex]; i++) {
		j = 0;
		/*hand made strcmp*/
		while ( name[j] != '\0' && 
				ContainerHash.hash[hashIndex][i].name[j] != '\0' && 
				name[j] == ContainerHash.hash[hashIndex][i].name[j]
			  ) {
			j++;
		}
		if (name[j] == '\0') {
			if (ContainerHash.hash[hashIndex][i].name[j] == '\0') {
				//found the file
				return (i << 4) | hashIndex;
			}
		}
	}
	return -1;
}

/*Refactored*/
int MT2D_Container_Get_FileLength(int id) {
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if(ContainerHash.count[hashIndex] < offset)
	{
		return ContainerHash.hash[hashIndex][offset].length;
	}
	return -1;
}

/*Refactored*/
int MT2D_Container_Get_FilePadding(int id) {
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if(ContainerHash.count[hashIndex] < offset)
	{
		return ContainerHash.hash[hashIndex][offset].xpadding;
	}
	return -1;
}

bool MT2D_Container_Update(BYTE * data, bool encrypt){
	/*TODO: Implement*/
	return false;
}

/*Refactored*/
void MT2D_Container_Init() {
	
	int i=0;	
	while(i < 15){
		ContainerHash.hash[i] = 0;
		ContainerHash.count[i] = 0;
		iv[i] = 0xf0 + i;
		i++;
	}
	iv[i] = 0xf0 + i;
	ListFilePaths.start = 0;
	ListFilePaths.end = 0;

}

/**
 * Refactored
Free the loaded files
**/
void MT2D_Container_Clear() {
	MT2D_ContainerFilePath *filePointer;
	int i,j;
	//Clear the hash
	for(i = 0; i < 15; i++){
		for(j = 0; j < ContainerHash.count[i]; j++){
			free(ContainerHash.hash[i][j].name);
		}
		free(ContainerHash.hash[i]);
		ContainerHash.count[i] = 0;
	}
	//Clear the files
    filePointer = ListFilePaths.start;
	while(filePointer != NULL){
		free(filePointer->file);
		ListFilePaths.start = filePointer->next;
		free(filePointer);
		filePointer = ListFilePaths.start;
	}
	ListFilePaths.end = 0;
	ListFilePaths.start = 0;
}

/**
	-Name is not cloned here, we reuse the pointer that u sent to us, so look out if you're going to use that pointer later.
	-Note: this function doesn't close the MT2D_FILE
	Refactored
**/
void container_LoadBuffer(MT2D_ContainerFilePath  *file, int length, int padding, int offset, char *name) {
	int hash = get_Hash_Index(name[0]);
	int count = ContainerHash.count[hash]; 
	if (count == 0) {
		ContainerHash.hash[hash] = (MT2D_ContainerFile*)malloc(sizeof(MT2D_ContainerFile));
	}
	else
	{
		ContainerHash.hash[hash] = (MT2D_ContainerFile*)realloc(ContainerHash.hash[hash],(count + 1)*sizeof(MT2D_ContainerFile));
	}
	ContainerHash.hash[hash][count].name = name;
	ContainerHash.hash[hash][count].length = length;
	ContainerHash.hash[hash][count].xpadding = padding;
	ContainerHash.hash[hash][count].offset = offset;
	ContainerHash.hash[hash][count].DataFileId = file;
	ContainerHash.count[hash]++;
}


/**
Can load one or more Containers under the main container struct
**/
bool MT2D_Container_Load(char *Path) {
	/*TODO: Refactor*/
	bool Loaded = false;
	unsigned char fbuffer=0;
	unsigned char *HiddenFileName;
	unsigned int  FileLength;
	unsigned char BitPrevious, BitActual;
	unsigned char Padding;
	unsigned char *SmartPointer;
	int i = 0, k;
	int Len;
	int j;
	MT2D_FILE *Destination = MT2D_FILE_OPEN(Path, "rb");
	if (Destination) {
		while (!MT2D_FILE_EOF(Destination)) {
			//PART 1: READ THE NAME
			Len = 0;
			HiddenFileName = (unsigned  char*)malloc(sizeof(unsigned  char));
			fbuffer = 0;
			//read line
			while (fbuffer != '\n' && !MT2D_FILE_EOF(Destination)) {
				fbuffer = MT2D_FILE_READ_BYTE(Destination);
				if (fbuffer != '\n') {
					HiddenFileName = (unsigned  char*)realloc(HiddenFileName,(Len+1) * sizeof(unsigned  char));
					HiddenFileName[Len] = fbuffer;
					Len++;
				}
				HiddenFileName = (unsigned  char*)realloc(HiddenFileName, (Len + 1) * sizeof(unsigned  char));
				HiddenFileName[Len] = '\0';
			}
			if (!MT2D_FILE_EOF(Destination)) {
				//PART 1: DECODE THE NAME
				j = Len - 1;
				BitPrevious = 0;// garbage's going to be inserted on the first loop and gets fixed after the loop
				while (j >= 0) {
					BitActual = HiddenFileName[j] & xb10000000; //get the first bit from the byte
					HiddenFileName[j] = HiddenFileName[j] << 1;
					HiddenFileName[j] = HiddenFileName[j] | (BitPrevious >> 7);
					BitPrevious = BitActual;
					j--;
				}
				HiddenFileName[Len - 1] = HiddenFileName[Len - 1] | (BitPrevious >> 7);
				//PART 2: GET THE PADDING AND LENGTH FROM THE DATA
				//PART 2.1:GET THE PADDING
				Padding = MT2D_FILE_READ_BYTE(Destination);
				//PART 2.2:GET THE LENGTH AND DECODE IT
				FileLength = 0;
				fbuffer = 0;
				SmartPointer = (unsigned char*)&FileLength;//get the Length pointer in form of a byte
				j = 0;
				while (fbuffer != '\n' && !MT2D_FILE_EOF(Destination)) {
					fbuffer = MT2D_FILE_READ_BYTE(Destination);
					if (fbuffer != '\n') {
						SmartPointer[j] = fbuffer;
						Len++;
						j++;
					}
				}
				//PART 3: LOAD THE DATA
				container_LoadBuffer(Destination, FileLength, Padding, (char*)HiddenFileName);
				MT2D_FILE_READ_BYTE(Destination);//Jump the \n
			}
			else {
				free(HiddenFileName);
			}
		}
		Loaded = true;
		MT2D_FILE_CLOSE(Destination);
	}
	return Loaded;
}

/**
Load a normal file under the container | Path char data cloned and not reused
-WARNING: remember to add the '\0' at the end of a raw text file in case you want to use the plain text file from a file
**/
bool MT2D_Container_Load_File(char *Path) {
		/*TODO: Refactor*/

	bool loaded = false;
	MT2D_FILE *file = MT2D_FILE_OPEN(Path, "rb");
	if (file) {

		loaded = true;
	}
	int length = strlen(Path) +1;
	char *newString = (char*)malloc(length * sizeof(char));
	strcpy(newString, Path);

	//get the size from the data
	MT2D_FILE_SEEK(file, 0L, SEEK_END);
	length = MT2D_FILE_TELL(file);
	MT2D_FILE_SEEK(file, 0L, SEEK_SET);
	container_LoadBuffer(file, length, 0, newString);
	MT2D_FILE_CLOSE(file);
	return loaded;
}

void MT2D_Container_Load_File_From_Memory(char *Name, BYTE *Data, int Length) {
		/*TODO: Refactor*/

	if (MT2D_GlobalContainer.Files == 0) {
		MT2D_GlobalContainer.Data = (BYTE**)malloc(sizeof(BYTE*));
		MT2D_GlobalContainer.Length = (unsigned int*)malloc(sizeof(unsigned int));
		MT2D_GlobalContainer.Names = (char**)malloc(sizeof(char*));
		MT2D_GlobalContainer.Xpadding = (unsigned char*)malloc(sizeof(unsigned char));
	}
	else
	{
		MT2D_GlobalContainer.Files++;
		MT2D_GlobalContainer.Data = (BYTE**)realloc(MT2D_GlobalContainer.Data, MT2D_GlobalContainer.Files * sizeof(BYTE*));
		MT2D_GlobalContainer.Length = (unsigned int*)realloc(MT2D_GlobalContainer.Length, MT2D_GlobalContainer.Files * sizeof(unsigned int));
		MT2D_GlobalContainer.Names = (char**)realloc(MT2D_GlobalContainer.Names, MT2D_GlobalContainer.Files * sizeof(char*));
		MT2D_GlobalContainer.Xpadding = (unsigned char*)realloc(MT2D_GlobalContainer.Xpadding, MT2D_GlobalContainer.Files * sizeof(unsigned char));
		MT2D_GlobalContainer.Files--;
	}
	MT2D_GlobalContainer.Names[MT2D_GlobalContainer.Files] = Name;
	MT2D_GlobalContainer.Length[MT2D_GlobalContainer.Files] = Length;
	MT2D_GlobalContainer.Xpadding[MT2D_GlobalContainer.Files] = 0;
	MT2D_GlobalContainer.Data[MT2D_GlobalContainer.Files] = Data;
	MT2D_GlobalContainer.Files++;
}
/**
Save all the loaded files in the container for an external file
**/
bool MT2D_Container_Save(char *NameAndPath) {
		/*TODO: Refactor*/

	bool Saved = false;
	unsigned char *HiddenFileName;
	unsigned char FileLength;
	unsigned char BitPrevious,BitActual;
	int i = 0,k;
	int Len;
	int j;
	MT2D_FILE *Destination = MT2D_FILE_OPEN(NameAndPath, "wb");
	if (Destination) {
		while (i < MT2D_GlobalContainer.Files) {
			//PART 1: SAVE THE NAME
			Len = strlen(MT2D_GlobalContainer.Names[i]);			
			HiddenFileName = (unsigned char*)malloc((Len+1) * sizeof(unsigned char));
			strcpy((char*)HiddenFileName, MT2D_GlobalContainer.Names[i]);
			j = 0;
			BitPrevious = HiddenFileName[j] & (1<<7);
			while (j < Len){ 
				BitActual = HiddenFileName[j] & 1;
				HiddenFileName[j] = HiddenFileName[j] >> 1;
				HiddenFileName[j] = HiddenFileName[j] | BitPrevious << 7;
				BitPrevious = BitActual;
				j++;
			}
			HiddenFileName[0] = HiddenFileName[0] | BitPrevious << 7;
			j = 0;
			while (j < Len) {
				MT2D_FILE_WRITE_BYTE(Destination, HiddenFileName[j]);
				j++;
			}
			free(HiddenFileName);
			MT2D_FILE_WRITE_BYTE(Destination, '\n');
			//PART 2: SAVE THE OFFSET AND LENGTH FROM THE DATA
			j = MT2D_GlobalContainer.Length[i];
			MT2D_FILE_WRITE_BYTE(Destination, MT2D_GlobalContainer.Xpadding[i]);
			FileLength = 0;
			Len++;
			k = 0;
			while (j > 0) {
				if (k == 8) {	//FileLength is full, put it under the file and clear it on the memory
					Len++;
					MT2D_FILE_WRITE_BYTE(Destination, FileLength);
					FileLength = 0;
					k = 0;
				}
				FileLength = FileLength | ( (j & 1) << (k)); //add bit a bit into FileLength
				j = j >> 1;
				k++;
			}
			MT2D_FILE_WRITE_BYTE(Destination, FileLength);//it'll not hurt if FileLength is zero at the end because the software will ignore it
			MT2D_FILE_WRITE_BYTE(Destination, '\n');
			//PART 3: SAVE THE DATA
			MT2D_FILE_WRITE(Destination, MT2D_GlobalContainer.Data[i], MT2D_GlobalContainer.Length[i] + MT2D_GlobalContainer.Xpadding[i] , 1);
			MT2D_FILE_WRITE_BYTE(Destination,'\n');
			i++;
		}
		Saved = true;
		MT2D_FILE_CLOSE(Destination);

	}
	return Saved;
}

/**
-Save the loaded file under a specific file.
-Warning: You may get some trouble saving encrypted files
**/
bool MT2D_Container_Save_File(char *Name, char * NewName, char *Path) {
		/*TODO: Refactor*/

	int Index = MT2D_Container_Get_FileId(Name);
	int OffsetSaved;
	char *Filename;
	bool Saved = false;
	Filename = (char*)malloc(strlen(NewName) + strlen(Path) + 1);
	sprintf(Filename, "%s%s\0",Path, NewName);
	MT2D_FILE *Destination = MT2D_FILE_OPEN(Filename, "wb");
	if (Destination) {
		free(Filename);
		//PART 3: SAVE THE DATA
		MT2D_FILE_WRITE(Destination,MT2D_GlobalContainer.Data[Index], MT2D_GlobalContainer.Length[Index] + MT2D_GlobalContainer.Xpadding[Index], 1);
		Saved - true;
		MT2D_FILE_CLOSE(Destination);
	}
	return Saved;
}


/**
Encode/Decode a file inside the Container
TO BE REMOVED
**/
void MT2D_Container_Decode_File(char *Name) {
		/*TODO: Refactor*/

	int Index = MT2D_Container_Get_FileId(Name);
	int Length = MT2D_GlobalContainer.Length[Index];
	if (Length > 0) {
		struct AES_ctx ctx;
		AES_init_ctx_iv(&ctx, MT2D_ContainerKey, iv);
		if (Length % 16 == 0) {
			//It doesn't matter in this case if we're  encoding\decoding
			AES_CTR_xcrypt_buffer(&ctx, MT2D_GlobalContainer.Data[Index], Length);
		}
		else {
			if (MT2D_GlobalContainer.Xpadding[Index] > 0) {
				//The file is encoded so we're going to encode
				AES_CTR_xcrypt_buffer(&ctx, MT2D_GlobalContainer.Data[Index], Length);
				//remove the padding memory
				MT2D_GlobalContainer.Data[Index] = (BYTE*)realloc(MT2D_GlobalContainer.Data[Index], (Length - MT2D_GlobalContainer.Xpadding[Index] + MT2D_GlobalContainer.Xpadding[Index] ) * sizeof(BYTE));
				//cancel the padding
				MT2D_GlobalContainer.Xpadding[Index] = 0;
			}
			else
			{
				//the file is decoded so we're going to encode
				MT2D_GlobalContainer.Xpadding[Index] = 16 - (Length % 16);
				int i = 1;

				MT2D_GlobalContainer.Data[Index] = (BYTE*)realloc(MT2D_GlobalContainer.Data[Index], (MT2D_GlobalContainer.Length[Index] + MT2D_GlobalContainer.Xpadding[Index] +1) * sizeof(BYTE));
				while (i <= MT2D_GlobalContainer.Xpadding[Index]) {
					MT2D_GlobalContainer.Data[Index][Length + i] = (i % 2 == 0 ? 255 : 0);
					i++;
				}
				AES_CTR_xcrypt_buffer(&ctx, MT2D_GlobalContainer.Data[Index], Length + MT2D_GlobalContainer.Xpadding[Index]);
			}
		}
	}
}

/*Refactored*/
void MT2D_Container_Password_Init() {
	int i = 0;
	while (i < 16) {
		MT2D_ContainerKey[i] = (i % 2 == 0 ? 0 : 255);
		i++;
	}
	MT2D_ContainerKeyOffset = 0;
}

/*Refactored*/
void MT2D_Container_Password_AddKey(unsigned char Key) {
	MT2D_ContainerKey[MT2D_ContainerKeyOffset] = Key;
	MT2D_ContainerKeyOffset++;
	MT2D_ContainerKeyOffset = MT2D_ContainerKeyOffset % 16;
}


/*Refactored*/
int MT2D_Container_Count_Files() {
	int i,ret;
	for(i =0, ret =0; i< 15; i++)
		ret += ContainerHash.count[i];
	return ret;
}


/*Return the pointer of the name of a file form the given ID*/
/*Refactored*/
char *MT2D_Container_Get_FileName_By_ID(int id) {
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if (offset < ContainerHash.count[hashIndex]) {
		/*TODO: return a copy not a pointer*/
		return ContainerHash.hash[hashIndex][offset].name;
	}
	return NULL;
}

/*return a new array of bytes containing the decrypted data. return 0 in case of not found*/
BYTE * MT2D_Container_Get_Data(int ID, bool needDecryption) {
	/*TODO: FINISh*/
	BYTE *buffer = 0;
	if (ID < MT2D_GlobalContainer.Files){
		MT2D_FILE *file = MT2D_FILE_OPEN((const char*)MT2D_GlobalContainer.FilePath[MT2D_GlobalContainer.DataFileId[ID]], "rb");
		MT2D_FILE_SEEK(file, MT2D_GlobalContainer.DataOffset[ID], SEEK_SET);
		/*
			*Load all the required bytes;
			*if decrypt required
				*return decrypted bytes
			*return bytes;
		*/
	}
	return buffer;
}







