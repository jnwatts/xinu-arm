#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

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
errcode HashGet(HashTable* hash, int key, void** value);

#ifdef __cplusplus
}
#endif

#endif
