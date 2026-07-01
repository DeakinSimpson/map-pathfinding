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

    printf("Pathfinder Starting...\n");

    const char* path = argv[1];
    // long long src = atoll(argv[2]);
    // long long dst = atoll(argv[3]);

    Graph* g = graph_load(path);
    HashMap* map = hashmap_create_index_from_graph(g);
    AdjList *adj = adjlist_create(g, map);

    Coordinate src_coord = {45.47739253394642, 12.223124034279873};
    Coordinate dst_coord = {45.546369611049755, 11.54529729703707};

    long long src_index = graph_nearest_node(g, src_coord, adj);
    long long dst_index = graph_nearest_node(g, dst_coord, adj);

    ResultPath *rp = dijkstra(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id);
    if (rp == NULL) {
        printf("dFailed to find path\n");
    } else {
        printf("travel time: %f minutes\ndistance travelled: %f kms\ntime to load: %f seconds\n", (rp->time_in_seconds / 60), (rp->distance_in_metres / 1000), rp->load_time_in_seconds);
    }
    
    adjlist_free(adj, g->node_count);
    hashmap_free(map);
    graph_free(g);

    printf("Terminating Program...\n");
    return 0;
}