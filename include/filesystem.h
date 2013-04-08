#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#define MAXPATH 256
#define MAXNAME 26

#define NFSTYPES 3

#define FSTYPE_NATIVE	1
#define FSTYPE_DEV		2
#define FSTYPE_THREAD	3

#define ERR_REPARSE_PATH	-100
#define ERR_FILE_NOT_FOUND	-101
#define ERR_ACCESS_DENIED	-102

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
	int (*getInfo)(ObjectHeader* obj);
	int (*openObj)(ObjectHeader* obj, char* path, ObjectHeader** newObj);
	int (*enumEntries)(ObjectHeader* obj, int index, char* buffer, int bufferLen);
	int (*deleteObj)(ObjectHeader* obj);
	int (*close)(ObjectHeader* obj);
} ObjectType;

typedef struct
{
	int objType;
	char objName[MAXNAME];
	int extraBytes;
} ObjectHeader;

typedef struct
{
	
} BuiltInObject;

extern ObjectHeader RootDir;
extern ObjectType ObjectTypes[];
extern List OpenObjects;

#endif