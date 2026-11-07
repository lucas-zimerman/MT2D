#include <MT2D/MT2D_Terminal_Define.h>
#ifdef MT2D_USING_CONTAINER

#include <stdlib.h>
#include <MT2D/ObjectCore/Object.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/Audio/MT2D_Audio_core.h>
#include <MT2D/MT2D_Debug.h>

/*
	The loaded files are cached by container id, so each sound/music is read (and decrypted) only once.
	The var is not used as the cache anymore: its Data is the text of a VAR_STRING.
*/
struct ContainerCacheNode {
	ContainerData Data;
	ContainerCacheNode *Next;
};
static ContainerCacheNode *ContainerCache = NULL;

static ContainerData *Cached_Container_Data(const char *path, bool decrypt)
{
	int id = MT2D_Container_Get_FileId((char*)path);
	ContainerCacheNode *node = ContainerCache;
	while (node != NULL) {
		if (node->Data.Id == id) {
			return &node->Data;
		}
		node = node->Next;
	}
	node = (ContainerCacheNode*)malloc(sizeof(ContainerCacheNode));
	node->Data.Length = MT2D_Container_Get_FileLength(id);
	node->Data.Data = MT2D_Container_Get_Data(id, decrypt);
	node->Data.Id = id;
	node->Next = ContainerCache;
	ContainerCache = node;
	return &node->Data;
}

/*
	VAR[0] = Sound file, the text of a VAR_STRING var (or the Name of a var when it is short)
	VAR[1] bool decrypt
*/
void Cscript_PlaySoundFromContainer(Object* object, MT2D_VAR** SoundName) 
{
	const char *path = MT2D_Object_VAR_GetPath(SoundName[0]);
	MT2D_Ide_Printf("Playing container sound");
	MT2D_Ide_Printf((char*)path);
	ContainerData *cdata = Cached_Container_Data(path, MT2D_Object_VAR_GetBool(SoundName[1]));
	MT2D_PlayAudioFromMemory(cdata->Data, cdata->Length, true);
}

/*
	VAR[0] = Music file, the text of a VAR_STRING var (or the Name of a var when it is short)
	VAR[1] bool decrypt
*/
void Cscript_PlayMusicFromContainer(Object* object, MT2D_VAR** MusicName) {
	ContainerData *cdata = Cached_Container_Data(MT2D_Object_VAR_GetPath(MusicName[0]), MT2D_Object_VAR_GetBool(MusicName[1]));
	MT2D_PlayMusicFromMemory(cdata->Data, cdata->Length, true);
}


#endif
