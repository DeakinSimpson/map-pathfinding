#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct HashMapEntry {
    long long key;              // osm id
    long long value;            // array index
    struct HashMapEntry *next;  // pointer to the next value
} HashMapEntry;

typedef struct {
    HashMapEntry **buckets; // array of pointers to buckets
    long long capacity;     // number of buckets
    long long size;         // number of entries
} HashMap;

HashMap* hashmap_create(long long capacity);
void hashmap_insert(HashMap *map, long long key, long long value);
long long hashmap_get(HashMap *map, long long key);
void hashmap_free(HashMap *map);

#endif