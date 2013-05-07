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
	fsDev_Data* data = GetObjectCustomData(header);

	strncpy(header->objName, path, MAXNAME);
	data->isRoot = isRoot;
	data->devIndex = devIndex;

	return header;
}

int fsDev_getInfo(ObjectHeader* obj, ObjectInfo* info)
{
	*info = (ObjectInfo) {0};
}

int fsDev_openObj(ObjectHeader* obj, char* name, ObjectHeader** newObj, FSMODE mode, FSACCESS access)
{
	if ((mode & FSMODE_BASIC_MASK) == FSMODE_CREATENEW)
		return ERR_ACCESS_DENIED;

	fsDev_Data* data = GetObjectCustomData(obj);

}

int fsDev_enumEntries(ObjectHeader* obj, int index, char* buffer)
{

}

int fsDev_readObj(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags)
{

}

int fsDev_writeObj(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags)
{

}

int fsDev_deleteObj(ObjectHeader* obj)
{

}

int fsDev_close(ObjectHeader* obj)
{

}