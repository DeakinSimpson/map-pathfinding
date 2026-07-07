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
    clock_t t_total = clock();

    // load in all variables
    LoadedVariables vars = utils_load_variables(argv[1]);

    t_total = clock() - t_total;
    printf("Variables Loaded in   %fs\n\n", ((double)t_total / CLOCKS_PER_SEC));

    // get indexes
    long long src_index;
    long long dst_index;
    utils_get_index(&src_index, &dst_index, argv, vars.tree, vars.g, vars.adj);

    // run algorithms
    ResultPath *dijkstra_rp_full = dijkstra(vars.g, vars.adj, vars.map, vars.g->nodes[src_index].id, vars.g->nodes[dst_index].id, 0);
    utils_print_results(dijkstra_rp_full);

    ResultPath *dijkstra_rp_early = dijkstra(vars.g, vars.adj, vars.map, vars.g->nodes[src_index].id, vars.g->nodes[dst_index].id, 1);
    utils_print_results(dijkstra_rp_early);

    ResultPath *astar_rp = astar(vars.g, vars.adj, vars.map, vars.g->nodes[src_index].id, vars.g->nodes[dst_index].id);
    utils_print_results(astar_rp);

    ResultPath *astar_reverse_rp = astar_bidir(vars.g, vars.adj, vars.adj_r, vars.map, vars.g->nodes[src_index].id, vars.g->nodes[dst_index].id);
    utils_print_results(astar_reverse_rp);

    ResultPath *ch_rp = ch_query(vars.g, vars.ch_g, vars.adj, vars.adj_r, vars.map, vars.g->nodes[src_index].id, vars.g->nodes[dst_index].id);
    utils_print_results(ch_rp);

    // freeing variables
    utils_free_variables(&vars);
    result_path_free(dijkstra_rp_full);
    result_path_free(dijkstra_rp_early);

    printf("Terminating Program...\n");
    return 0;
}