#include <stddef.h>

static const int HashDefaulCapacity = 10;

void HashRehash(HashTable* hash, int newCapacity)
{
	HashEntry* oldEntries = hash->entries;
	int allocSize = sizeof(HashEntry) * newCapacity
	hash->entries entries = malloc(allocSize);
	memset(hash->entries, 0, allocSize);
	hash->count = 0;
	hash->capacity = newCapacity;

	for (int i = 0; i < hash->capacity; i++)
	{
		HashEntry* entry = &hash->entries[i];
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
	table->entries = malloc(initAlloc);
	memset(table->entries, 0, initAlloc)
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
	int i = key % table->capacity;
	int counter = 0;
	while (hash->entries[i].key != key && hash->entries[i].filled && counter < hash->count)
	{
		i = (i + 1) % table->capacity;
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

		entry->filled = true;
		entry->key = key;
		entry->value = value;
	}
}

void HashRemove(HashTable* hash, int key)
{
	int i = key % table->capacity;
	int counter = 0;
	while (hash->entries[i].key != key && hash->entries[i].filled && counter < hash->count)
	{
		i = (i + 1) % table->capacity;
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
	int i = key % table->capacity;
	int counter = 0;
	while (hash->entries[i].key != key && hash->entries[i].filled && counter < hash->count)
	{
		i = (i + 1) % table->capacity;
		counter++;
	}
	if (hash->entries[i].filled && hash->entries[i].key == key)
		return hash->entries[i].value;
	return NULL;
}