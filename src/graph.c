#include "graph.h"
#include <stdlib.h>

/*
Allocated space in memory for graph
*/
Graph* graph_create(long long node_capacity, long long edge_capacity) {
    Graph* g = malloc(sizeof(Graph));
    
    // allocatges space in memory for nodes and edges, then sets count to 0
    g->nodes = malloc(node_capacity * sizeof(Node));
    g->edges = malloc(edge_capacity * sizeof(Edge));
    g->node_count = 0;
    g->edge_count = 0;

    return g;
}

/*
frees the graph from memory
*/
void graph_free(Graph* g) {
    free(g->nodes);
    free(g->edges);
    free(g);
}