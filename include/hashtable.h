#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

typedef struct HashEntry
{
	int filled;
	int key;
	void* value;
} HashEntry;

typedef struct HashTable
{
	int count;
	int capacity;
	HashEntry* entries;
} HashTable;

void HashRehash(HashTable* hash, int newCapacity);
void HashInit(HashTable* table);
void HashDelete(HashTable* hash);
void HashPut(HashTable* hash, int key, void* value);
void HashRemove(HashTable* hash, int key);
void* HashGet(HashTable* hash, int key);

#endif
