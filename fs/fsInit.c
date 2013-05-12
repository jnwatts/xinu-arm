// Xinu Filesystem Management
// By Zachary Northrup

#include <stdlib.h>
#include <device.h>
#include <thread.h>
#include <filesystem.h>
#include <string.h>

#define ARRAYSIZE(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

ObjectHeader* RootDir;
ObjectType ObjectTypes[NFSTYPES];
HashTable OpenHandles;
fshandle NextHandle = 1;

void* zmalloc(size_t size)
{
	void* ret = malloc(size);
	if (!ret)
		kprintf("OUT OF MEMORY?!?!?! BEEP BOOP EXPLODE\n");
	memset(ret, 0, size);
	return ret;
}

fshandle CreateHandle(ObjectHeader* header)
{
	fshandle handle = NextHandle++;
	HashPut(&OpenHandles, handle, (void*)header);
	return handle;
}

errcode CloseFile(fshandle handle)
{
	//kprintf("CloseFile(%d)\n", handle);

	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, (void**)&header);
	if (err < 0 || !header)
		return err;

	HashRemove(&OpenHandles, handle);

	return CloseObject(header);
}

errcode CloseObject(ObjectHeader* header)
{
	//kprintf("CloseObject(%d)\n", header);

	if (!header)
		return SUCCESS;

	errcode err = ObjectTypes[header->objType].close(header);

	// Decrement the reference count and delete the object if it isn't referenced by anyone
	if (--header->refCount <= 0)
	{
		free(header);
	}

	return err;
}

errcode CreateFile(char* path, fshandle* openedHandle, FSMODE mode, FSACCESS access)
{
	//kprintf("CreateFile(%s, _, mode: %d, access: %d)\n", path, mode, access);

	ObjectHeader* header = NULL;
	errcode result = OpenObject(path, NULL, &header, mode, access);
	if (result < 0)
		return result;

	*openedHandle = CreateHandle(header);

	return result;
}

errcode DeleteFile(char* path)
{
	ObjectHeader* header = NULL;
	errcode err = OpenObject(path, NULL, &header, FSMODE_OPEN, FSACCESS_WRITE);
	if (err < 0 || !header)
		return err;

	err = ObjectTypes[header->objType].deleteObj(header);

	return CloseObject(header);
}

errcode CloseAndDeleteFile(fshandle handle)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, (void**)&header);
	if (err < 0 || !header)
		return err;

	err = ObjectTypes[header->objType].deleteObj(header);

	return CloseObject(header);
}

errcode ReadFile(fshandle handle, char* buffer, int len)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, &header);
	if (err < 0 || !header)
		return err;
	return SUCCESS;
}

errcode WriteFile(fshandle handle, char* buffer, int len)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, &header);
	if (err < 0 || !header)
		return err;
	return SUCCESS;
}

errcode EnumFiles(fshandle handle, int index, char* buffer)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, &header);
	if (err < 0 || !header)
		return err;

	err = ObjectTypes[header->objType].enumEntries(header, index, buffer);
	return err;
}

static void PreprocessPath(char*);

errcode MountFileSystem(char* mountPoint, int typeId, void* mountArg)
{
	//kprintf("MountFileSystem(%s, %d, %d)\r\n", mountPoint, typeId, (int)mountArg);

	if (typeId >= NFSTYPES)
		return ERR_INVALID_FS_TYPE;

	char* pathCopy = (char*)zmalloc(MAXPATH + 1);
	strncpy(pathCopy, mountPoint, MAXPATH);
	
	// Tidy up the path, removing dots, trailing slashes, etc.
	PreprocessPath(pathCopy);
	
	// Reject invalid paths
	if (pathCopy[0] == 0)
	{
		kprintf("Invalid path\r\n");
		free(pathCopy);
		return ERR_FILE_NOT_FOUND;
	}

	// Separate the directory path from the mount point name
	char* lastSlash = strrchr(pathCopy, PATH_SEPARATOR);
	*lastSlash = 0;
	lastSlash++;

	//kprintf("Mounting file system inside \"%s\" with name \"%s\"\r\n", pathCopy, lastSlash);

	// Open the folder that will contain the mount point
	ObjectHeader* enclosingDir = NULL;
	errcode err = OpenObject(*pathCopy == 0 ? "/" : pathCopy, NULL, &enclosingDir, (FSMODE)(FSMODE_OPEN | FSMODE_DIR), FSACCESS_INFO);
	if (err || !enclosingDir)
	{
		kprintf("Failed to open enclosing directory\r\n");
		free(pathCopy);
		return err ? err : ERR_FILE_NOT_FOUND;
	}

	//kprintf("Creating root object\r\n");

	// Create the root of the new filesystem
	ObjectHeader* newRoot = NULL;
	err = ObjectTypes[typeId].initRoot(&newRoot, mountArg);
	if (err || !newRoot)
	{
		kprintf("Failed to create root\r\n");
		CloseObject(enclosingDir);
		free(pathCopy);
		return err ? err : ERR_UNKNOWN;
	}

	//kprintf("Mounting into enclosing FS\r\n");

	// Assign the new object a place in the enclosing filesystem
	err = ObjectTypes[enclosingDir->objType].mountObj(enclosingDir, newRoot);
	if (err)
	{
		kprintf("Failed to mount into enclosing FS\r\n");
		CloseObject(newRoot);
	}

	CloseObject(enclosingDir);
	free(pathCopy);
	return err;
}

void AddObjectType(ObjectType* type)
{
	ObjectTypes[type->typeId] = *type;
}

static void StrToLower(char* str)
{
	while (*str)
	{
		if (*str >= 'A' && *str <= 'Z')
			*str = (char)((int)*str + (int)'a' - (int)'A');
		str++;
	}
}

static void StrToUpper(char* str)
{
	while (*str)
	{
		if (*str >= 'a' && *str <= 'z')
			*str = (char)((int)*str + (int)'A' - (int)'a');
		str++;
	}
}

static int StrIndexOf(char* str, char chr)
{
	int result = 0;
	while (*str && *str != chr)
	{
		str++;
		result++;
	}
	return result;
}

// Cleans up a path by eliminating dots, double dots, trailing slashes, and double slashes
static void PreprocessPath(char* path)
{
	// segStart always moves forward faster than dest
	char *dest, *segStart;
	int segLength = 0;
	dest = segStart = path;

	if (path[0] != PATH_SEPARATOR)
	{
		path[0] = 0;
		return;
	}
	
	// Advance beyond the first slash
	segStart++;

	while (*segStart)
	{
		// Find the next slash
		segLength = StrIndexOf(segStart, PATH_SEPARATOR);

		// Remove double-slashes
		if (segLength == 0)
		{
			segStart++;
		}
		// Handle single dots by throwing them away
		else if (segLength == 1 && !memcmp(segStart, ".", 1))
		{
			segStart++;
		}
		// Handle double dots by moving back to the last slash
		else if (segLength == 2 && !memcmp(segStart, "..", 2))
		{
			// Find the last slash
			char* lastSlash = dest - 1;
			while (lastSlash >= path && *lastSlash != PATH_SEPARATOR)
			{
				lastSlash--;
			}

			// If we hit the beginning of the buffer, the path is invalid
			if (*lastSlash != PATH_SEPARATOR)
			{
				*path = 0;
				return;
			}

			dest = lastSlash + 1;
			segStart += 2;
		}
		// Handle normal path segments
		else
		{
			*dest++ = PATH_SEPARATOR;
			memmove(dest, segStart, segLength);
			dest += segLength;
			segStart += segLength;
		}
	}
	if (dest == path)
		dest++;
	*dest = 0;

	// Remove trailing slashes
	if ((dest - path) > 1 && *(dest - 1) == PATH_SEPARATOR)
	{
		*(dest - 1) = 0;
		dest--;
	}
}

// Determines whether enumeration functions are called to validate sub-object naming
//#define ENUM_TO_OPEN

errcode OpenObject(char* path, char* actualPath, ObjectHeader** newObj, FSMODE mode, FSACCESS access)
{
	//kprintf("OpenObject(%s, _, _, mode: %d, access: %d)\n", path, mode, access);

	char* pathCopy = zmalloc(MAXPATH + 1);
	pathCopy[0] = 0;
	errcode err = SUCCESS;

	*newObj = NULL;

	// Copy the path onto the stack, prepending the working directory for relative paths
	if (path[0] != PATH_SEPARATOR)
	{
		strncpy(pathCopy, GetWorkingDirectory(), MAXPATH);
		strncat(pathCopy, "/", MAXPATH);
		strncat(pathCopy, path, MAXPATH);
		//kprintf("Built initial path: %s\n", pathCopy);
	}
	else
	{
		strncpy(pathCopy, path, MAXPATH);
	}

	// Put the path into the nice form "/thing/thing/thing/file" without dots or double-slashes
	PreprocessPath(pathCopy);

	// Preprocessing can indicate an invalid path by setting the first character to null
	// This also handles zero-length paths
	if (!pathCopy[0])
	{
		free(pathCopy);
		return ERR_FILE_NOT_FOUND;
	}

	//kprintf("Processed path: %s\n", pathCopy);

	if (actualPath)
		strncpy(actualPath, pathCopy, MAXPATH);

#ifdef CASE_INSENSITIVE
	// Compare all strings as lower-case for case insensitivity
	StrToLower(pathCopy);
#endif

	// Find the object referred to by the path
	ObjectHeader* currObj = RootDir;
	ObjectType* currObjType = &ObjectTypes[currObj->objType];

	// Fake "opening" the root object, to allow it to be closed
	currObj->refCount++;

	char* currSeg = pathCopy;
	char compareBuffer[MAXNAME + 1] = {0};
	int segLength;
	while (*currSeg)
	{
		// Advance past the path separator
		currSeg++;

		segLength = StrIndexOf(currSeg, PATH_SEPARATOR);
		if (segLength == 0)
		{
			//kprintf("Done with path\n");
			break;
		}
		else if (segLength > MAXNAME)
		{
			err = ERR_INVALID_NAME;
		}
		int hasMoreSegments = currSeg[segLength] == PATH_SEPARATOR;

		//kprintf("Segment: %.*s\n", segLength, currSeg);
		
#ifdef ENUM_TO_OPEN
		int foundName = FALSE;
		int index = 0;
		do
		{
			err = currObjType->enumEntries(currObj, index, compareBuffer);
			if (err == ERR_NO_MORE_ENTRIES)
			{
				err = ERR_FILE_NOT_FOUND;
				break;
			}
			else if (err)
				break;

			if (strnlen(compareBuffer, MAXNAME) == segLength)
			{
#ifdef CASE_INSENSITIVE
				StrToLower(compareBuffer);
#endif

				if (!memcmp(compareBuffer, currSeg, segLength))
					foundName = TRUE;
			}

			index++;
		} while (!foundName);

		if (!foundName)
		{
			err = ERR_FILE_NOT_FOUND;
			break;
		}

#ifdef CASE_INSENSITIVE
		strncpy(compareBuffer, currSeg, segLength);
		compareBuffer[segLength] = 0;
#endif

#else
		// Copy the segment into a buffer as a string
		strncpy(compareBuffer, currSeg, segLength);
		compareBuffer[segLength] = 0;
#endif

		// Open the sub-object
		ObjectHeader* nextObj = NULL;
		err = currObjType->openObj(currObj, compareBuffer, &nextObj, hasMoreSegments ? (FSMODE)(FSMODE_OPEN | FSMODE_DIR) : mode, hasMoreSegments ? FSACCESS_INFO : access);
		if (err || !nextObj)
		{
			err = err ? err : ERR_FILE_NOT_FOUND;
			break;
		}

		// Close the current object and make the new object our current object
		CloseObject(currObj);
		currObj = nextObj;
		currObjType = &ObjectTypes[currObj->objType];

		// Advance to the next path segment
		currSeg += segLength;
	}

	if (err)
	{
		//kprintf("OpenObject returning error %d\n", err);
		if (currObj)
			CloseObject(currObj);
	}
	else
	{
		*newObj = currObj;
	}

	free(pathCopy);
	return err;
}

errcode ChangeWorkingDirectory(char* path)
{
	char* pathCopy = zmalloc(MAXPATH + 1);
	ObjectHeader* header;
	errcode err = OpenObject(path, pathCopy, &header, FSMODE_OPEN | FSMODE_DIR, FSACCESS_READ);
	if (!err)
	{
		strncpy(thrtab[thrcurrent].currdir, pathCopy, MAXPATH);
		CloseObject(header);
	}
	free(pathCopy);
	return err;
}

char* GetWorkingDirectory(void)
{
	return thrtab[thrcurrent].currdir;
}

ObjectHeader* AllocateObjectHeader(int extraBytes)
{
	ObjectHeader* ret = zmalloc(sizeof(ObjectHeader));

	ret->extraData = extraBytes ? zmalloc(extraBytes) : NULL;
	ret->extraBytes = extraBytes;
	return ret;
}

void* GetObjectCustomData(ObjectHeader* header)
{
	return header->extraData;
}

void fsInit(void)
{
	memset(ObjectTypes, 0, ARRAYSIZE(ObjectTypes));
	HashInit(&OpenHandles);

	// Register the native VFS object type
	ObjectType type = 
	{
		.typeId = FSTYPE_NATIVE,
		.getInfo = fsNative_getInfo,
		.openObj = fsNative_openObj,
		.enumEntries = fsNative_enumEntries,
		.deleteObj = fsNative_deleteObj,
		.close = fsNative_close,
		.mountObj = fsNative_mountObj
	};
	AddObjectType(&type);

	// Create the root directory
	RootDir = fsNative_CreateHeader("");

	fsDev_init();

	// Mount the list of devices at /dev
	MountFileSystem("/dev", FSTYPE_DEV, NULL);
}
