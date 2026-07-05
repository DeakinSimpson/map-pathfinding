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

CHGraph *ch_init(Graph *g);
CHGraph *ch_build(Graph *g, AdjList *adj, AdjList *adj_r);

#endif