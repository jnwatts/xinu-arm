#include <filesystem.h>
#include <list.h>
#include <hashtable.h>

ObjectHeader* fsNative_CreateHeader(char* path)
{
	ObjectHeader* header = AllocateObjectHeader(sizeof(fsNative_Dir));
	fsNative_Dir* dir = GetObjectCustomData(header);

	strcpy(header->objName, path);
	header->refCount = 1;

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
		ListGet(dir->children, i, &child);
		if (!strcmp(child->objName, path))
		{
			child->refCount++;
			*newObj = child;
			return OK;
		}
	}
	return ERR_FILE_NOT_FOUND;
}

int fsNative_enumEntries(ObjectHeader* obj, int index, char* buffer)
{
	fsNative_Dir dir = GetObjectCustomData(obj);
	ObjectHeader* child = NULL;
	if (index >= dir->children->count)
		return ERR_NO_MORE_ENTRIES;

	ListGet(dir->children, index, &child);
	strcpy(buffer, child->objName);
	return OK;
}

int fsNative_deleteObj(ObjectHeader* obj)
{
	obj->refCount--;
	return OK;
}

int fsNative_close(ObjectHeader* obj)
{
	return OK;
}