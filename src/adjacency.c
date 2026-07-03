#include "adjacency.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <time.h>

AdjList* adjlist_create(Graph *g, HashMap *map) {
    // allocate an adjacency list per node
    AdjList *adj = calloc(g->node_count, sizeof(AdjList));

    if (adj == NULL) {
        printf("Failed to allocate memory for adj\n");
        free(adj);
        return NULL;
    }

    for (long long i = 0; i < g->edge_count; i++) {
        Edge *e = &g->edges[i];

        // convert the OSM id into hashmap index
        long long src_index = hashmap_get(map, e->src);

        if (src_index == -1) {
            printf("failed to get hashmap for %lld", src_index);
            continue;
        }

        AdjList *list = &adj[src_index];

        // grop the list capacity if needed
        if (list->count >= list->capacity) {
            if (list->capacity == 0) {
                list->capacity = 2;
            } else {
                list->capacity = list->capacity * 2;
            }

            list->edges = realloc(list->edges, list->capacity * sizeof(AdjEdge));
        }

        // add the edge
        list->edges[list->count].dst_index = hashmap_get(map, e->dst);
        list->edges[list->count].weight = e->weight;
        list->edges[list->count].speed_limit = e->speed_limit;
        list->edges[list->count].road_type = e->road_type;
        list->count += 1;
    }

    return adj;
}

void adjlist_free(AdjList *adj, long long node_count) {
    for (long long i = 0; i < node_count; i++) {
        free(adj[i].edges);
    }

    free(adj);
}