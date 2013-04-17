#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include "../include/hashtable.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

static const int HashDefaultCapacity = 10;

void HashRehash(HashTable* hash, int newCapacity)
{
	HashEntry* oldEntries = hash->entries;
	int oldCapacity = hash->capacity;
	int allocSize = sizeof(HashEntry) * newCapacity;
	int i;
	hash->entries = (HashEntry*)malloc(allocSize);
	memset(hash->entries, 0, allocSize);
	hash->count = 0;
	hash->capacity = newCapacity;

	for (i = 0; i < oldCapacity; i++)
	{
		HashEntry* entry = &oldEntries[i];
		if (entry->filled)
			HashPut(hash, entry->key, entry->value);
	}

	free(oldEntries);
}

void HashInit(HashTable* table)
{
	int initAlloc = sizeof(HashEntry) * HashDefaultCapacity;

	table->count = 0;
	table->capacity = HashDefaultCapacity;
	table->entries = (HashEntry*)malloc(initAlloc);
	memset(table->entries, 0, initAlloc);
}

void HashDelete(HashTable* hash)
{
	free(hash->entries);
	hash->capacity = 0;
	hash->count = 0;
	hash->entries = NULL;
}

void HashPut(HashTable* hash, int key, void* value)
{
	int i = key % hash->capacity;
	int counter = 0;
	while (hash->entries[i].key != key && hash->entries[i].filled && counter < hash->count)
	{
		i = (i + 1) % hash->capacity;
		counter++;
	}

	if (counter >= hash->count)
	{
		HashRehash(hash, hash->capacity * 2);
		HashPut(hash, key, value);
	}
	else
	{
		HashEntry* entry = &hash->entries[i];

		if (!entry->filled)
			hash->count++;

		entry->filled = TRUE;
		entry->key = key;
		entry->value = value;
	}
}

void HashRemove(HashTable* hash, int key)
{
	int i = key % hash->capacity;
	int counter = 0;
	while (hash->entries[i].key != key && hash->entries[i].filled && counter < hash->count)
	{
		i = (i + 1) % hash->capacity;
		counter++;
	}
	if (hash->entries[i].filled && hash->entries[i].key == key)
	{
		hash->entries[i].filled = FALSE;
		hash->count--;
	}
}

void* HashGet(HashTable* hash, int key)
{
	int i = key % hash->capacity;
	int counter = 0;
	while (hash->entries[i].key != key && hash->entries[i].filled && counter < hash->count)
	{
		i = (i + 1) % hash->capacity;
		counter++;
	}
	if (hash->entries[i].filled && hash->entries[i].key == key)
		return hash->entries[i].value;
	return NULL;
}