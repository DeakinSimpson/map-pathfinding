#ifndef ADJACENCY_H
#define ADJACENCY_H

#include "graph.h"
#include "hashmap.h"

typedef struct {
    long long dst_index;
    double weight;
} AdjEdge;

typedef struct {
    AdjEdge *edges;

    long long count;
    long long capacity;
} AdjList;

AdjList* adjlist_create(Graph *g, HashMap *index);
void adjlist_free(AdjList *adj, long long node_count);

#endif