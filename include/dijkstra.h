#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include "adjacency.h"
#include "hashmap.h"

ResultPath* dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id, int early_break);

#endif