#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

/*
Allocated space in memory for graph
*/
Graph* graph_create(long long node_capacity, long long edge_capacity) {
    Graph* g = malloc(sizeof(Graph));
    
    // allocatges space in memory for nodes and edges, then sets count to 0
    g->nodes = malloc(node_capacity * sizeof(Node));

    if (g->nodes == NULL) {
        printf("failed to allocate memory");
        graph_free(g);
    }

    g->edges = malloc(edge_capacity * sizeof(Edge));

    if (g->edges == NULL) {
        printf("failed to allocate memory");
        graph_free(g);
    }

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

Graph* graph_load(const char* path) {
    // open the binary
    FILE *f = fopen(path, "rb");

    // check if the file opened
    if (f == NULL) {
        printf("Failed to open bin\n");
        return NULL;
    }

    // read node count
    long long node_count;
    fread(&node_count, sizeof(long long), 1, f);

    // read edge count
    long long edge_count;
    fread(&edge_count, sizeof(long long), 1, f);

    // initialise Graph with counts
    Graph *g = graph_create(node_count, edge_count);

    // read all nodes & edges
    // as each chunk is exactly the size of a node, we can just set the nodes to the binary
    fread(g->nodes, sizeof(Node), node_count, f);
    fread(g->edges, sizeof(Edge), edge_count, f);

    // sets the node and edge count of the graph
    g->node_count = node_count;
    g->edge_count = edge_count;

    // close file
    fclose(f);

    // return graph
    return g;
}