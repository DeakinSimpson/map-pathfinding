#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "hashmap.h"
#include "adjacency.h"
#include "dijkstra.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: ./builder/pathfinder.exe <path_to_bin> <src> <dst>\n");
        return 1;
    }

    printf("sizeof Node: %zu\n", sizeof(Node));
    printf("sizeof Edge: %zu\n", sizeof(Edge));

    const char* path = argv[1];
    // long long src = atoll(argv[2]);
    // long long dst = atoll(argv[3]);

    printf("loading graph...\n");
    Graph* g = graph_load(path);
    printf("loaded - node_count: %lld, edge_count: %lld\n", g->node_count, g->edge_count);

    long long src = g->nodes[1000].id;
    long long dst = g->nodes[0].id;

    printf("starting to create hash map\n");
    HashMap* map = hashmap_create_index_from_graph(g);
    printf("hashmap created\n");

    AdjList *adj = adjlist_create(g, map);
    printf("adj created\n");

    long long *dpath = dijkstra(g, adj, map, src, dst);
    if (dpath == NULL) {
        printf("dpath created ... NULL\n");
    }
    
    adjlist_free(adj, g->node_count);
    hashmap_free(map);
    graph_free(g);
    printf("freed\n");
    return 0;
}