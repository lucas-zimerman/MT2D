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
#include <MT2D/MT2D_Debug.h>
#include "MT2D_Container.h"
#include "../File/MT2D_File.h"
#include "aes.h"

#define xb10000000 128
#define xb00001111 15
#define xb00000001 01


/*REFACTORY VARS*/
#pragma region Container Global Vars

MT2D_ContainerHash ContainerHash;
MT2D_ContainerList ListFilePaths;
BYTE MT2D_ContainerKey[16];
int MT2D_ContainerKeyOffset = 0;
BYTE iv[16]; /* { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };*/

#pragma endregion Container Global Vars

#pragma region Internal Helpers

bool file_is_empty(MT2D_FILE* file) {
	unsigned int pos = MT2D_FILE_TELL(file);
	int size;
		MT2D_FILE_SEEK(file, 0, SEEK_END);
		size = ftell(file);
		MT2D_FILE_SEEK(file, pos, SEEK_SET);
		if (size == 0) {
			return true;
		}
	return false;
}

/*Refactored*/
int get_Hash_Index(int id) {
	return id & xb00001111;
}

/*Refactored*/
int get_Hash_File_Index(int id) {
	return id >> 4;
}


bool try_remove_container(MT2D_ContainerFilePath* file) {
	if (file->refCount == 0 || file->refCount == MT2D_CONT_DATATYPE_MEM) {
		if (file->prev != NULL)
		{
			file->prev->next = file->next;
		}
		else
		{
			ListFilePaths.start = file->next;
		}
		if (file->next != NULL) {
			file->next->prev = file->prev;
		}
		else
		{
			ListFilePaths.end = file->prev;
		}
		free(file->file);
		free(file);
		return true;
	}
	return false;
}

BYTE* clone_data(BYTE* data, int length)
{
	BYTE* clonedData = NULL;
	if (data) {
		clonedData = (BYTE*)malloc(length * sizeof(BYTE));
		memcpy(clonedData, data, length);
	}
	return clonedData;
}

BYTE* decryptAes(BYTE* data, int length, unsigned char* padding) {
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, MT2D_ContainerKey, iv);
	if (length % 16 == 0) {
		//It doesn't matter in this case if we're  encoding\decoding
		AES_CTR_xcrypt_buffer(&ctx, data, length);
	}
	else {
		if (padding[0] > 0) {
			//The file is encoded so we're going to encode
			AES_CTR_xcrypt_buffer(&ctx, data, length);
			//remove the padding memory
			data = (BYTE*)realloc(data, (length + 1) * sizeof(BYTE));
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
		//data[length + padding[0] - 1] = '\0';
	}
	return data;
}

void write_Name(MT2D_FILE* openFile, char* name) {
	int Len = strlen(name);
	unsigned char* HiddenFileName = (unsigned char*)malloc((Len + 1) * sizeof(unsigned char));
	strcpy((char*)HiddenFileName, name);
	unsigned int j = 0;
	unsigned char BitPrevious = HiddenFileName[j] & (1 << 7);
	unsigned char BitActual;
	while ((int)j < Len) {
		BitActual = HiddenFileName[j] & 1;
		HiddenFileName[j] = HiddenFileName[j] >> 1;
		HiddenFileName[j] = HiddenFileName[j] | BitPrevious << 7;
		BitPrevious = BitActual;
		j++;
	}
	HiddenFileName[0] = HiddenFileName[0] | BitPrevious << 7;
	MT2D_FILE_WRITE(openFile, HiddenFileName, Len, 1);
	free(HiddenFileName);
	MT2D_FILE_WRITE_BYTE(openFile, '\n');
}

unsigned char* read_Hidden_Name(int* Len, MT2D_FILE* openedFile) {
	Len[0] = 0;
	unsigned char* HiddenFileName = (unsigned  char*)malloc(sizeof(unsigned  char));
	char fbuffer = 0;
	//read line
	while (fbuffer != '\n' && !MT2D_FILE_EOF(openedFile)) {
		fbuffer = MT2D_FILE_READ_BYTE(openedFile);
		if (fbuffer != '\n') {
			HiddenFileName = (unsigned  char*)realloc(HiddenFileName, (Len[0] + 1) * sizeof(unsigned  char));
			HiddenFileName[Len[0]] = fbuffer;
			Len[0]++;
		}
		HiddenFileName = (unsigned  char*)realloc(HiddenFileName, (Len[0] + 1) * sizeof(unsigned  char));
		HiddenFileName[Len[0]] = '\0';
	}
	return HiddenFileName;
}
void write_padding_length(MT2D_FILE* openedFile, unsigned char padding, int length) {
	unsigned char k = 1;
	unsigned int lengthSize = length;
	while (lengthSize > 255) {
		k++;
		lengthSize = lengthSize >> 8;
	}
	MT2D_FILE_WRITE_BYTE(openedFile, padding + k << 4); //Write the padding  on the right and the amount of lenght bytes on the left (max 4 bits)
	MT2D_FILE_WRITE(openedFile, (unsigned char*)&length, k , 1);//write all the bytes of the integer length
}

void transfer_file_data(MT2D_FILE* source, MT2D_FILE* destination, unsigned int initOffset, unsigned int finalOffset, unsigned int bufferSize, BYTE* buffer) {
	unsigned int copybuffer = initOffset + bufferSize;
	while (copybuffer < finalOffset) {
		MT2D_FILE_READ(source, buffer, bufferSize, 1);
		MT2D_FILE_WRITE(destination, buffer, bufferSize, 1);
		copybuffer += bufferSize;
	}
	copybuffer = copybuffer - finalOffset;// copy the remaining of the file
	if (copybuffer > 0) {
		MT2D_FILE_READ(source, buffer, copybuffer, 1);
		MT2D_FILE_WRITE(destination, buffer, copybuffer, 1);
	}
}

/*Refactored*/
void remove_Node(MT2D_ContainerFilePath* node) {
	/*Edit the ListFilePaths in case this node is the first or the last node*/
	if (node->prev != NULL) {
		node->prev->next = node->next;
	}
	else {
		ListFilePaths.start = node->next;
	}
	if (node->next != NULL) {
		node->next->prev = node->prev;
	}
	else {
		ListFilePaths.end = node->prev;
	}
	free(node->file);
	free(node);
}

/*Refactored*/
void add_Node(MT2D_ContainerFilePath* node) {
	/*Always add the node to the end*/
	if (ListFilePaths.start == NULL) {
		ListFilePaths.start = node;
		ListFilePaths.end = node;
	}
	else if (ListFilePaths.start == ListFilePaths.end) {
		ListFilePaths.end = node;
		node->prev = ListFilePaths.start;
		ListFilePaths.start->next = node;
	}
	else {
		ListFilePaths.end->next = node;
		node->prev = ListFilePaths.end;
		ListFilePaths.end = node;
	}
}

/*
  return an empty FilePath
	path: pointer copy, doesn't clone the path
*/
MT2D_ContainerFilePath* create_MT2D_ContainerFilePath(char* path, bool fromMemory) {
	MT2D_ContainerFilePath* filePath = (MT2D_ContainerFilePath*)malloc(sizeof(MT2D_ContainerFilePath));
	filePath->file = path;
	filePath->next = NULL;
	filePath->prev = NULL;
	filePath->refCount = fromMemory ? MT2D_CONT_DATATYPE_MEM : 0;
	add_Node(filePath);
	return filePath;
}

/**
	-Name is not cloned here, we reuse the pointer that u sent to us, so look out if you're going to use that pointer later.
	-Note: this function doesn't close the MT2D_FILE
	-offset: Where the data is located in the file
	Refactored
**/
MT2D_ContainerFile* container_LoadBuffer(MT2D_ContainerFilePath* file, int length, int padding, int offset, char* name, MT2D_ContainerFile* previousData) {
	int hash = get_Hash_Index(name[0]);
	int count = ContainerHash.count[hash];
	if (count == 0) {
		ContainerHash.hash[hash] = (MT2D_ContainerFile**)malloc(sizeof(MT2D_ContainerFile*));
	}
	else
	{
		ContainerHash.hash[hash] = (MT2D_ContainerFile**)realloc(ContainerHash.hash[hash], (count + 1) * sizeof(MT2D_ContainerFile*));
	}
	MT2D_ContainerFile* data = (MT2D_ContainerFile*)malloc(sizeof(MT2D_ContainerFile));
	data->name = name;
	data->length = length;
	data->xpadding = padding;
	data->fileDataOffset = offset;
	data->DataFileId = file;
	data->next = NULL;
	ContainerHash.hash[hash][count] = data;
	ContainerHash.count[hash]++;
	if (file->refCount != -1) {
		file->refCount++;
		if (file->refCount == 1) {
			file->firstData = data;
		}
		else if (previousData != NULL)
		{
			previousData->next = data;
		}
	}
	return data;
}

void remove_filedata_node(MT2D_ContainerFile* data)
{
	MT2D_ContainerFile* first = (MT2D_ContainerFile*)data->DataFileId->firstData;
	//
	while (first->next != data)
	{
		first = first->next;
	}
	first->next = first->next->next;
}

/*
* deleted the data from the physical disc or memory if it's a memory container.
* if it's a container with more than one file, the desired data will be removed
* and the next datas will get rearranged
*/
void delete_data_from_file(int offset, int index) {
	if (ContainerHash.hash[index][offset]->DataFileId == NULL); //Already deleted
	else {
		ContainerHash.hash[index][offset]->DataFileId->refCount--;

		if (ContainerHash.hash[index][offset]->fileDataOffset == 0 || ContainerHash.hash[index][offset]->DataFileId->refCount == 0)
		{
			MT2D_FILE_DELETE(ContainerHash.hash[index][offset]->DataFileId->file);
		}
		else if (ContainerHash.hash[index][offset]->DataFileId->refCount == MT2D_CONT_DATATYPE_CONT) {
			//PART 1: copy all the content to a temp file, first the content before the updated file will get updated
			MT2D_FILE* tmp = MT2D_FILE_OPEN_TEMP();
			MT2D_FILE* old = MT2D_FILE_OPEN(ContainerHash.hash[index][offset]->DataFileId->file, "r");
			int initialDataOffset;//where the name of the file is located at
			int nameSize;
			unsigned int oldLength = ContainerHash.hash[index][offset]->length;
			unsigned int oldPadding = ContainerHash.hash[index][offset]->xpadding;
			BYTE* buffer = (BYTE*)malloc(5000 * sizeof(BYTE));
			//get the initialDataOffset;
			nameSize = strlen(ContainerHash.hash[index][offset]->name);
			initialDataOffset = oldLength
				- nameSize
				- 4 /*the \n ,padding byte, length remaining dirt, last \n*/
				- (oldLength >> 3);// the real initial data offset pointing to the name

//start transfering the file before the updated file to a new file
			transfer_file_data(old, tmp, 0, initialDataOffset, 5000, buffer);
			MT2D_FILE_SEEK(old, oldLength + oldPadding + initialDataOffset, SEEK_SET);// skip the old updated file

			initialDataOffset = ContainerHash.hash[index][offset]->fileDataOffset;// reusing the var to point to the old offset
			//PART 1.3:
			//get the last offset from the old file
			MT2D_ContainerFile* file = ContainerHash.hash[index][offset];
			while (file->next != NULL) {
				file = file->next;
			}
			int offsetDiff = MT2D_FILE_TELL(old);
			initialDataOffset = file->fileDataOffset + file->length;
			transfer_file_data(old, tmp, offsetDiff, initialDataOffset, 5000, buffer);
			offsetDiff = MT2D_FILE_TELL(tmp) - offsetDiff;

			remove_filedata_node(ContainerHash.hash[index][offset]);
			//now update the foward files offsets from the updated one 
			file = ContainerHash.hash[index][offset];
			while (file->next != NULL) {
				file = file->next;
				file->fileDataOffset += offsetDiff;
			}
			MT2D_FILE_CLOSE(old);
			old = MT2D_FILE_OPEN(ContainerHash.hash[index][offset]->DataFileId->file, "w");
			initialDataOffset = MT2D_FILE_TELL(tmp);
			MT2D_FILE_SEEK(tmp, 0, SEEK_SET);
			transfer_file_data(tmp, old, 0, initialDataOffset, 5000, buffer);

			MT2D_FILE_CLOSE(tmp);
			MT2D_FILE_CLOSE(old);
			free(buffer);
		}
		free(ContainerHash.hash[index][offset]->name);
		try_remove_container(ContainerHash.hash[index][offset]->DataFileId);
		ContainerHash.hash[index][offset]->DataFileId = NULL;// This looks so wrong but I blame vscode for nagging
	}
}
/*
	Doesn't update the reference count
*/
void free_MT2D_ContainerFile(MT2D_ContainerFile* file)
{
	if (file->DataFileId->refCount > 0) {
		file->DataFileId->refCount--;
	}
	free(file->name);
	free(file);
}



MT2D_ContainerFile* get_last_file_from_container(MT2D_ContainerFilePath* container)
{
	MT2D_ContainerFile* file = (MT2D_ContainerFile*)container->firstData;
	if (container->refCount == MT2D_CONT_DATATYPE_MEM) {
		free(container->file);//TODO: check
		return file;
	}
	else if (container->refCount == 0) {
		return NULL;
	}
	while (file->next != NULL)
	{
		file = file->next;
	}
	return file;
}

#pragma endregion

/**
 * 	Return the index of the container file found with the given name.
	-If failed the function returns -1
**/
int MT2D_Container_Get_FileId(char *name) {
	MT2D_Ide_Printf("Getting Container ID FOR");
	MT2D_Ide_Printf(name);
	int hashIndex = name[0] & xb00001111;
	int j;
	for (int i = 0; i < ContainerHash.count[hashIndex]; i++) {
		j = 0;
		/*hand made strcmp*/
		while ( name[j] != '\0' && 
				ContainerHash.hash[hashIndex][i]->name[j] != '\0' && 
				name[j] == ContainerHash.hash[hashIndex][i]->name[j]
			  ) {
			j++;
		}
		if (name[j] == '\0') {
			if (ContainerHash.hash[hashIndex][i]->name[j] == '\0') {
				//found the file
				MT2D_Ide_Printf("data found");
				return (i << 4) | hashIndex;
			}
		}
	}
	MT2D_Ide_Printf("data not found");

	return -1;
}

int MT2D_Container_Get_FileLength(int id) {
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if(ContainerHash.count[hashIndex] > offset)
	{
		return ContainerHash.hash[hashIndex][offset]->length;
	}
	return -1;
}

int MT2D_Container_Get_FilePadding(int id) {
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if(ContainerHash.count[hashIndex] < offset)
	{
		return ContainerHash.hash[hashIndex][offset]->xpadding;
	}
	return -1;
}


MT2D_Container_DataStore_Type MT2D_GetFile_StoreType(int id){
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	if(ContainerHash.hash[hashIndex][offset]->fileDataOffset == 0){
		return MT2D_CONT_DATATYPE_FILE;
	}
	else if(ContainerHash.hash[hashIndex][offset]->DataFileId->refCount == MT2D_CONT_DATATYPE_MEM){
		return MT2D_CONT_DATATYPE_MEM;
	}
	return MT2D_CONT_DATATYPE_CONT;
}

/*
	Update the container disk file withe the new data.
	WARNING: this operation may take a long time if you have alot of loaded files
	data is not cloned but copied the reference
*/
bool MT2D_Container_Update(int id, BYTE * data, int length, bool encrypt){
	int hashIndex  =get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	int oldLength = ContainerHash.hash[hashIndex][offset]->length;
	ContainerHash.hash[hashIndex][offset]->length = length;
	if(ContainerHash.hash[hashIndex][offset]->DataFileId->refCount == MT2D_CONT_DATATYPE_MEM){
		free(ContainerHash.hash[hashIndex][offset]->DataFileId->file);
		ContainerHash.hash[hashIndex][offset]->DataFileId->file = (char*)data;
	}
	else if(ContainerHash.hash[hashIndex][offset]->fileDataOffset == 0){
		//the updated data is inside of a file and not a container
		MT2D_FILE *file = MT2D_FILE_OPEN(ContainerHash.hash[hashIndex][offset]->DataFileId->file,"w");
		MT2D_FILE_WRITE(file,data,length,1);
		MT2D_FILE_CLOSE(file);
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
		MT2D_FILE *old = MT2D_FILE_OPEN(ContainerHash.hash[hashIndex][offset]->DataFileId->file,"r");
		int initialDataOffset;//where the name of the file is located at
		int nameSize;
		BYTE* buffer = (BYTE*)malloc(5000*sizeof(BYTE));
		//get the initialDataOffset;
		nameSize = strlen(ContainerHash.hash[hashIndex][offset]->name);
		initialDataOffset = oldLength 
							- nameSize
							- 4 /*the \n ,padding byte, length remaining dirt, last \n*/
							- (oldLength >> 3);// the real initial data offset pointing to the name

		//start transfering the file before the updated file to a new file
		transfer_file_data(old,tmp,0,initialDataOffset,5000,buffer);

		//PART 1.2: update the desired file into the temporary file.
		unsigned char oldPadding = ContainerHash.hash[hashIndex][offset]->xpadding;
		if(encrypt){
			ContainerHash.hash[hashIndex][offset]->xpadding = 0;
			data = decryptAes(data, length,&ContainerHash.hash[hashIndex][offset]->xpadding);			
		}
		int pos=0;
		unsigned char *name = read_Hidden_Name(&pos,old);// no need to be decoded
		MT2D_FILE_WRITE(tmp,name,nameSize,1);
		MT2D_FILE_WRITE_BYTE(tmp,'\n');
	    write_padding_length(tmp,ContainerHash.hash[hashIndex][offset]->xpadding, length);

		MT2D_FILE_SEEK(old,oldLength + oldPadding + initialDataOffset,SEEK_SET);// skip the old updated file

		initialDataOffset = ContainerHash.hash[hashIndex][offset]->fileDataOffset;// reusing the var to point to the old offset
		ContainerHash.hash[hashIndex][offset]->fileDataOffset = MT2D_FILE_TELL(tmp); // update the data offset
		MT2D_FILE_WRITE(tmp,data,length + ContainerHash.hash[hashIndex][offset]->xpadding, 1);
		//PART 1.3:
		//get the last offset from the old file
		MT2D_ContainerFile *file = ContainerHash.hash[hashIndex][offset];
		while (file->next != NULL) {
			file = file->next;
		}
		int offsetDiff = MT2D_FILE_TELL(old);
		initialDataOffset = file->fileDataOffset + file->length;
		transfer_file_data(old,tmp,offsetDiff,initialDataOffset,5000,buffer);
		offsetDiff = MT2D_FILE_TELL(tmp) - offsetDiff;

		//now update the foward files offsets from the updated one 
		file = ContainerHash.hash[hashIndex][offset];
		while(file->next != NULL){
			file = file->next;
			file->fileDataOffset += offsetDiff;
		}
		MT2D_FILE_CLOSE(old);
		old = MT2D_FILE_OPEN(ContainerHash.hash[hashIndex][offset]->DataFileId->file,"w");
		initialDataOffset = MT2D_FILE_TELL(tmp);
		MT2D_FILE_SEEK(tmp,0,SEEK_SET);
		transfer_file_data(tmp,old,0,initialDataOffset,5000,buffer);

		MT2D_FILE_CLOSE(tmp);
		MT2D_FILE_CLOSE(old);
		free(buffer);
	}
	return false;
}

/*
 * Delete the referenced id from the disc or memory
*/
void MT2D_Container_Delete_Data(int id){
	int hashIndex = get_Hash_Index(id);
	int offset = get_Hash_File_Index(id);
	delete_data_from_file(offset,hashIndex);
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
			free_MT2D_ContainerFile(ContainerHash.hash[i][j]);
		}
		free(ContainerHash.hash[i]);
		ContainerHash.count[i] = 0;
	}
	//Clear the files
    ListFilePaths.start;
	while(ListFilePaths.start != NULL){
		try_remove_container(ListFilePaths.start);
	}
}




/**
Can load one or more Containers under the main container struct
**/
bool MT2D_Container_Load(char *Path) {
	bool Loaded = false;
	unsigned char *HiddenFileName;
	unsigned int  FileLength;
	unsigned char BitPrevious, BitActual;
	unsigned char Padding;
	unsigned char *SmartPointer;
	MT2D_ContainerFilePath *filePath;
	MT2D_ContainerFile *previousData = NULL;
	int Len = 0;
	int j;
	MT2D_Ide_Printf("Loading Contaiener");
	MT2D_Ide_Printf(Path);
	MT2D_FILE *Destination = MT2D_FILE_OPEN(Path, "rb");
	if (Destination) {
		MT2D_Ide_Printf("Container found");
		filePath = create_MT2D_ContainerFilePath(Path, false);
		MT2D_Ide_Printf("Container file path loaded");
		if (!file_is_empty(Destination)) {
			while (!MT2D_FILE_EOF(Destination)) {
				//PART 1: READ THE NAME
				HiddenFileName = read_Hidden_Name(&Len, Destination);
				MT2D_Ide_Printf("Container Got Hidden Name");
				MT2D_Ide_Printf((char*)HiddenFileName);
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
					Len = Padding >> 4;
					Padding = Padding & 15;
					//PART 2.2:GET THE LENGTH
					FileLength = 0;
					SmartPointer = (unsigned char*)&FileLength;//get the Length pointer in form of a byte
					j = 0;
					//Half data is inverted, so we lie to SmartPointer so he thinks its an array of chars but instead its just a an integer or "unsigned char .[2]"
					//It might cause an error if the length is too big so the max size of a file is 256 bytes for 8 bits int, 65kb for 16 bits int and 4gb for 32 bits int
					
					MT2D_Ide_Printf("Got Padding and length reference");

					MT2D_FILE_READ(Destination,SmartPointer,Len ,1);

					MT2D_Ide_Printf("Got Length");
					//PART 3: LOAD THE DATA
					previousData = container_LoadBuffer(filePath, FileLength, Padding, MT2D_FILE_TELL(Destination), (char*)HiddenFileName, previousData);

					MT2D_Ide_Printf("Data Loaded");

					MT2D_FILE_SEEK(Destination, FileLength, SEEK_CUR);
					MT2D_FILE_READ_BYTE(Destination);//Jump the \n
				}
				else {
					free(HiddenFileName);
				}
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
	MT2D_Ide_Printf("Loading Container File");
	MT2D_Ide_Printf(Path);
	bool loaded = false;
	MT2D_FILE *file = MT2D_FILE_OPEN(Path, "rb");
	if (file) {
		MT2D_Ide_Printf("Container File found");
		loaded = true;
	}
	int length = strlen(Path) +1;
	char *newString = (char*)malloc(length * sizeof(char));
	char* newString2 = (char*)malloc(length * sizeof(char));	
	strcpy(newString, Path);
	strcpy(newString2, Path);

	//get the size from the data
	MT2D_FILE_SEEK(file, 0L, SEEK_END);
	length = MT2D_FILE_TELL(file) + 1;
	MT2D_ContainerFilePath* path = create_MT2D_ContainerFilePath(newString, false);
	container_LoadBuffer(path, length -1 , 0, 0, newString2, NULL);
	MT2D_FILE_CLOSE(file);
	return loaded;
}

void MT2D_Container_Load_File_From_Memory(char *Name, BYTE *Data, int Length) {
	MT2D_ContainerFilePath* path = create_MT2D_ContainerFilePath((char*)Data,true);
	container_LoadBuffer(path,Length,0,0,Name,  NULL);
}
/**
Save all the loaded files in the container for an external file
**/
/*
bool MT2D_Container_Save(char *NameAndPath) {

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
*/
/**
-Export the loaded data.
-Name: name of the loaded data
-FileName: name of the file
-Path: Where it's going to save the data
-Decrypt: if the file needs to be decrypted
**/
bool MT2D_Container_Export_as_File(int index, char * NewName, char *Path, bool decrypt) {
	int OffsetSaved;
	char *Filename;
	bool Saved = false;
	Filename = (char*)malloc(strlen(NewName) + strlen(Path) + 1);
	sprintf(Filename, "%s%s\0",Path, NewName);
	MT2D_FILE *Destination = MT2D_FILE_OPEN(Filename, "wb");
	if (Destination) {
		free(Filename);
		BYTE *data = MT2D_Container_Get_Data(index,decrypt);
		int length = MT2D_Container_Get_FileLength(index);
		int xpadding = MT2D_Container_Get_FilePadding(index);
		//PART 3: SAVE THE DATA
		MT2D_FILE_WRITE(Destination,data, length + xpadding, 1);
		free(data);
		Saved = true;
		MT2D_FILE_CLOSE(Destination);
	}
	return Saved;
}

/**
 * Copy the data loaded in MT2D_Container into an existing container.
 * -containerName: the valid name of the container file
 * -dataId: the id to be copied
 * -encrypt: if the copied file is going to be encrypted (mark as false if the copied file is already encrypted)
 * return false if the container wasn't found
 **/
bool MT2D_Container_Export_into_Container(int dataId, char *containerName, bool encrypt){
	if(ListFilePaths.start != NULL){
		MT2D_ContainerFilePath *cont = ListFilePaths.start;
		bool found = false;
		while(cont != NULL && !found){
			if(cont->refCount == MT2D_CONT_DATATYPE_MEM){
				cont = cont->next;
			}
			else if(strcmp(cont->file,containerName) == 0){//TODO: check this part
				found = true;
			}
			else{
				cont = cont->next;
			}
		}
		if(found){
			MT2D_FILE *contFile = MT2D_FILE_OPEN(cont->file,"ab");

			int hashIndex  =get_Hash_Index(dataId);
			int offset = get_Hash_File_Index(dataId);

			if (!file_is_empty(contFile)) {
				MT2D_FILE_WRITE_BYTE(contFile, '\n');
			}
			write_Name(contFile,ContainerHash.hash[hashIndex][offset]->name);

			unsigned char* data = MT2D_Container_Get_Data(dataId,false);
			unsigned int length = ContainerHash.hash[hashIndex][offset]->length;
			unsigned char padding = ContainerHash.hash[hashIndex][offset]->xpadding;

			if(encrypt){
				data = decryptAes(data,length,&padding);
			}

			write_padding_length(contFile,padding,length);

			unsigned int originalNamelen = strlen(ContainerHash.hash[hashIndex][offset]->name);
			char *originalName = (char*)malloc((originalNamelen + 1) * sizeof(char));
			strcpy(originalName,ContainerHash.hash[hashIndex][offset]->name);

			container_LoadBuffer(cont, length, padding, MT2D_FILE_TELL(contFile), originalName,get_last_file_from_container(cont));
			
			MT2D_FILE_WRITE(contFile,data,length + padding,1);
			MT2D_FILE_CLOSE(contFile);
			return true;
		}
	}
	return false;
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
		int len = strlen(ContainerHash.hash[hashIndex][offset]->name);
		char *name = (char*)malloc((len+1)*sizeof(char));
		strcpy(name,ContainerHash.hash[hashIndex][offset]->name);
		return name;
	}
	return NULL;
}

/*return a new array of bytes containing the decrypted data. return 0 in case of not found*/
BYTE * MT2D_Container_Get_Data(int id, bool decrypt) {
	int hashIndex = get_Hash_Index(id);
	int hashOffset = get_Hash_File_Index(id);
	int length = ContainerHash.hash[hashIndex][hashOffset]->length;
	int offset = ContainerHash.hash[hashIndex][hashOffset]->fileDataOffset;
	unsigned char padding = ContainerHash.hash[hashIndex][hashOffset]->xpadding;
	BYTE *data = NULL;
	if(ContainerHash.hash[hashIndex][hashOffset]->DataFileId->refCount == -1){
			//Data is in memory
			data = clone_data((BYTE*)ContainerHash.hash[hashIndex][hashOffset]->DataFileId->file, length);

	}
	else {
		MT2D_FILE *file = MT2D_FILE_OPEN((const char*)ContainerHash.hash[hashIndex][hashOffset]->DataFileId->file, "rb");
		MT2D_FILE_SEEK(file, offset, SEEK_SET);
		data = (BYTE*)malloc((offset + padding + length)*sizeof(BYTE));
		MT2D_FILE_READ(file, data, padding + length, 1);
		if(decrypt){
			decryptAes(data,length,&padding);
		}
		data[padding + length] = '\0';
		MT2D_FILE_CLOSE(file);
	}
	return data;
}
