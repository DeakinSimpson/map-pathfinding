#ifndef ADJACENCY_H
#define ADJACENCY_H

#include "graph.h"
#include "hashmap.h"

typedef struct {
    long long dst_index;
    double weight;
    double km_weight;
    int speed_limit;
    enum RoadType road_type;
} AdjEdge;

typedef struct {
    AdjEdge *edges;

    long long count;
    long long capacity;
} AdjList;

AdjList* adjlist_create(Graph *g, HashMap *map, int reverse);
void adjlist_free(AdjList *adj, long long node_count);
void adjlist_add_edge(AdjList *adj, long long dst, double weight, double km_weight, int speed_limit);
AdjList* adjlist_create_empty(long long node_count);

#endif