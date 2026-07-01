#ifndef ADJACENCY_H
#define ADJACENCY_H

#include "graph.h"
#include "hashmap.h"

typedef struct {
    long long dst_index;
    double weight;
    int speed_limit;
    enum RoadType road_type;
} AdjEdge;

typedef struct {
    AdjEdge *edges;

    long long count;
    long long capacity;
} AdjList;

AdjList* adjlist_create(Graph *g, HashMap *index);
void adjlist_free(AdjList *adj, long long node_count);
long long graph_nearest_node(Graph *g, Coordinate coord, AdjList *adj);

#endif