#ifndef ASTAR_H
#define ASTAR_H

#include "graph.h"
#include "adjacency.h"
#include "hashmap.h"

double heuristic(Coordinate a, Coordinate b);
ResultPath* astar(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id);
ResultPath* astar_bidir(Graph *g, AdjList *adj, AdjList *adj_r, HashMap *map, long long src_id, long long dst_id);

#endif