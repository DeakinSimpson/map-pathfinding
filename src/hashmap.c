#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>

HashMap* hashmap_create(long long capacity) {
    HashMap* map = NULL;

    // allocate memory for the hashmap
    map = malloc(sizeof(HashMap));
    if (!map) goto cleanup;

    // set capacity and size defaults
    map->capacity = capacity;
    map->size = 0;

    // allocate memory for each bucket
    map->buckets = calloc(capacity, sizeof(HashMapEntry*));
    if (!map->buckets) goto cleanup;

    return map;

    cleanup:
        hashmap_free(map);
        return NULL;
}

void hashmap_insert(HashMap *map, long long key, long long value) {
    long long       index;
    HashMapEntry    *entry = NULL;

    // hash the key into an index
    index = key % map->capacity;

    // create a new entry
    entry = malloc(sizeof(HashMapEntry));
    if (!entry) hashmap_free(map);

    entry->key = key;
    entry->value = value;

    // insert at the front of the linked list for the bucket
    entry->next = map->buckets[index];
    map->buckets[index] = entry;

    // increase the size of the HashMap
    map->size += 1;
}

long long hashmap_get(HashMap *map, long long key) {
    long long       index;
    HashMapEntry    *entry;

    // get the index from the key
    index = key % map->capacity;

    // find the bucket that is associated with the index
    entry = map->buckets[index];

    // check over each entry until the key matches
    while (entry != NULL) {
        if (entry->key == key) {
            return entry->value;
        }

        entry = entry->next;
    }

    // return -1 if not found
    return -1;
}

void hashmap_free(HashMap *map) {
    if (map == NULL) {
        return;
    }

    // loop through all entries in a bucket and free them
    for (long long i = 0; i < map->capacity; i++) {
        HashMapEntry *entry = map->buckets[i];

        while (entry != NULL) {
            HashMapEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(map->buckets);
    free(map);
}

HashMap* hashmap_create_index_from_graph(Graph *g) {
    HashMap *map = NULL;

    // multiplying by 2 keeps hashmap half full to reduce collisions, can change this later
    map = hashmap_create(g->node_count * 2);
    if (!map) goto cleanup;

    // insert all nodes into the hashmap
    for (long long i = 0; i < g->node_count; i++){
        hashmap_insert(map, g->nodes[i].id, i);
    }

    return map;

    cleanup:
        hashmap_free(map);
        return NULL;
}