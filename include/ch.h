#ifndef CH_H
#define CH_H

#include "graph.h"
#include "adjacency.h"

typedef struct 
{
    Graph *g;
    long long *rank;
} CHGraph;

typedef struct 
{
    long long index;
    int score;
} NodeScore;

CHGraph *ch_build(Graph *g, AdjList *adj, AdjList *adj_r);
ResultPath *ch_query(Graph *g, CHGraph *ch_g, AdjList *adj, AdjList *adj_r, HashMap *map, long long src_id, long long dst_id);
int ch_save(const char *path, Graph *g, AdjList *adj, AdjList *adj_r, CHGraph *ch_g);
CHGraph *ch_load(const char *path, Graph *g, AdjList **out_adj, AdjList **out_adj_r);

#endif