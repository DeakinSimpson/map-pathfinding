#include "ch.h"
#include "stdlib.h"

// initialises contraction highrachy
CHGraph *ch_init(Graph *g)
{
    CHGraph *ch_g = malloc(sizeof(CHGraph));
    long long *rank = malloc(g->node_count * sizeof(long long));

    // initialises rank to be -1 (uncontracted) for the whole graph
    for (int i = 0; i < g->node_count; i++)
    {
        rank[i] = -1;
    }

    // sets the contraction ch values
    ch_g->g = g;
    ch_g->rank = rank;

    return ch_g;
}