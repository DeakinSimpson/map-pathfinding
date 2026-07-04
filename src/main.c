#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "hashmap.h"
#include "adjacency.h"
#include "dijkstra.h"
#include "rtree.h"
#include "string.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    // initialise terminal variables
    if (argc != 7  && argc != 5) {
        printf("usage: ./builder/pathfinder.exe <path_to_bin> -c <src lat> <src lon> <dst lat> <dst lon>\n");
        printf("usage: ./builder/pathfinder.exe <path_to_bin> -d <src place> <dst place>\n");
        return 1;
    }

    printf("Pathfinder Starting...\n");
    const char* path = argv[1];
    Coordinate src_coord;
    Coordinate dst_coord;

    utils_get_coord(&src_coord, &dst_coord, argv);

    // initialise variables
    Graph* g = graph_load(path);
    HashMap* map = hashmap_create_index_from_graph(g);
    AdjList *adj = adjlist_create(g, map);
    RTree *tree = rtree_build(g);

    long long src_index = rtree_nearest(tree, src_coord, g, adj);
    long long dst_index = rtree_nearest(tree, dst_coord, g, adj);

    // run algorithms
    ResultPath *dijkstra_rp = dijkstra(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id);
    if (dijkstra_rp == NULL) {
        printf("dFailed to find path\n");
    } else {
        printf("travel time: %f minutes\ndistance travelled: %f kms\ntime to load: %f seconds\n", (dijkstra_rp->time_in_seconds / 60), (dijkstra_rp->distance_in_metres / 1000), dijkstra_rp->load_time_in_seconds);
    }

    // freeing variables
    adjlist_free(adj, g->node_count);
    hashmap_free(map);
    graph_free(g);
    result_path_free(dijkstra_rp);
    rtree_free(tree);

    printf("Terminating Program...\n");
    return 0;
}