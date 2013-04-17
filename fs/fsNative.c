#include <filesystem.h>
#include <list.h>
#include <hashtable.h>

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