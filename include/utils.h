#ifndef UTILS_H
#define UTILS_H

#include "graph.h"
#include "adjacency.h"
#include "rtree.h"
#include "ch.h"
#include <stddef.h>

typedef struct {
    const char *name;
    Coordinate coord;
} Place;

typedef struct {
    Graph   *g;
    HashMap *map;
    AdjList *adj;
    AdjList *adj_r;
    AdjEdge *adj_pool;
    AdjEdge *adj_r_pool;
    CHGraph *ch_g;
    RTree   *tree;
    int     loaded_from_cache;
} LoadedVariables;

extern Place PLACES[];
extern int PLACES_COUNT;

void utils_print_places();
void utils_parse_arg(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]);
void utils_parse_places(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]);
void utils_get_coord(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]);
void utils_get_index(long long *src_index, long long *dst_index, char *argv[], RTree *tree, Graph *g, AdjList *adj);
void utils_print_results(ResultPath *rp);
void utils_cache_path(char *out, size_t out_size, const char *bin_path);
LoadedVariables utils_load_variables(const char *bin_path);
void utils_free_variables(LoadedVariables *vars);

#endif