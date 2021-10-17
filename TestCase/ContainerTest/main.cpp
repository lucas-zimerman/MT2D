#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/File/MT2D_File.h>

int main(int argc, char** argv)
{
	MT2D_Container_Init();
	printf("StartTest OK\n");

	MT2D_Container_Password_Init();
	printf("StartEncryption OK\n");

	int ret = MT2D_Container_Count_Files();
	printf("CountEmpty %d = 0 ? OK\n", ret);

	MT2D_Container_Load_File((char*)"helloworld.txt");
	printf("File Loaded OK\n");

	ret = MT2D_Container_Count_Files();
	printf("CountEmpty %d = 1 ? OK\n", ret);

	int id = MT2D_Container_Get_FileId((char*)"helloworld.txt");
	printf("File Id  %d ? OK\n", id);

	int length = MT2D_Container_Get_FileLength(id);
	printf("File Length  %d ? OK\n", length);

	unsigned char* data = MT2D_Container_Get_Data(id, false);
	printf("got\n%s\nexpected\nHi World\n", data);

	char constData[] = "data data and more data!";
	char* dataData = (char*)malloc((strlen(constData) + 1) * sizeof(char));
	strcpy(dataData, constData);
	dataData[strlen(dataData)] = '\0';
	const char constdataName[] = "data";
	char* dataName = (char*)malloc((strlen(constdataName)+1) * sizeof(char));
	strcpy(dataName, constdataName);
	MT2D_Container_Load_File_From_Memory(dataName, (unsigned char*)dataData, strlen(dataData) + 1);
	printf("Loaded memory data into container\n");

	const char constdataName2[] = "data2";
	char* dataName2 = (char*)malloc((strlen(constdataName2) + 1) * sizeof(char));
	strcpy(dataName2, constdataName2);
	char constData2[] = "data data and more data!\0 ";
	char* dataData2 = (char*)malloc((strlen(constData2) + 1) * sizeof(char));
	strcpy(dataData2, constData2);
	dataData2[strlen(dataData2)] = '\0';
	MT2D_Container_Load_File_From_Memory(dataName2, (unsigned char*)dataData2, strlen(dataData2) + 1);
	printf("Loaded memory data into container\n");


	ret = MT2D_Container_Count_Files();
	printf("CountEmpty %d = 2 ? OK\n", ret);

	free(data);
	printf("data released OK\n");

	int id2 = MT2D_Container_Get_FileId(dataName);
	data = MT2D_Container_Get_Data(id2, false);
	printf("got\n%s\nexpected\ndata data and more data!\n", data);

	/*Create an empty Container*/
	MT2D_FILE* file = MT2D_FILE_OPEN("tmp.mt2", "wb");
	if (file) {
		MT2D_FILE_CLOSE(file);
	}
	printf("Empty container created\n");

	char consttmp2[] = "tmp.mt2";
	char* tmp2Path = (char*)malloc((strlen(consttmp2) + 1) * sizeof(char));
	strcpy(tmp2Path, consttmp2);
	MT2D_Container_Load(tmp2Path);
	printf("Container tmp.mt2 loaded\n");

	ret = MT2D_Container_Count_Files();
	printf("total files loaded = %d OK\n", ret);

	for (int i = 0; i < 1000; i = i + 3)
	{
		MT2D_Container_Password_AddKey((unsigned char)i);
	}
	printf("Password OK\n");

	//hello world
	MT2D_Container_Export_into_Container(id, (char*)"tmp.mt2", false);
	//const data
	printf("file Exported into container tmp.mt2 OK\n");

	//hello world
	MT2D_Container_Export_into_Container(id2, (char*)"tmp.mt2", false);
	//const data
	printf("file Exported into container tmp.mt2 OK\n");

	//hello world
	MT2D_Container_Export_into_Container(MT2D_Container_Get_FileId(dataName2), (char*)"tmp.mt2", true);	//const data
	printf("Encrypted file Exported into container tmp.mt2 OK\n");

	MT2D_Container_Clear();
	printf("Containers Cleared OK\n");

	ret = MT2D_Container_Count_Files();
	printf("CountEmpty %d = 0 ? OK\n", ret);

	MT2D_Container_Load((char*)"tmp.mt2");
	printf("Container tmp.mt2 Loaded OK\n");

	ret = MT2D_Container_Count_Files();
	printf("CountEmpty %d = 3 ? OK\n", ret);

	int id3 = MT2D_Container_Get_FileId((char*)"helloworld.txt");
	printf("tmp2 file1 id %d = id %d ? OK\n", id3, id);

	int length2 = MT2D_Container_Get_FileLength(id3);
	printf("tmp2 file1 length %d = length %d ? OK\n", length2, length);

	int id4 = MT2D_Container_Get_FileId((char*)"data");
	unsigned char* data2 = MT2D_Container_Get_Data(id4, false);
	printf("tmp2 file1 data got\n%s\n should be %s OK \n", data2, data);
	unsigned char* data3 = MT2D_Container_Get_Data(id3, false);
	printf("tmp2 file2 got \n%s\n should be Hi world OK \n", data3);
	unsigned char* data4 = MT2D_Container_Get_Data(MT2D_Container_Get_FileId((char*)"data2"), false);
	printf("tmp2 file3 got \n%s OK \n", data4);
	unsigned char* data5 = MT2D_Container_Get_Data(MT2D_Container_Get_FileId((char*)"data2"), true);
	printf("tmp2 file3 decrypted got \n%s\n should be \ndata data and more data! OK \n", data5);

	printf("MT2D CONTAINER BASIC TEST PASSED!\n");

	return 1;
}