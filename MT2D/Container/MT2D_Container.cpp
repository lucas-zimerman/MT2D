#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MT2D_Container.h"
#include "../File/MT2D_File.h"
#include "aes.h"

#define xb10000000 128
#define xb00000001 01

MT2D_Container MT2D_GlobalContainer;
BYTE MT2D_ContainerKey[16];
int MT2D_ContainerKeyOffset = 0;
BYTE iv[16] = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

/**
	Return the index of the first file found with the given name.
	-If failed the function returns -1
**/



int MT2D_Container_Get_FileId(char *Name) {
	int Index = -1;
	int j;
	for (int i = 0; i < MT2D_GlobalContainer.Files; i++) {
		j = 0;
		while (Name[j] != '\0' && MT2D_GlobalContainer.Names[i][j] != '\0' && Name[j] == MT2D_GlobalContainer.Names[i][j]) {
			j++;
		}
		if (Name[j] == '\0') {
			if (MT2D_GlobalContainer.Names[i][j] == '\0') {
				Index = i;
			}
		}
	}
	return Index;
}

int MT2D_Container_Get_FileLength(int Id) {
	int Size = -1;
	if (MT2D_GlobalContainer.Files > Id) {
		Size = MT2D_GlobalContainer.Length[Id];
	}
	return Size;
}

int MT2D_Container_Get_FilePadding(int Id) {
	int Size = -1;
	if (MT2D_GlobalContainer.Files > Id) {
		Size = MT2D_GlobalContainer.Xpadding[Id];
	}
	return Size;
}

BYTE *MT2D_Container_Get_FileData(int Id) {
	BYTE *Data =0;
	if (MT2D_GlobalContainer.Files > Id) {
		Data = MT2D_GlobalContainer.Data[Id];
	}
	return Data;
}


void MT2D_Container_Init() {
	MT2D_GlobalContainer.Data = 0;
	MT2D_GlobalContainer.Files = 0;
	MT2D_GlobalContainer.Length = 0;
	MT2D_GlobalContainer.Names = 0;
	MT2D_GlobalContainer.Xpadding = 0;
}

/**
Free the loaded files
**/
void MT2D_Container_Clear() {
	if (MT2D_GlobalContainer.Files > 0) {
		free(MT2D_GlobalContainer.Length);
		free(MT2D_GlobalContainer.Xpadding);
		int i = 0;
		while (i < MT2D_GlobalContainer.Files) {
			free(MT2D_GlobalContainer.Data[i]);
			free(MT2D_GlobalContainer.Names[i]);
			i++;
		}
		free(MT2D_GlobalContainer.Data);
		free(MT2D_GlobalContainer.Names);
		MT2D_GlobalContainer.Files = 0;
	}
}

/**
	-Name is not cloned here, we reuse the pointer that u send to us, so look out if you're going to use that pointer later.
	-Also this function does not perform fclose
**/
void MT2D_Container_LoadBuffer(MT2D_FILE  *file, int Length, int Padding, char *Name) {
	if (MT2D_GlobalContainer.Files == 0) {
		MT2D_GlobalContainer.Data = (BYTE**)malloc(sizeof(BYTE*));
		MT2D_GlobalContainer.Length = (unsigned int*)malloc(sizeof(unsigned int));
		MT2D_GlobalContainer.Names = (char**)malloc(sizeof(char*));
		MT2D_GlobalContainer.Xpadding = (unsigned char*)malloc(sizeof(unsigned char));
	}
	else
	{
		MT2D_GlobalContainer.Files++;
		MT2D_GlobalContainer.Data = (BYTE**)realloc(MT2D_GlobalContainer.Data, MT2D_GlobalContainer.Files*sizeof(BYTE*));
		MT2D_GlobalContainer.Length = (unsigned int*)realloc(MT2D_GlobalContainer.Length, MT2D_GlobalContainer.Files * sizeof(unsigned int));
		MT2D_GlobalContainer.Names = (char**)realloc(MT2D_GlobalContainer.Names, MT2D_GlobalContainer.Files*sizeof(char*));
		MT2D_GlobalContainer.Xpadding = (unsigned char*)realloc(MT2D_GlobalContainer.Xpadding, MT2D_GlobalContainer.Files * sizeof(unsigned char));
		MT2D_GlobalContainer.Files--;
	}
	MT2D_GlobalContainer.Names[MT2D_GlobalContainer.Files] = Name;
	MT2D_GlobalContainer.Length[MT2D_GlobalContainer.Files] = Length;
	MT2D_GlobalContainer.Xpadding[MT2D_GlobalContainer.Files] = Padding;
	MT2D_GlobalContainer.Data[MT2D_GlobalContainer.Files] = (BYTE*)malloc((Length+Padding) * sizeof(BYTE));
	MT2D_FILE_READ(file, MT2D_GlobalContainer.Data[MT2D_GlobalContainer.Files], Length + Padding, 1);
	MT2D_GlobalContainer.Files++;
}


/**
Can load one or more Containers under the main container struct
**/
bool MT2D_Container_Load(char *Path) {
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
				//PART 1.: DECODE THE NAME
				j = Len - 1;
				BitPrevious = 0;// garbage's going to be inserted on the first loop and fixed after the loop
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
				SmartPointer = (unsigned char*)&FileLength;//get the Length pointer in form of 1byte
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
				MT2D_Container_LoadBuffer(Destination, FileLength, Padding, (char*)HiddenFileName);
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
	bool Loaded = false;
	MT2D_FILE *file = MT2D_FILE_OPEN(Path, "rb");
	if (file) {

		Loaded = true;
	}
	int length = strlen(Path) +1;
	char *NewString = (char*)malloc(length * sizeof(char));
	strcpy(NewString, Path);

	//get the size from the data
	MT2D_FILE_SEEK(file, 0L, SEEK_END);
	length = MT2D_FILE_TELL(file);
	MT2D_FILE_SEEK(file, 0L, SEEK_SET);
	MT2D_Container_LoadBuffer(file, length, 0, NewString);
	MT2D_FILE_CLOSE(file);
	return Loaded;
}

void MT2D_Container_Load_File_From_Memory(char *Name, BYTE *Data, int Length) {
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
**/
void MT2D_Container_Decode_File(char *Name) {
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


void MT2D_Container_Password_Init() {
	int i = 0;
	while (i < 16) {
		MT2D_ContainerKey[i] = (i % 2 == 0 ? 0 : 255);
		i++;
	}
	MT2D_ContainerKeyOffset = 0;
}

void MT2D_Container_Password_AddKey(unsigned char Key) {
	MT2D_ContainerKey[MT2D_ContainerKeyOffset] = Key;
	MT2D_ContainerKeyOffset++;
	MT2D_ContainerKeyOffset = MT2D_ContainerKeyOffset % 16;
}


int MT2D_Container_Count_Files() {
	return MT2D_GlobalContainer.Files;
}

/*Return the pointer of the name of a file form the given ID*/
char *MT2D_Container_Get_FileName_By_ID(int ID) {
	if (ID >= 0 && ID < MT2D_GlobalContainer.Files) {
		return MT2D_GlobalContainer.Names[ID];
	}
	return NULL;
}

/*return a new array of bytes containing the decrypted data. return 0 in case of not found*/
BYTE * MT2D_Container_Get_Data(int ID, bool needDecryption) {
	BYTE *buffer = 0;
	if (ID < MT2D_GlobalContainer.Files){
		MT2D_FILE *file = MT2D_FILE_OPEN((const char*)MT2D_GlobalContainer.FilePath[MT2D_GlobalContainer.DataFileId[ID]], "rb");
		MT2D_FILE_SEEK(file, MT2D_GlobalContainer.DataOffset[ID], SEEK_SET);

	}
	return buffer;
}