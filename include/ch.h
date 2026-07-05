#ifndef CH_H
#define CH_H

#include "graph.h"

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

#endif