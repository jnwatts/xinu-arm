// Xinu Lists
// Contains an implementation of a useful linked list 
// structure, designed to hold pointer-sized values.

#include "../include/list.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifndef OK
#define OK 0
#endif
#ifndef SYSERR
#define SYSERR -1
#endif

#ifdef __cplusplus
extern "C" {
#endif

void ListInit(List* header)
{
	header->count = 0;
	header->next = NULL;
}

void ListAdd(List* header, void* item)
{
	int count = header->count;
	header->count++;
	while (count >= ENTRIES_PER_LIST)
	{
		if (header->next == NULL)
		{
			header->next = (List*)malloc(sizeof(List));
			header->next->count = 0;
			header->next->next = NULL;
		}
		count -= ENTRIES_PER_LIST;
		header = header->next;
	}
	header->entries[count] = item;
}

void ListInsert(List* header, int index, void* item)
{
	int count = header->count + 1;
	void* displaced = item;

	// Just call add if this index is at the end
	if (index >= count)
	{
		ListAdd(header, item);
		return;
	}

	header->count++;
	while (index >= ENTRIES_PER_LIST)
	{
		if (!header->next)
		{
			header->next = (List*)malloc(sizeof(List));
			header->next->count = 0;
			header->next->next = NULL;
		}
		header = header->next;
		count -= ENTRIES_PER_LIST;
		index -= ENTRIES_PER_LIST;
	}

	while (count > 0)
	{
		// Insert into the first run
		void* newDisplaced = header->entries[ENTRIES_PER_LIST - 1];
		memmove(&header->entries[index + 1], &header->entries[index], sizeof(void*) * (ENTRIES_PER_LIST - index - 1));

		// Displace each run by 1
		header->entries[index] = displaced;
		displaced = newDisplaced;

		index = 0;
		count -= ENTRIES_PER_LIST;

		if (count > 0 && !header->next)
		{
			header->next = (List*)malloc(sizeof(List));
			header->next->count = 0;
			header->next->next = NULL;
		}
		header = header->next;
	}
}

int ListGet(List* header, int index, void** item)
{
	while (index >= ENTRIES_PER_LIST)
	{
		if (!header)
			return SYSERR;

		header = header->next;
		index -= ENTRIES_PER_LIST;
	}
	*item = header->entries[index];
	return OK;
}

int ListCount(List* header)
{
	return header->count;
}

void ListClear(List* header)
{
	List* tail = header->next;
	header->count = 0;
	while (tail)
	{
		List* lastTail = tail;
		tail = tail->next;
		free(lastTail);
	}
}

void ListRemoveAt(List* header, int index)
{
	int totalCount = header->count;

	if (index < 0 || index >= totalCount)
		return;

	header->count--;

	// Find the list with the entry
	while (index >= ENTRIES_PER_LIST)
	{
		header = header->next;
		index -= ENTRIES_PER_LIST;
		totalCount -= ENTRIES_PER_LIST;
	}

	// Remove the entry
	do
	{
		memmove(&header->entries[index], &header->entries[index + 1], sizeof(void*) * (ENTRIES_PER_LIST - index - 1));
		index = 0;
		if (header->next)
		{
			header->entries[ENTRIES_PER_LIST - 1] = header->next->entries[0];
			totalCount -= ENTRIES_PER_LIST;

			if (totalCount == 1)
			{
				free(header->next);
				header->next = NULL;
			}
		}
		header = header->next;
		
	} while (header);
}

#ifdef __cplusplus
}
#endif