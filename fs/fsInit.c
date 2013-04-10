// Xinu Filesystem Management
// By Zachary Northrup

#include <device.h>
#include <filesystem.h>

#define ARRAYSIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

ObjectHeader RootDir;
ObjectType ObjectTypes[NFSTYPES];
HashTable OpenHandles;

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
	ObjectHeader* ret = malloc(sizeof(ObjectHeader) + extraBytes);
	ret->extraBytes = extraBytes;
}

void* GetObjectCustomData(ObjectHeader* header)
{
	return header + 1;
}

ObjectHeader* fsNative_CreateHeader(char* path)
{
	ObjectHeader* header = AllocateObjectHeader(sizeof(fsNative_Dir));
	fsNative_Dir* dir = GetObjectCustomData(header);
	ListInit(dir->children);
	return header;
}

int fsNative_getInfo(ObjectHeader* obj, ObjectInfo* info)
{
	*info = {};
}

int fsNative_openObj(ObjectHeader* obj, char* path, ObjectHeader** newObj)
{
	fsNative_Dir dir = GetObjectCustomData(obj);
	for (int i = 0; i < dir->children.count; i++)
	{
		ObjectHeader* child;
		ListGet(dir->children, i, child);
		child->
	}
}

int fsNative_enumEntries(ObjectHeader* obj, int index, int* isDir, char* buffer, int bufferLen)
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
	memset(ObjectTypes, 0, ARRAYSIZE(ObjectTypes));
	HashInit(&OpenHandles);

	// Register the native VFS object type
	ObjectType type = 
	{
		typeId = FSTYPE_NATIVE,
		getInfo = fsNative_getInfo,
		openObj = fsNative_openObject,
		enumEntries = fsNative_enumEntries,
		deleteObj = fsNative_deleteObj,
		close = fsNative_close
	};
	AddObjectType(&type);

	// Create the root directory
	
}