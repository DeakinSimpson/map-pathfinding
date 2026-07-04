#include "utils.h"
#include "rtree.h"
#include "graph.h"
#include "adjacency.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define places
Place PLACES[] = {
    {"Venice", {45.441310241560494, 12.31523127982292}},
    {"Vicenza", {45.54745520724044, 11.547651470387395}},
    {"Rome", {41.89107596300499, 12.492670206083258}}
};

int PLACES_COUNT = 3;

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
    if (strcmp(argv[2], "-c") == 0) {
        utils_parse_arg(src_coord, dst_coord, argv);
    } else {
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
