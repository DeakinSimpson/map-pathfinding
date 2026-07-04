#ifndef ASTAR_H
#define ASTAR_H

#include "graph.h"
#include "adjacency.h"
#include "hashmap.h"

ResultPath* astar(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id);

#endif