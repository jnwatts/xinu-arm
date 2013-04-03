// Xinu Lists
// Contains an implementation of a useful linked list 
// structure, designed to hold pointer-sized values.

#ifndef _LIST_H_
#define _LIST_H_

#include "stddef.h"

#define ENTRIES_PER_LIST 10

typedef struct List_t
{
	int count;
	void* entries[ENTRIES_PER_LIST];
	List_t* next;
} List;

void ListInit(List* header);
void ListAdd(List* header, void* item);
int ListGet(List* header, int index, void** item);
int ListCount(List* header);
void ListClear(List* header);
int ListRemoveAt(List* header, int index);

#endif