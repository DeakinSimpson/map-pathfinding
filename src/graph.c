#include "graph.h"
#include "adjacency.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
Allocated space in memory for graph
*/
Graph* graph_create(long long node_capacity, long long edge_capacity) {
    Graph* g = NULL;
  
    g = malloc(sizeof(Graph));
    if (!g) goto cleanup;

    g->node_count = 0;
    g->edge_count = 0;

    // allocatges space in memory for nodes and edges, then sets count to 0
    g->nodes = malloc(node_capacity * sizeof(Node));
    if (!g->nodes) goto cleanup;

    g->edges = malloc(edge_capacity * sizeof(Edge));
    if (!g->edges) goto cleanup;

    return g;

    cleanup:
        graph_free(g);
        return NULL;
}

/*
frees the graph from memory
*/
void graph_free(Graph* g) {
    free(g->nodes);
    free(g->edges);
    free(g);
}

Graph* graph_load(const char* path) {
    // open the binary
    FILE *f = fopen(path, "rb");

    // check if the file opened
    if (f == NULL) {
        printf("Failed to open bin\n");
        return NULL;
    }

    // read node count
    long long node_count;
    fread(&node_count, sizeof(long long), 1, f);

    // read edge count
    long long edge_count;
    fread(&edge_count, sizeof(long long), 1, f);

    // initialise Graph with counts
    Graph *g = graph_create(node_count, edge_count);

    // read all nodes & edges
    // as each chunk is exactly the size of a node, we can just set the nodes to the binary
    fread(g->nodes, sizeof(Node), node_count, f);
    fread(g->edges, sizeof(Edge), edge_count, f);

    // sets the node and edge count of the graph
    g->node_count = node_count;
    g->edge_count = edge_count;

    // close file
    fclose(f);

    // return graph
    return g;
}

#define PI 3.14159265358979323846

double to_rad(double deg) {
    return (deg * PI) / 180;
}

#define EARTH_RADIUS_KM 6371.0

double haversine(Coordinate coord1, Coordinate coord2) {
    double dlat = to_rad(coord2.lat - coord1.lat);
    double dlon = to_rad(coord2.lon - coord1.lon);
    double a = sin(dlat/2) * sin(dlat/2) +
               cos(to_rad(coord1.lat)) * cos(to_rad(coord2.lat)) *
               sin(dlon/2) * sin(dlon/2);
    return EARTH_RADIUS_KM * 2 * atan2(sqrt(a), sqrt(1-a));
}

void result_path_free(ResultPath *rp) {
    free(rp->path_inx);
    free(rp);
}