// Xinu Lists
// Contains an implementation of a useful linked list 
// structure, designed to hold pointer-sized values.

#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define ENTRIES_PER_LIST 10

struct List;

typedef struct List
{
	int count;
	void* entries[ENTRIES_PER_LIST];
	struct List* next;
} List;

void ListInit(List* header);
void ListAdd(List* header, void* item);
void ListInsert(List* header, int index, void* item);
int ListGet(List* header, int index, void** item);
int ListCount(List* header);
void ListClear(List* header);
void ListRemoveAt(List* header, int index);

#ifdef __cplusplus
}
#endif

#endif