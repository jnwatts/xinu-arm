#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "hashtable.h"
#include "list.h"

#define PATH_SEPARATOR '/'

// Comment out this define to enable case sensitivity
#define CASE_INSENSITIVE
typedef enum
{
	FSTYPE_NATIVE,
	FSTYPE_DEV,
	FSTYPE_THREAD,

	NFSTYPES
} FSTYPE;

#define MAXPATH 128
#define MAXNAME 32

#define ERR_REPARSE_PATH	-100
#define ERR_FILE_NOT_FOUND	-101
#define ERR_ACCESS_DENIED	-102
#define ERR_NO_MORE_ENTRIES	-103
#define ERR_FILE_ALREADY_EXISTS	-104
#define ERR_DIR_NOT_EMPTY	-105
#define ERR_INVALID_FS_TYPE	-106
#define ERR_INVALID_NAME	-107
#define ERR_UNKNOWN			-108

typedef int fshandle;

typedef enum
{
	FSMODE_OPEN = 0,
	FSMODE_OPENORCREATE = 1,
	FSMODE_CREATENEW = 2,

	// OR with this flag to specify the creation of a directory rather than a file
	FSMODE_DIR = 0x100,

	FSMODE_BASIC_MASK = 0x3
} FSMODE;

typedef enum
{
	FSACCESS_INFO = 0,
	FSACCESS_READ = 1,
	FSACCESS_WRITE = 2,
	FSACCESS_READWRITE = FSACCESS_READ | FSACCESS_WRITE
} FSACCESS;

// Contains information about an object, as returned by a file system driver
typedef struct
{
	long creationDate;
	long lastOpenDate;
	long lastWriteDate;
} ObjectInfo;

// The most important structure in the filesystem subsystem, represents an FS object of some kind
typedef struct
{
	int objType;
	char objName[MAXNAME];
	int refCount;
	int extraBytes;
	void* extraData;
} ObjectHeader;

typedef struct
{
	int typeId;

	int (*initRoot)(ObjectHeader** root, void* mountArg);

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

	// Requests that the filesystem create a mount point for a given object
	int (*mountObj)(ObjectHeader* obj, ObjectHeader* mountedObj);
} ObjectType;

// Opens or creates a file in the filesystem
// - openedHandle - a pointer to a location in which to place the handle to the opened file
errcode CreateFile(char* path, fshandle* openedHandle, FSMODE mode, FSACCESS access);
errcode CloseAndDeleteFile(fshandle handle);
errcode CloseFile(fshandle handle);
errcode DeleteFile(char* path);
errcode ReadFile(fshandle handle, char* buffer, int len);
errcode WriteFile(fshandle handle, char* buffer, int len);
errcode EnumFiles(fshandle handle, int index, char* buffer);
errcode ChangeWorkingDirectory(char* path);
char* GetWorkingDirectory(void);

// Internal filesystem API
void fsInit(void);
fshandle CreateHandle(ObjectHeader* header);
errcode OpenObject(char* path, char* actualPath, ObjectHeader** newObj, FSMODE mode, FSACCESS access);
void AddObjectType(ObjectType* type);
errcode CloseObject(ObjectHeader* header);

// Internal native FS functions
ObjectHeader* fsNative_CreateHeader(char* path);
int fsNative_getInfo(ObjectHeader* obj, ObjectInfo* info);
int fsNative_openObj(ObjectHeader* obj, char* path, ObjectHeader** newObj, FSMODE mode, FSACCESS access);
int fsNative_enumEntries(ObjectHeader* obj, int index, char* buffer);
int fsNative_deleteObj(ObjectHeader* obj);
int fsNative_close(ObjectHeader* obj);
int fsNative_readObj(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags);
int fsNative_writeObj(ObjectHeader* obj, fileptr position, char* buffer, int len, int flags);
int fsNative_mountObj(ObjectHeader* obj, ObjectHeader* mountedObj);

// Internal dev FS functions
void fsDev_init(void);

// Zeroing malloc
void* zmalloc(size_t size);

extern ObjectType ObjectTypes[];
extern List OpenObjects;

#endif