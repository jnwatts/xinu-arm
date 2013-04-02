
#define MAXPATH 256
#define MAXNAME 26

#define ENTRIESPERLIST 10

typedef struct
{
	int count;
	void* entries[ENTRIESPERLIST];
	ListHeader* next;
} ListHeader;

void ListInit(ListHeader* header)
{
	header->count = 0;
	header->next = NULL;
	header->prev = NULL;
}

void ListAdd(ListHeader* header, void* item)
{
	int count = header->count;
	header->count++;
	while (count >= ENTRIESPERLIST)
	{
		if (header->next == NULL)
		{
			header->next = malloc(sizeof(ListHeader));
			header->next->count = 0;
			header->next->next = NULL;
		}
		count -= ENTRIESPERLIST;
		header = header->next;
	}
	header->entries[count] = item;
}

int ListGet(ListHeader* header, int index, void** item)
{
	while (index >= ENTRIESPERLIST)
	{
		if (!header)
			return SYSERR;

		header = header->next;
		index -= ENTRIESPERLIST;
	}
	return header->entries[index];
}

int ListCount(ListHeader* header)
{
	return header->count;
}

void ListClear(ListHeader* header)
{
	ListHeader* tail = header->next;
	header->count = 0;
	while (tail)
	{
		ListHeader* lastTail = tail;
		tail = tail->next;
		free(lastTail);
	}
}

int ListRemoveAt(ListHeader* header, int index)
{
	int totalCount = header->count;

	// Find the list with the entry
	while (index >= ENTRIESPERLIST)
	{
		header = header->next;
		index -= ENTRIESPERLIST;
		totalCount -= ENTRIESPERLIST;
	}

	// Remove the entry
	do
	{
		memmove(&header->entries[index], &header->entries[index + 1], ENTRIESPERLIST - index - 1);
		index = 0;
		if (header->next)
		{
			header->entries[ENTRIESPERLIST - 1] = header->next->entries[0];
			header = header->next;
		}
		else if (totalCount == 1)
		{
			free(header);
			header = NULL;
		}
		else
			header = NULL;
		
	} while (header);
}

typedef struct
{
	long lastOpenDate;
	long lastWriteDate;
} ObjectInfo;

typedef struct
{
	int (*getInfo)(ObjectHeader* obj);
	int (*openObj)(ObjectHeader* obj, char* path, ObjectHeader** newObj);
	int (*enumEntries)(ObjectHeader* obj, int index, char* buffer, int bufferLen);
	int (*deleteObj)(ObjectHeader* obj);
} ObjectType;

typedef struct
{
	int objType;
	char objName[MAXNAME];
} ObjectHeader;

typedef struct
{
	
} dirent;

typedef struct
{
	void (*enumEntries)(int count);
} vfsdir;

typedef struct
{
	
} vfsdev;