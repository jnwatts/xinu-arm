// Xinu Filesystem Management
// By Zachary Northrup

#include <device.h>
#include <filesystem.h>

#define ARRAYSIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

ObjectHeader RootDir;
ObjectType ObjectTypes[NFSTYPES];
HashTable OpenHandles;
fshandle NextHandle;

fshandle CreateHandle(ObjectHeader* header)
{
	fshandle handle = NextHandle++;
	HashPut(&OpenHandles, handle, (void*)header);
	return handle;
}

errcode CloseFile(fshandle handle)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, (void**)&header);
	if (err < 0 || !header)
		return err;

	err = ObjectTypes[header->objType].close(header);

	// Decrement the reference count and delete the object if it isn't referenced by anyone
	if (--header->refCount <= 0)
	{
		free(header);
	}

	return err;
}

errcode CreateFile(char* path, fshandle* openedHandle, FSMODE mode, FSACCESS access)
{
	ObjectHeader* header = NULL;
	errcode result = OpenObject(path, &header, mode, access);
	if (result < 0)
		return result;

	header->refCount++;
}

errcode DeleteFile(char* path)
{
	ObjectHeader* header = NULL;
	errcode err = OpenObject(path, &header, FSMODE_OPENEXISTING, FSACCESS_WRITE);
	if (err < 0 || !header)
		return err;

	ObjectTypes[header->objType].deleteObj(header);

	if (header->refCount <= 0)
		free(header);

	return OK;
}

errcode ReadFile(fshandle handle, char* buffer, int len)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, &header);
	if (err < 0 || !header)
		return err;

	ObjectTypes[header->objType].read
}

errcode WriteFile(fshandle handle, char* buffer, int len)
{

}

void AddObjectType(ObjectType* type)
{
	ObjectType* newType = malloc(sizeof(ObjectType));
	*newType = *type;
	ListAdd(&ObjectTypes, newType);
}

errcode OpenObject(char* path, ObjectHeader** newObj, FSMODE mode, FSACCESS access)
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