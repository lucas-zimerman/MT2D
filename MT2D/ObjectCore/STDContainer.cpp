#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MT2D_USING_CONTAINER

#include <MT2D/ObjectCore/Object.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <MT2D\MT2D_Debug.h>
/*
	VAR[0] = Sound Name
	VAR[1] bool decrypt
*/
void Cscript_PlaySoundFromContainer(Object* object, MT2D_VAR** SoundName) 
{
	MT2D_Ide_Printf("Playing container sound");
	MT2D_Ide_Printf(SoundName[0]->Name);
	if (SoundName[0]->Data == NULL) {
		int id = MT2D_Container_Get_FileId(SoundName[0]->Name);
		int size = MT2D_Container_Get_FileLength(id);
		unsigned char* data = MT2D_Container_Get_Data(id, MT2D_Object_VAR_GetBool(SoundName[1]));
		ContainerData *cdata = (ContainerData*)malloc(sizeof(ContainerData));
		cdata->Data = data;
		cdata->Length = size;
		cdata->Id = id;
		SoundName[0]->Data = cdata;
	}
	MT2D_PlayAudioFromMemory(((ContainerData*)(SoundName[0]->Data))->Data, ((ContainerData*)(SoundName[0]->Data))->Length, true);
}

/*
	VAR[0] = Music Name
	VAR[1] bool decrypt
*/
void Cscript_PlayMusicFromContainer(Object* object, MT2D_VAR** MusicName) {
	if (MusicName[0]->Data == NULL) {
		int id = MT2D_Container_Get_FileId(MusicName[0]->Name);
		int size = MT2D_Container_Get_FileLength(id);
		unsigned char* data = MT2D_Container_Get_Data(id, MT2D_Object_VAR_GetBool(MusicName[1]));
		ContainerData* cdata = (ContainerData*)malloc(sizeof(ContainerData));
		cdata->Data = data;
		cdata->Id = id;
		cdata->Length = size;
		MusicName[0]->Data = cdata;
	}
	MT2D_PlayMusicFromMemory(((ContainerData*)(MusicName[0]->Data))->Data, ((ContainerData*)(MusicName[0]->Data))->Length, true);
}


#endif