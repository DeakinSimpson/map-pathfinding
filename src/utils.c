#include "utils.h"
#include "rtree.h"
#include "graph.h"
#include "adjacency.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

// Define places
Place PLACES[] = {
    {"Venice", {45.441310241560494, 12.31523127982292}},
    {"Vicenza", {45.54745520724044, 11.547651470387395}},
    {"Rome", {41.89107596300499, 12.492670206083258}},
    {"malta-east", {35.86126395007788, 14.571882644990628}},
    {"malta-west", {35.958249006888245, 14.365918959624256}},
    {"Traralgon", {-38.195017105603625, 146.53809419732184}},
    {"Melbourne", {-37.81746326177289, 144.9674458085295}}
};

int PLACES_COUNT = 7;

void utils_parse_arg(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]) {
    src_coord->lat = atof(argv[3]);
    src_coord->lon = atof(argv[4]);

    dst_coord->lat = atof(argv[5]);
    dst_coord->lon = atof(argv[6]);
}

void utils_parse_places(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]) {
    // check for src_coord
    for (int i = 0; i < PLACES_COUNT; i++)
    {
        if (strcmp(PLACES[i].name, argv[3]) == 0)
        {
            src_coord->lat = PLACES[i].coord.lat;
            src_coord->lon = PLACES[i].coord.lon;
        }
    }

    // check for dst_coord
    for (int i = 0; i < PLACES_COUNT; i++)
    {
        if (strcmp(PLACES[i].name, argv[4]) == 0)
        {
            dst_coord->lat = PLACES[i].coord.lat;
            dst_coord->lon = PLACES[i].coord.lon;
        }
    }    
}

void utils_get_coord(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]) {
    if (strcmp(argv[2], "-c") == 0)
    {
        utils_parse_arg(src_coord, dst_coord, argv);
    } else
    {
        utils_parse_places(src_coord, dst_coord, argv);
    }
}

void utils_get_index(long long *src_index, long long *dst_index, char *argv[], RTree *tree, Graph *g, AdjList *adj) {
    Coordinate src_coord;
    Coordinate dst_coord;
    utils_get_coord(&src_coord, &dst_coord, argv);

    *src_index = rtree_nearest(tree, src_coord, g, adj);
    *dst_index = rtree_nearest(tree, dst_coord, g, adj);
}

void utils_print_results(ResultPath *rp) {
    if (rp == NULL) {
        printf("dFailed to find path\n");
    } else {
        printf( 
                "%s\n"
                "\ttravel time:         %f minutes\n"
                "\tdistance travelled:  %f kms\n"
                "\ttime to load:        %f seconds\n", 
                rp->name, 
                (rp->time_in_seconds / 60), 
                (rp->distance_in_metres / 1000), 
                rp->load_time_in_seconds
            );
    }
}

// write *out to the *bin_path
void utils_cache_path(char *out, size_t out_size, const char *bin_path)
{
    // 
    const char *slash     = strrchr(bin_path, '/');
    const char *backslash = strrchr(bin_path, '\\');
    const char *filename  = bin_path;

    if (slash && (!backslash || slash > backslash)) filename = slash + 1;
    else if (backslash) filename = backslash + 1;

    snprintf(out, out_size, "data/contractions/%s", filename);
}

LoadedVariables utils_load_variables(const char *bin_path)
{
    LoadedVariables vars = {0};
    clock_t t_stage = clock();

    vars.g = graph_load(bin_path);
    printf("graph_load:           %fs\n", (double)(clock() - t_stage) / CLOCKS_PER_SEC);

    t_stage = clock();
    vars.map = hashmap_create_index_from_graph(vars.g);
    printf("hashmap_create:       %fs\n", (double)(clock() - t_stage) / CLOCKS_PER_SEC);

    char cache_path[512];
    utils_cache_path(cache_path, sizeof(cache_path), bin_path);

    vars.adj = NULL;
    vars.adj_r = NULL;
    vars.adj_pool = NULL;
    vars.adj_r_pool = NULL;
    vars.loaded_from_cache = 0;

    t_stage = clock();
    vars.ch_g = ch_load(cache_path, vars.g, &vars.adj, &vars.adj_r, &vars.adj_pool, &vars.adj_r_pool);
    printf("ch_load:              %fs\n", (double)(clock() - t_stage) / CLOCKS_PER_SEC);

    if (vars.ch_g) {
        vars.loaded_from_cache = 1;
    } else {
        t_stage = clock();
        vars.adj   = adjlist_create(vars.g, vars.map, 0);
        vars.adj_r = adjlist_create(vars.g, vars.map, 1);
        printf("adjlist_create x2:    %fs\n", (double)(clock() - t_stage) / CLOCKS_PER_SEC);

        vars.ch_g = ch_build(vars.g, vars.adj, vars.adj_r);   // prints its own internal timing already

        t_stage = clock();
        ch_save(cache_path, vars.g, vars.adj, vars.adj_r, vars.ch_g);
        printf("ch_save:              %fs\n", (double)(clock() - t_stage) / CLOCKS_PER_SEC);
    }

    t_stage = clock();
    vars.tree = rtree_build(vars.g);
    printf("rtree_build:          %fs\n", (double)(clock() - t_stage) / CLOCKS_PER_SEC);

    printf("Number of nodes: %lld, edges %lld\n", vars.g->node_count, vars.g->edge_count);

    return vars;
}

void utils_free_variables(LoadedVariables *vars)
{
    if (vars->loaded_from_cache) {
        adjlist_free_pooled(vars->adj, vars->adj_pool);
        adjlist_free_pooled(vars->adj_r, vars->adj_r_pool);
    } else {
        adjlist_free(vars->adj, vars->g->node_count);
        adjlist_free(vars->adj_r, vars->g->node_count);
    }
    hashmap_free(vars->map);
    graph_free(vars->g);
    rtree_free(vars->tree);
}