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
fshandle NextHandle;

fshandle CreateHandle(ObjectHeader* header)
{
	fshandle handle = NextHandle++;
	HashPut(&OpenHandles, handle, (void*)header);
	return handle;
}

errcode CloseFile(fshandle handle)
{
	kprintf("CloseFile(%d)\n", handle);

	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, (void**)&header);
	if (err < 0 || !header)
		return err;

	HashRemove(&OpenHandles, handle);

	return CloseObject(header);
}

errcode CloseObject(ObjectHeader* header)
{
	kprintf("CloseObject(%d)\n", header);

	if (!header)
		return OK;

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
	kprintf("CreateFile(%s, _, mode: %d, access: %d)\n", path, mode, access);

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
	return OK;
}

errcode WriteFile(fshandle handle, char* buffer, int len)
{
	ObjectHeader* header = NULL;
	errcode err = HashGet(&OpenHandles, handle, &header);
	if (err < 0 || !header)
		return err;
	return OK;
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

void AddObjectType(ObjectType* type)
{
	/*ObjectType* newType = malloc(sizeof(ObjectType));
	*newType = *type;
	ListAdd(&ObjectTypes, newType);*/
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

// Cleans up a path by eliminating dots, double dots, and double slashes
static void PreprocessPath(char* path)
{
	// segStart always moves forward faster than dest
	char *dest, *segStart;
	int segLength = 0;
	dest = segStart = path;

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
	*dest = 0;
}

// Determines whether enumeration functions are called to validate sub-object naming
//#define ENUM_TO_OPEN

errcode OpenObject(char* path, char* actualPath, ObjectHeader** newObj, FSMODE mode, FSACCESS access)
{
	kprintf("OpenObject(%s, _, _, mode: %d, access: %d)\n", path, mode, access);

	char pathCopy[MAXPATH + 1] = {0};
	errcode err = OK;

	*newObj = NULL;

	// Copy the path onto the stack, prepending the working directory for relative paths
	if (path[0] != PATH_SEPARATOR)
	{
		strncpy(pathCopy, thrtab[thrcurrent].currdir, MAXPATH);
		strncat(pathCopy, path, MAXPATH);
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
		return ERR_FILE_NOT_FOUND;

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
	char compareBuffer[MAXNAME + 1];
	int segLength;
	while (*currSeg)
	{
		// Advance past the path separator
		currSeg++;

		segLength = StrIndexOf(currSeg, PATH_SEPARATOR);
		if (segLength == 0)
		{
			kprintf("done with path\n");
			break;
		}

		kprintf("Segment: %.*s\n", segLength, currSeg);
		
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
#else
		// Copy the segment into a buffer as a string
		strncpy(compareBuffer, currSeg, segLength);
#endif

		// Open the sub-object
		ObjectHeader* nextObj = NULL;
		err = currObjType->openObj(currObj, compareBuffer, &nextObj, mode, access);
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
		if (currObj)
			CloseObject(currObj);
	}
	else
	{
		*newObj = currObj;
	}

	return err;
}

errcode ChangeWorkingDirectory(char* path)
{
	char* pathCopy = malloc(MAXPATH + 1);
	ObjectHeader* header;
	errcode err = OpenObject(path, pathCopy, &header, FSMODE_OPEN | FSMODE_DIR, FSACCESS_READ);
	if (!err)
	{
		strncpy(thrtab[thrcurrent].currdir, pathCopy, MAXPATH);
		CloseObject(header);
	}

	return err;
}

ObjectHeader* AllocateObjectHeader(int extraBytes)
{
	ObjectHeader* ret = malloc(sizeof(ObjectHeader) + extraBytes);
	ret->extraBytes = extraBytes;
}

void* GetObjectCustomData(ObjectHeader* header)
{
	return header + 1;
}

void fsInit()
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
		.close = fsNative_close
	};
	AddObjectType(&type);

	// Create the root directory
	RootDir = fsNative_CreateHeader("");
}