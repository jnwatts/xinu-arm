#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "hashtable.h"
#include "list.h"

#define MAXPATH 256
#define MAXNAME 26

enum FSTYPE
{
	FSTYPE_NATIVE,
	FSTYPE_DEV,
	FSTYPE_THREAD,

	NFSTYPES
};

#define ERR_REPARSE_PATH	-100
#define ERR_FILE_NOT_FOUND	-101
#define ERR_ACCESS_DENIED	-102

typedef int fshandle;

enum FSMODE
{
	FSMODE_OPEN = 0,
	FSMODE_OPENORCREATE = 1,
	FSMODE_CREATENEW = 2
};

enum FSACCESS
{
	FSACCESS_INFO = 0,
	FSACCESS_READ = 1,
	FSACCESS_WRITE = 2
};

// Contains information about an object, as returned by a file system driver
typedef struct
{
	long creationDate;
	long lastOpenDate;
	long lastWriteDate;
} ObjectInfo;

typedef struct
{
	int typeId;
	int (*getInfo)(ObjectHeader* obj, ObjectInfo* info);
	int (*openObj)(ObjectHeader* obj, char* path, ObjectHeader** newObj, FSMODE mode, FSACCESS access);
	int (*enumEntries)(ObjectHeader* obj, int index, int* isDir, char* buffer, int bufferLen);
	int (*readObj)(ObjectHeader* obj, char* buffer, int len, int flags);
	int (*writeObj)(ObjectHeader* obj, char* buffer, int len, int flags);
	int (*deleteObj)(ObjectHeader* obj);
	int (*close)(ObjectHeader* obj);
} ObjectType;

typedef struct
{
	int objType;
	char objName[MAXNAME];
	int refCount;
	int extraBytes;
} ObjectHeader;

typedef struct
{
	List children;
} fsNative_Dir;

// Opens or creates a file in the filesystem
// - openedHandle - a pointer to a location in which to place the handle to the opened file
errcode CreateFile(char* path, fshandle* openedHandle, FSMODE mode, FSACCESS access);
errcode CloseFile(fshandle handle);
errcode DeleteFile(char* path);
errcode ReadFile(fshandle handle, char* buffer, int len);
errcode WriteFile(fshandle handle, char* buffer, int len);

extern ObjectHeader RootDir;
extern ObjectType ObjectTypes[];
extern List OpenObjects;

#endif