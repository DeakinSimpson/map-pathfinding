#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "graph.h"
#include "hashmap.h"
#include "adjacency.h"
#include "dijkstra.h"
#include "rtree.h"
#include "string.h"
#include "utils.h"
#include "astar.h"
#include "ch.h"

int main(int argc, char* argv[]) {
    // initialise terminal variables
    if (argc != 7  && argc != 5) {
        printf("usage: ./builder/pathfinder.exe <path_to_bin> -c <src lat> <src lon> <dst lat> <dst lon>\n");
        printf("usage: ./builder/pathfinder.exe <path_to_bin> -d <src place> <dst place>\n");
        return 1;
    }

    printf("Pathfinder Starting...\n");
    // initialise variables
    clock_t t = clock();
    
    const char  *bin_path   = argv[1];
    Graph       *g          = graph_load(bin_path);
    HashMap     *map        = hashmap_create_index_from_graph(g);
    RTree       *tree       = rtree_build(g);
    printf("Number of nodes: %lld, edges %lld\n", g->node_count, g->edge_count);

    char cache_path[512];
    utils_cache_path(cache_path, sizeof(cache_path), bin_path);

    AdjList *adj    = NULL;
    AdjList *adj_r  = NULL;
    CHGraph *ch_g   = ch_load(cache_path, g, &adj, &adj_r);

    if (ch_g) {
        printf("Loaded cached contraction hierarchy from %s\n", cache_path);
    } else {
        adj   = adjlist_create(g, map, 0);
        adj_r = adjlist_create(g, map, 1);
        ch_g  = ch_build(g, adj, adj_r);
        ch_save(cache_path, g, adj, adj_r, ch_g);
    }
    
    t = clock() - t;
    printf("Variables Loaded in %fs\n\n", ((double)t / CLOCKS_PER_SEC));

    // get indexes
    long long src_index;
    long long dst_index;
    utils_get_index(&src_index, &dst_index, argv, tree, g, adj);

    // run algorithms
    ResultPath *dijkstra_rp_full = dijkstra(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id, 0);
    utils_print_results(dijkstra_rp_full);

    ResultPath *dijkstra_rp_early = dijkstra(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id, 1);
    utils_print_results(dijkstra_rp_early);

    ResultPath *astar_rp = astar(g, adj, map, g->nodes[src_index].id, g->nodes[dst_index].id);
    utils_print_results(astar_rp);

    ResultPath *astar_reverse_rp = astar_bidir(g, adj, adj_r, map, g->nodes[src_index].id, g->nodes[dst_index].id);
    utils_print_results(astar_reverse_rp);

    ResultPath *ch_rp = ch_query(g, ch_g, adj, adj_r, map, g->nodes[src_index].id, g->nodes[dst_index].id);
    utils_print_results(ch_rp);

    // freeing variables
    adjlist_free(adj, g->node_count);
    hashmap_free(map);
    graph_free(g);
    // result_path_free(dijkstra_rp_full);
    // result_path_free(dijkstra_rp_early);
    rtree_free(tree);

    printf("Terminating Program...\n");
    return 0;
}