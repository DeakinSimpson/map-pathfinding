#include <stdio.h>
#include "graph.h"
#include "loader.h"

int main(void) {
    printf("Starting Main\n");

    Graph* g = load_graph("data/bin/jamaica.bin");
    printf("node_count: %lld, edge_count: %lld\n", g->node_count, g->edge_count);

    return 0;
}