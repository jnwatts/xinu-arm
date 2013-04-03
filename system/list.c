// Xinu Lists
// Contains an implementation of a useful linked list 
// structure, designed to hold pointer-sized values.

#include "list.h"

void ListInit(List* header)
{
	header->count = 0;
	header->next = NULL;
	header->prev = NULL;
}

void ListAdd(List* header, void* item)
{
	int count = header->count;
	header->count++;
	while (count >= ENTRIES_PER_LIST)
	{
		if (header->next == NULL)
		{
			header->next = malloc(sizeof(List));
			header->next->count = 0;
			header->next->next = NULL;
		}
		count -= ENTRIES_PER_LIST;
		header = header->next;
	}
	header->entries[count] = item;
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

int ListRemoveAt(List* header, int index)
{
	int totalCount = header->count;

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
		memmove(&header->entries[index], &header->entries[index + 1], ENTRIES_PER_LIST - index - 1);
		index = 0;
		if (header->next)
		{
			header->entries[ENTRIES_PER_LIST - 1] = header->next->entries[0];
			totalCount -= ENTRIES_PER_LIST;
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