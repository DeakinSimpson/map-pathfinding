#ifndef HASHMAP_H
#define HASHMAP_H

#include "graph.h"

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

/*
Creates an empty hashmap of size capacity
*/
HashMap* hashmap_create(long long capacity);

/*
inserts a key value pair into the hashmap
*/
void hashmap_insert(HashMap *map, long long key, long long value);
/*
gets the index from in the hashmap from the OSM value
*/
long long hashmap_get(HashMap *map, long long key);
/*
Clears the hashmap from memory
*/
void hashmap_free(HashMap *map);
/*
Creates a indexed hashmap from a Graph
*/
HashMap* hashmap_create_index_from_graph(Graph *g);

#endif