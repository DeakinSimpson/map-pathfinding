#ifndef CH_H
#define CH_H

#include "graph.h"

typedef struct 
{
    Graph *g;
    long long *rank;
} CHGraph;

CHGraph *ch_init(Graph *g);

#endif