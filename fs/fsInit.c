// Xinu Filesystem Management
// By Zachary Northrup

#include <device.h>
#include <filesystem.h>

ObjectHeader RootDir;
ObjectType ObjectTypes[NFSTYPES];

void AddObjectType(ObjectType* type)
{
	ObjectType* newType = malloc(sizeof(ObjectType));
	*newType = *type;
	ListAdd(&ObjectTypes, newType);
}

errcode OpenObject(char* path, ObjectHeader** newObj)
{
	char pathCopy[MAXPATH];
	strcpy(pathCopy, path);


}

ObjectHeader* AllocateObjectHeader(int extraBytes)
{

}

int fsNative_getInfo(ObjectHeader* obj)
{

}

int fsNative_openObj(ObjectHeader* obj, char* path, ObjectHeader** newObj)
{

}

int fsNative_enumEntries(ObjectHeader* obj, int index, char* buffer, int bufferLen)
{

}

int fsNative_deleteObj(ObjectHeader* obj)
{

}

int fsNative_close(ObjectHeader* obj)
{

}

void fsInit()
{
	ListInit(&OpenObjects);

	ObjectType type = 
	{
		typeId = FSTYPE_NATIVE,
		getInfo = 
}