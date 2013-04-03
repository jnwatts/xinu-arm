// Xinu Filesystem Management
// By Zachary Northrup

#include <device.h>
#include <filesystem.h>

ObjectHeader RootDir;
List ObjectTypes;

void AddObjectType(ObjectType* type)
{
	ObjectType* newType = malloc(sizeof(ObjectType));
	*newType = *type;
	ListAdd(&ObjectTypes, newType);
}

int (*getInfo)(ObjectHeader* obj);
int (*openObj)(ObjectHeader* obj, char* path, char* reparsePath, ObjectHeader** newObj);
int (*enumEntries)(ObjectHeader* obj, int index, char* buffer, int bufferLen);
int (*deleteObj)(ObjectHeader* obj);
int (*close)(ObjectHeader* obj);

void OpenObject(char* path, ObjectHeader** newObj)
{
	char pathCopy[MAXPATH];
	strcpy(pathCopy, path);


}

void fsInit()
{
	ListInit(&OpenObjects);
}