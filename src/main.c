#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "loader.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("usage: ./builder/pathfinder.exe <path_to_bin> <src> <dst>\n");
        return 1;
    }

    const char* path = argv[1];
    long long src = atoll(argv[2]);
    long long dst = atoll(argv[3]);

    printf("loading graph...\n");
    Graph* g = load_graph(path);
    printf("loaded - node_count: %lld, edge_count: %lld\n", g->node_count, g->edge_count);

    printf("TEMPORARY ---- %lld, %lld", src, dst);

    return 0;
}