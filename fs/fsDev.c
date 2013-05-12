// fsDev.c
// By Zachary Northrup
//
// Contains methods implementing the built-in /dev filesystem, which provides access to 
// Xinu devices through the filesystem interface.

#include <filesystem.h>
#include <list.h>
#include <hashtable.h>
#include <device.h>

typedef struct
{
	int isRoot;
	int devIndex;
} fsDev_Data;

ObjectHeader* fsDev_CreateHeader(char* path, int isRoot, int devIndex)
{
	ObjectHeader* header = AllocateObjectHeader(sizeof(fsDev_Data));
	fsDev_Data* data = (fsDev_Data*)GetObjectCustomData(header);

	strncpy(header->objName, path, MAXNAME);
	data->isRoot = isRoot;
	data->devIndex = devIndex;

	return header;
}

int fsDev_getInfo(ObjectHeader* obj, ObjectInfo* info)
{
	*info = (ObjectInfo) {0};
	return SUCCESS;
}

int fsDev_openObj(ObjectHeader* obj, char* name, ObjectHeader** newObj, FSMODE mode, FSACCESS access)
{
	*newObj = NULL;

	if ((mode & FSMODE_BASIC_MASK) == FSMODE_CREATENEW)
		return ERR_ACCESS_DENIED;

	fsDev_Data* data = GetObjectCustomData(obj);
	if (!data->isRoot)
		return ERR_FILE_NOT_FOUND;

	syscall deviceIndex = getdev(name);
	if (deviceIndex < 0)
		return ERR_FILE_NOT_FOUND;

	syscall err = open(deviceIndex);
	if (err != OK)
		return err;

	*newObj = fsDev_CreateHeader(name, FALSE, deviceIndex);

	return SUCCESS;
}

int fsDev_enumEntries(ObjectHeader* obj, int index, char* buffer)
{
	fsDev_Data* data = (fsDev_Data*)GetObjectCustomData(obj);

	if (!data->isRoot || index >= NDEVS)
		return ERR_NO_MORE_ENTRIES;

	char* namePtr = devtab[index].name;
	int i = 0;
	while (*namePtr && i <= MAXNAME)
	{
		if (i == MAXNAME || *namePtr < ' ' || namePtr > '~')
		{
			kprintf("Device %d is invalid!\r\n", index);
			strncpy(buffer, "invalid", MAXNAME);
			return SUCCESS;
		}
		namePtr++;
		i++;
	}

	strncpy(buffer, devtab[index].name, MAXNAME);

	return SUCCESS;
}

int fsDev_readObj(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags)
{
	return ERR_ACCESS_DENIED;
}

int fsDev_writeObj(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags)
{
	return ERR_ACCESS_DENIED;
}

int fsDev_deleteObj(ObjectHeader* obj)
{
	return ERR_ACCESS_DENIED;
}

int fsDev_close(ObjectHeader* obj)
{
	return SUCCESS;
}

int fsDev_initRoot(ObjectHeader** newRoot, void* mountArg)
{
	*newRoot = fsDev_CreateHeader("dev", TRUE, -1);
	return SUCCESS;
}

void fsDev_init(void)
{
	ObjectType type =
	{
		.typeId = FSTYPE_DEV,
		.initRoot = fsDev_initRoot,
		.getInfo = fsDev_getInfo,
		.openObj = fsDev_openObj,
		.enumEntries = fsDev_enumEntries,
		.deleteObj = fsDev_deleteObj,
		.close = fsDev_close,
		.readObj = fsDev_readObj,
		.writeObj = feDev_writeObj
	};
	AddObjectType(&type);
}