#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "hashmap.h"
#include "adjacency.h"
#include "dijkstra.h"
#include "rtree.h"
#include "string.h"
#include "utils.h"
#include "astar.h"

int main(int argc, char* argv[]) {
    // initialise terminal variables
    if (argc != 7  && argc != 5) {
        printf("usage: ./builder/pathfinder.exe <path_to_bin> -c <src lat> <src lon> <dst lat> <dst lon>\n");
        printf("usage: ./builder/pathfinder.exe <path_to_bin> -d <src place> <dst place>\n");
        return 1;
    }

    printf("Pathfinder Starting...\n");
    // initialise variables
    const char  *bin_path   = argv[1];
    Graph       *g          = graph_load(bin_path);
    HashMap     *map        = hashmap_create_index_from_graph(g);
    AdjList     *adj        = adjlist_create(g, map);
    RTree       *tree       = rtree_build(g);

    // get indexes
    long long src_index;
    long long dst_index;
    utils_get_index(&src_index, &dst_index, argv, tree, g, adj);

    // run algorithms
    ResultPath *dijkstra_rp_full = dijkstra(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id, 0);
    if (dijkstra_rp_full == NULL) {
        printf("dFailed to find path\n");
    } else {
        printf("Dijksta full:\n\ttravel time: %f minutes\n\tdistance travelled: %f kms\n\ttime to load: %f seconds\n", (dijkstra_rp_full->time_in_seconds / 60), (dijkstra_rp_full->distance_in_metres / 1000), dijkstra_rp_full->load_time_in_seconds);
    }

    ResultPath *dijkstra_rp_early = dijkstra(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id, 1);
    if (dijkstra_rp_early == NULL) {
        printf("dFailed to find path\n");
    } else {
        printf("Dijksta early:\n\ttravel time: %f minutes\n\tdistance travelled: %f kms\n\ttime to load: %f seconds\n", (dijkstra_rp_early->time_in_seconds / 60), (dijkstra_rp_early->distance_in_metres / 1000), dijkstra_rp_early->load_time_in_seconds);
    }

    ResultPath *astar_rp = astar(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id);
    if (astar_rp == NULL) {
        printf("dFailed to find path\n");
    } else {
        printf("Astar:\n\ttravel time: %f minutes\n\tdistance travelled: %f kms\n\ttime to load: %f seconds\n", (astar_rp->time_in_seconds / 60), (astar_rp->distance_in_metres / 1000), astar_rp->load_time_in_seconds);
    }

    // freeing variables
    adjlist_free(adj, g->node_count);
    hashmap_free(map);
    graph_free(g);
    result_path_free(dijkstra_rp_full);
    result_path_free(dijkstra_rp_early);
    rtree_free(tree);

    printf("Terminating Program...\n");
    return 0;
}