#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#define MAXPATH 256
#define MAXNAME 26

#define NTYPES 3

#define REPARSE_PATH

typedef struct
{
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