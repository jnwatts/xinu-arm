#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "hashtable.h"
#include "list.h"

#define PATH_SEPARATOR '/'

// Comment out this define to enable case sensitivity
#define CASE_INSENSITIVE

#define MAXPATH 256
#define MAXNAME 26

#define NFSTYPES 3

#define FSTYPE_NATIVE	1
#define FSTYPE_DEV		2
#define FSTYPE_THREAD	3

#define ERR_REPARSE_PATH	-100
#define ERR_FILE_NOT_FOUND	-101
#define ERR_ACCESS_DENIED	-102
#define ERR_NO_MORE_ENTRIES	-103

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

	// Gets information about the given object.
	int (*getInfo)(ObjectHeader* obj, ObjectInfo* info);

	// Opens an object with the specified name.
	int (*openObj)(ObjectHeader* obj, char* name, ObjectHeader** newObj, FSMODE mode, FSACCESS access);

	// Enumerates through the sub-objects of this object. 
	//   Index is the index of the sub-object whose name to place into the buffer. 
	//   The buffer should be of length MAXNAME.
	//   Returns ERROR_NO_MORE_ENTRIES for indices that are out of range.
	int (*enumEntries)(ObjectHeader* obj, int index, char* buffer);

	// Reads the object at the given position, or reads the next bytes if the object is not a block device.
	//   Returns the number of bytes read or zero if there are no more bytes to read.
	int (*readObj)(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags);

	// Writes the object at the given position, or writes the next bytes if the object is not a block device.
	int (*writeObj)(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags);

	// Notifies the type manager that all representations of the object should be deleted.
	int (*deleteObj)(ObjectHeader* obj);

	// Notifies the type manager that the object has been closed.
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