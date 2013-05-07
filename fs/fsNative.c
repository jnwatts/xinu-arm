#include <filesystem.h>
#include <list.h>
#include <hashtable.h>

ObjectHeader* fsNative_CreateHeader(char* path)
{
	ObjectHeader* header = AllocateObjectHeader(sizeof(fsNative_Dir));
	fsNative_Dir* dir = GetObjectCustomData(header);

	header->objType = FSTYPE_NATIVE;
	strncpy(header->objName, path, MAXNAME);
	header->refCount = 1;

	ListInit(&dir->children);
	return header;
}

int fsNative_getInfo(ObjectHeader* obj, ObjectInfo* info)
{
	*info = (ObjectInfo){0};
	return SUCCESS;
}

int fsNative_openObj(ObjectHeader* obj, char* path, ObjectHeader** newObj, FSMODE mode, FSACCESS access)
{
	fsNative_Dir* dir = GetObjectCustomData(obj);
	for (int i = 0; i < dir->children.count; i++)
	{
		ObjectHeader* child;
		ListGet(&dir->children, i, (void**)&child);
		if (!strncmp(child->objName, path, MAXNAME))
		{
			if ((mode & FSMODE_BASIC_MASK) == FSMODE_CREATENEW)
				return ERR_FILE_ALREADY_EXISTS;

			child->refCount++;
			*newObj = child;
			return SUCCESS;
		}
	}

	// Create a new entry if requested
	if ((mode & FSMODE_BASIC_MASK) != FSMODE_OPEN)
	{
		*newObj = fsNative_CreateHeader(path);
		ListAdd(&dir->children, *newObj);
		(*newObj)->refCount++;
		return SUCCESS;
	}

	return ERR_FILE_NOT_FOUND;
}

int fsNative_enumEntries(ObjectHeader* obj, int index, char* buffer)
{
	fsNative_Dir* dir = GetObjectCustomData(obj);
	ObjectHeader* child = NULL;
	if (index >= dir->children.count)
		return ERR_NO_MORE_ENTRIES;

	ListGet(&dir->children, index, (void**)&child);
	strncpy(buffer, child->objName, MAXNAME);
	return SUCCESS;
}

int fsNative_deleteObj(ObjectHeader* obj)
{
	obj->refCount--;
	return SUCCESS;
}

int fsNative_close(ObjectHeader* obj)
{
	return SUCCESS;
}

int fsNative_mountObj(ObjectHeader* obj)
{
	return SUCCESS;
}