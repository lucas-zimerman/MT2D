#ifndef __MT2D_RAWOBJECT__
#define __MT2D_RAWOBJECT__

struct RawObject
{
    char *Name;
    char **Lines;
    int Count;
};

RawObject *Mt2D_RawObject_Create(char *object);
void MT2D_RawObject_Destroy(RawObject *object);

#endif