#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "loader.h"
#include "hashmap.h"
#include "adjacency.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("usage: ./builder/pathfinder.exe <path_to_bin> <src> <dst>\n");
        return 1;
    }

    printf("sizeof Node: %zu\n", sizeof(Node));
    printf("sizeof Edge: %zu\n", sizeof(Edge));

    const char* path = argv[1];
    long long src = atoll(argv[2]);
    long long dst = atoll(argv[3]);

    printf("loading graph...\n");
    Graph* g = load_graph(path);
    printf("loaded - node_count: %lld, edge_count: %lld\n", g->node_count, g->edge_count);

    HashMap* map = hashmap_create_index_from_graph(g);
    printf("%lld\n", map->buckets[1]->key);

    AdjList *adj = adjlist_create(g, map);

    printf("TEMPORARY ---- %lld, %lld\n", src, dst);
    adjlist_free(adj, g->node_count);
    hashmap_free(map);
    graph_free(g);
    return 0;
}