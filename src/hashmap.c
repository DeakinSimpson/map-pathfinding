#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>

HashMap* hashmap_create(long long capacity) {
    HashMap* map = malloc(capacity * sizeof(HashMap));

    if (map == NULL) {
        printf("Failed to allocate memory for HashMap");
        hashmap_free(map);
        return NULL;
    }

    map->buckets = calloc(capacity, sizeof(HashMapEntry*));

    if (map->buckets == NULL) {
        printf("Failed to allocate memory for map->buckets");
        hashmap_free(map);
        return NULL;
    }

    map->capacity = capacity;
    map->size = 0;

    return map;
}

void hashmap_insert(HashMap *map, long long key, long long value) {
    // hash the key into an index
    long long index = key % map->capacity;

    // create a new entry
    HashMapEntry *entry = malloc(sizeof(HashMapEntry));
    
    if (entry == NULL) {
        printf("Failed to allocate memory for HashMapEntry");
        hashmap_free(map);
        return;
    }

    entry->key = key;
    entry->value = value;

    // insert at the front of the linked list for the bucket
    entry->next = map->buckets[index];
    map->buckets[index] = entry;

    // increase the size of the HashMap
    map->size += 1;
}

long long hashmap_get(HashMap *map, long long key) {
    // get the index from the key
    long long index = key % map->capacity;

    // find the bucket that is associated with the index
    HashMapEntry *entry = map->buckets[index];

    // check over each entry until the key matches
    while (entry != NULL) {
        if (entry->key == key) {
            return entry->key;
        }

        entry = entry->next;
    }

    // return -1 if not found
    return -1;
}

void hashmap_free(HashMap *map) {
    free(map->buckets);
    free(map);
}

HashMap* hashmap_create_index_from_graph(Graph *g) {
    // multiplying by 2 keeps hashmap half full to reduce collisions, can change this later
    HashMap *map = hashmap_create(g->node_count * 2);

    for (long long i = 0; i < g->node_count; i++){
        hashmap_insert(map, g->nodes[i].id, i);
    }

    return map;
}