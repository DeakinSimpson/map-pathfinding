#include "adjacency.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <time.h>

/*
generate an empty adjacency list, this is used so the cached contraction hierachies can be generated
*/
AdjList* adjlist_create_empty(long long node_count) {
    AdjList *adj = calloc(node_count, sizeof(AdjList));

    if (adj == NULL)
    {
        printf("Failed to allocate memory for adj\n");
    }

    return adj;
}

AdjList* adjlist_create(Graph *g, HashMap *map, int reverse) {
    // allocate an adjacency list per node
    AdjList *adj = adjlist_create_empty(g->node_count);
    if (adj == NULL) return NULL;

    for (long long i = 0; i < g->edge_count; i++) {
        Edge *e = &g->edges[i];

        // convert the OSM id into hashmap index
        long long src_index;
        if (reverse == 1)
        {
            src_index = hashmap_get(map, e->dst);
        } else
        {
            src_index = hashmap_get(map, e->src);
        }

        if (src_index == -1) {
            printf("failed to get hashmap for %lld", src_index);
            continue;
        }

        AdjList *list = &adj[src_index];

        // increase the list capacity if needed
        if (list->count >= list->capacity) {
            if (list->capacity == 0) {
                list->capacity = 2;
            } else {
                list->capacity = list->capacity * 2;
            }

            list->edges = realloc(list->edges, list->capacity * sizeof(AdjEdge));
        }

        // add the edge
        if (reverse == 1) {
            list->edges[list->count].dst_index = hashmap_get(map, e->src);
        } else {
            list->edges[list->count].dst_index = hashmap_get(map, e->dst);
        }

        list->edges[list->count].weight = e->weight;
        list->edges[list->count].km_weight = e->weight;
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

void adjlist_add_edge(AdjList *adj, long long dst, double weight, double km_weight, int speed_limit)
{
    if (adj->count >= adj->capacity)
    {
        if (adj->capacity == 0)
        {
            adj->capacity = 2;
        } else
        {
            adj->capacity *= 2;
        }
        adj->edges = realloc(adj->edges, adj->capacity * sizeof(AdjEdge));
    }
    adj->edges[adj->count].dst_index = dst;
    adj->edges[adj->count].weight = weight;
    adj->edges[adj->count].km_weight = km_weight;
    adj->edges[adj->count].speed_limit = speed_limit;
    adj->count++;
}

