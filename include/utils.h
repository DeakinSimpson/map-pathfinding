#ifndef UTILS_H
#define UTILS_H

#include "graph.h"

typedef struct {
    const char *name;
    Coordinate coord;
} Place;

extern Place PLACES[];
extern int PLACES_COUNT;

void utils_print_places();
void utils_parse_arg(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]);
void utils_parse_places(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]);
void utils_get_coord(Coordinate *src_coord, Coordinate *dst_coord, char *argv[]);
#endif