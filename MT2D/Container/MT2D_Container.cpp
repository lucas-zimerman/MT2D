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

#define MT2D_MEMORY_CONTAINER -1

/*REFACTORY VARS*/
#pragma region  Container Struct

typedef struct {
	char *file;//if refCount is -1 it points to a memory address
	unsigned int refCount;// if -1 it represents a memory data and not a container
	MT2D_ContainerFilePath *prev;
	MT2D_ContainerFilePath *next;
}MT2D_ContainerFilePath;

typedef struct {
	/*minimal struct whitch represents the a reference from the file to be stored*/
	char *name;
	unsigned int length; //in bytes
	unsigned char xpadding;//the number of bytes that were added to make this file to be multiple of X
	unsigned int fileDataOffset;//coordinate where the data is located in the file, not counting where the name of the file is located at
	MT2D_ContainerFilePath *DataFileId;
	MT2D_ContainerFile *next;
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

/*
  return an empty FilePath
	path: pointer copy, doesn't clone the path
*/
MT2D_ContainerFilePath *create_MT2D_ContainerFilePath(char * path, bool fromMemory){
	MT2D_ContainerFilePath *filePath = (MT2D_ContainerFilePath*)malloc(sizeof(MT2D_ContainerFilePath));
	filePath->file = path;
	filePath->next = NULL;
	filePath->prev = NULL;
	filePath->refCount = fromMemory ? -1 : 0;
	add_Node(filePath);
	return filePath;
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

/*
	Update the container disk file withe the new data.
	WARNING: this operation may take a long time if you have alot of loaded files
	data is not cloned but copied the reference
*/
bool MT2D_Container_Update(int id, BYTE * data, int length, bool encrypt){
	/*TODO: Implement*/
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	int oldLength = ContainerHash.hash[hashIndex][offset].length;
	ContainerHash.hash[hashIndex][offset].length = length;
	if(ContainerHash.hash[hashIndex][offset].DataFileId->refCount == MT2D_MEMORY_CONTAINER){
		free(ContainerHash.hash[hashIndex][offset].DataFileId->file);
		ContainerHash.hash[hashIndex][offset].DataFileId->file = (char*)data;
	}
	else{
/*
	-COPY EVERYTHING BEFORE THE UPDATED FILE TO A TEMPORARY FILE
	-copy the encrypted name from the file and paste in the tmp FILE
	-paste the new padding + length in the tmp FILE
	-update the data offset in the hash table
	-paste the new data in the tmp FILE
	-COPY the remaining of the files and update their offsets in the hash.
*/
		//PART 1: copy all the content to a temp file, first the content before the updated file will get updated
		MT2D_FILE *tmp = MT2D_FILE_OPEN_TEMP();
		MT2D_FILE *old = MT2D_FILE_OPEN(ContainerHash.hash[hashIndex][offset].DataFileId->file,"r");
		int initialDataOffset;//where the name of the file is located at
		int nameSize;
		int copyBlock = 5000;
		char * block = (char*)malloc(5000*sizeof(char));
		//get the initialDataOffset;
		nameSize = strlen(ContainerHash.hash[hashIndex][offset].name);
		initialDataOffset = oldLength 
							- nameSize
							- 4 /*the \n ,padding byte, length remaining dirt, last \n*/
							- oldLength >> 3;// the real initial data offset pointing to the name

		//start transfering the file before the updated file to a new file
		while(copyBlock < initialDataOffset){
			MT2D_FILE_READ(old,block,5000,1);
			MT2D_FILE_WRITE(tmp,block,5000,1);
			copyBlock += 5000;
		}
		copyBlock = copyBlock - initialDataOffset;// copy the remaining of the file
		if(copyBlock > 0){
			MT2D_FILE_READ(old,block,copyBlock,1);
			MT2D_FILE_WRITE(tmp,block,copyBlock,1);
		}
		//PART 1.2: update the desired file into the temporary file.
		unsigned char oldPadding = ContainerHash.hash[hashIndex][offset].xpadding;
		if(encrypt){
			ContainerHash.hash[hashIndex][offset].xpadding = 0;
			data = decryptAes(data, length,&ContainerHash.hash[hashIndex][offset].xpadding);			
		}
		int pos=0;
		unsigned char *name = read_Hidden_Name(&pos,old);// no need to be decoded
		MT2D_FILE_WRITE(tmp,name,nameSize,1);
		MT2D_FILE_WRITE_BYTE(tmp,'\n');
		int j = nameSize;
		MT2D_FILE_WRITE_BYTE(tmp, ContainerHash.hash[hashIndex][offset].xpadding);
		unsigned char FileLength = 0;
		int Len = 1;
		unsigned char k = 0;
		while (j > 0) {
			if (k == 8) {	//FileLength is full, put it under the file and clear it on the memory
				Len++;
				MT2D_FILE_WRITE_BYTE(tmp, FileLength);
				FileLength = 0;
				k = 0;
			}
			FileLength = FileLength | ( (j & 1) << (k)); //add bit a bit into FileLength
			j = j >> 1;
			k++;
		}
		MT2D_FILE_WRITE_BYTE(tmp, FileLength);//it'll not hurt if FileLength is zero at the end because the software will ignore it
		MT2D_FILE_WRITE_BYTE(tmp,'\n');

		MT2D_FILE_SEEK(old,oldLength + oldPadding + initialDataOffset,SEEK_SET);// skip the old updated file

		initialDataOffset = ContainerHash.hash[hashIndex][offset].fileDataOffset;// reusing the var to point to the old offset
		ContainerHash.hash[hashIndex][offset].fileDataOffset = MT2D_FILE_TELL(tmp); // update the data offset
		MT2D_FILE_WRITE(tmp,data,length + ContainerHash.hash[hashIndex][offset].xpadding, 1);
		//PART 1.3:
		//get the last offset from the old file
		MT2D_ContainerFile *file = &ContainerHash.hash[hashIndex][offset];
		while(file->next != NULL){
			file = file->next;
		}
		//get the difference of data offset
		copyBlock = MT2D_FILE_TELL(old);
		int offsetDiff = MT2D_FILE_TELL(tmp) - copyBlock;
		copyBlock += 5000;
		initialDataOffset = file->fileDataOffset + file->length;
		while(copyBlock < initialDataOffset){
			MT2D_FILE_READ(old,block,5000,1);
			MT2D_FILE_WRITE(tmp,block,5000,1);
			copyBlock += 5000;
		}
		copyBlock = copyBlock - initialDataOffset;// copy the remaining of the file
		if(copyBlock > 0){
			MT2D_FILE_READ(old,block,copyBlock,1);
			MT2D_FILE_WRITE(tmp,block,copyBlock,1);
		}
		//now update the foward files offsets from the updated one 
		file = &ContainerHash.hash[hashIndex][offset];
		while(file->next != NULL){
			file = file->next;
			file->fileDataOffset += offsetDiff;
		}
		MT2D_FILE_CLOSE(old);
		MT2D_FILE_OPEN(ContainerHash.hash[hashIndex][offset].DataFileId->file,"w");
		initialDataOffset = MT2D_FILE_TELL(tmp);
		MT2D_FILE_SEEK(tmp,0,SEEK_SET);
		copyBlock = 5000;
		while(copyBlock < initialDataOffset){
			MT2D_FILE_READ(old,block,5000,1);
			MT2D_FILE_WRITE(tmp,block,5000,1);
			copyBlock += 5000;
		}
		copyBlock = copyBlock - initialDataOffset;// copy the remaining of the file
		if(copyBlock > 0){
			MT2D_FILE_READ(old,block,copyBlock,1);
			MT2D_FILE_WRITE(tmp,block,copyBlock,1);
		}
		MT2D_FILE_CLOSE(tmp);
		MT2D_FILE_CLOSE(old);
	}
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
	-offset: Where the data is located in the file 
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
	ContainerHash.hash[hash][count].fileDataOffset = offset;
	ContainerHash.hash[hash][count].DataFileId = file;
	ContainerHash.count[hash]++;
}


/**
Can load one or more Containers under the main container struct
**/
bool MT2D_Container_Load(char *Path) {
	bool Loaded = false;
	unsigned char fbuffer=0;
	unsigned char *HiddenFileName;
	unsigned int  FileLength;
	unsigned int  dataOffset;
	unsigned char BitPrevious, BitActual;
	unsigned char Padding;
	unsigned char *SmartPointer;
	MT2D_ContainerFilePath *filePath;
	int i = 0, k;
	int Len;
	int j;
	MT2D_FILE *Destination = MT2D_FILE_OPEN(Path, "rb");
	if (Destination) {
		filePath = create_MT2D_ContainerFilePath(Path, false);
		while (!MT2D_FILE_EOF(Destination)) {
			//PART 1: READ THE NAME
			HiddenFileName = read_Hidden_Name(&Len,Destination);
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
				//Half data is inverted, so we lie to SmartPointer so he thinks its an array of chars but instead its just a an integer or "unsigned char .[2]"
				//It might cause an error if the length is too big so the max size of a file is 256 bytes for 8 bits int, 65kb for 16 bits int and 4gb for 32 bits int
				while (fbuffer != '\n' && !MT2D_FILE_EOF(Destination)) {
					fbuffer = MT2D_FILE_READ_BYTE(Destination);
					if (fbuffer != '\n') {
						SmartPointer[j] = fbuffer;
						Len++;
						j++;
					}
				}
				//PART 3: LOAD THE DATA
				container_LoadBuffer(filePath, FileLength, Padding, MT2D_FILE_TELL(Destination), (char*)HiddenFileName);
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
Load a normal file under the container | Path char data cloned and not reused.
-the data will be cloned inside of the memory and it'll not support encryption.
-WARNING: remember to add the '\0' at the end of a raw text file in case you want to use the plain text file from a file
**/
bool MT2D_Container_Load_File(char *Path) {
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
	BYTE *data = (BYTE*)malloc(length*sizeof(data));
  MT2D_FILE_READ(file,data,length,1);
	MT2D_Container_Load_File_From_Memory(Path,data,length);
	MT2D_FILE_CLOSE(file);
	return loaded;
}

void MT2D_Container_Load_File_From_Memory(char *Name, BYTE *Data, int Length) {
	MT2D_ContainerFilePath* path = create_MT2D_ContainerFilePath((char*)Data,true);
	container_LoadBuffer(path,Length,0,0,Name);
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
-Export the loaded data.
-Name: name of the loaded data
-FileName: name of the file
-Path: Where it's going to save the data
-Decrypt: if the file needs to be decrypted
**/
bool MT2D_Container_Export_as_File(char *Name, char * NewName, char *Path, bool decrypt) {
	int Index = MT2D_Container_Get_FileId(Name);
	int OffsetSaved;
	char *Filename;
	bool Saved = false;
	Filename = (char*)malloc(strlen(NewName) + strlen(Path) + 1);
	sprintf(Filename, "%s%s\0",Path, NewName);
	MT2D_FILE *Destination = MT2D_FILE_OPEN(Filename, "wb");
	if (Destination) {
		free(Filename);
		BYTE *data = MT2D_Container_Get_Data(Index,decrypt);
		int length = MT2D_Container_Get_FileLength(Index);
		int xpadding = MT2D_Container_Get_FilePadding(Index);
		//PART 3: SAVE THE DATA
		MT2D_FILE_WRITE(Destination,data, length + xpadding, 1);
		free(data);
		Saved = true;
		MT2D_FILE_CLOSE(Destination);
	}
	return Saved;
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


/*Return a new filename string*/
/*Refactored*/
char *MT2D_Container_Get_FileName_By_ID(int id) {
	int hashIndex = get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if (offset < ContainerHash.count[hashIndex]) {
		int len = strlen(ContainerHash.hash[hashIndex][offset].name);
		char *name = (char*)malloc((len+1)*sizeof(char));
		strcpy(name,ContainerHash.hash[hashIndex][offset].name);
		return name;
	}
	return NULL;
}

/*return a new array of bytes containing the decrypted data. return 0 in case of not found*/
BYTE * MT2D_Container_Get_Data(int id, bool decrypt) {
	int hashIndex = get_Hash_Index(id);
	int hashOffset = get_Hash_File_Index(id);
	int length = ContainerHash.hash[hashIndex][hashOffset].length;
	int offset = ContainerHash.hash[hashIndex][hashOffset].fileDataOffset;
	unsigned char padding = ContainerHash.hash[hashIndex][hashOffset].xpadding;
	BYTE *data = NULL;
	if(ContainerHash.hash[hashIndex][offset].DataFileId->refCount == -1){
			//Data is in memory
			data = clone_data((BYTE*)ContainerHash.hash[hashIndex][offset].DataFileId->file, length);

	}
	else {
		MT2D_FILE *file = MT2D_FILE_OPEN((const char*)ContainerHash.hash[hashIndex][offset].DataFileId->file, "rb");
		MT2D_FILE_SEEK(file, offset, SEEK_SET);
		data = (BYTE*)malloc((offset + padding)*sizeof(BYTE));
		MT2D_FILE_READ(file, data, offset + padding, 1);
		if(decrypt){
			decryptAes(data,length,&padding);
		}
	}
	return data;
}

BYTE* clone_data(BYTE* data, int length)
{
	BYTE *clonedData = NULL;
	if(data){
		clonedData = (BYTE*)malloc(length*sizeof(BYTE));
		memcpy(clonedData,data,length);
	}
	return clonedData;
}

BYTE *decryptAes(BYTE *data, int length, unsigned char *padding){
		struct AES_ctx ctx;
		AES_init_ctx_iv(&ctx, MT2D_ContainerKey, iv);
		if (length % 16 == 0) {
			//It doesn't matter in this case if we're  encoding\decoding
			AES_CTR_xcrypt_buffer(&ctx, data, length);
		}
		else {
			if (padding > 0) {
				//The file is encoded so we're going to encode
				AES_CTR_xcrypt_buffer(&ctx, data, length);
				//remove the padding memory
				data = (BYTE*)realloc(data, (length - padding[0]) * sizeof(BYTE));
			}
			else
			{
				//the file is decoded so we're going to encode
				padding[0] = 16 - (length % 16);
				int i = 1;

				data = (BYTE*)realloc(data, (length + padding[0] + 1) * sizeof(BYTE));
				while (i <= padding[0]) {
					data[length + i] = (i % 2 == 0 ? 255 : 0);
					i++;
				}
				AES_CTR_xcrypt_buffer(&ctx, data, length + padding[0]);
			}
		}
	return data;
}

unsigned char *read_Hidden_Name(int *Len, MT2D_FILE *openedFile){
	Len = 0;
	unsigned char *HiddenFileName = (unsigned  char*)malloc(sizeof(unsigned  char));
	char fbuffer = 0;
	//read line
	while (fbuffer != '\n' && !MT2D_FILE_EOF(openedFile)) {
		fbuffer = MT2D_FILE_READ_BYTE(openedFile);
		if (fbuffer != '\n') {
			HiddenFileName = (unsigned  char*)realloc(HiddenFileName,(Len[0]+1) * sizeof(unsigned  char));
			HiddenFileName[Len[0]] = fbuffer;
			Len++;
		}
		HiddenFileName = (unsigned  char*)realloc(HiddenFileName, (Len[0] + 1) * sizeof(unsigned  char));
		HiddenFileName[Len[0]] = '\0';
	}
	return HiddenFileName;
}






