#include <stdio.h>
#include <stdlib.h>
#include <float.h>      // used for DBL_MAX
#include "dijkstra.h"
#include "hashmap.h"
#include "graph.h"

long long* dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id) {
    // get source index from node id
    long long src_index = hashmap_get(map, src_id);

    if (src_index == -1) {
        printf("failed to get index for src");
        return NULL;
    }

    // get destination index from node id
    long long dst_index = hashmap_get(map, dst_id);

    if (dst_index == -1) {
        printf("failed to get index for dst");
        return NULL;
    }

    // allocate dist array (size node_count), set all to DBL_MAX
    double *dist = malloc(g->node_count * sizeof(double));

    if (dist == NULL) {
        printf("failed to allocate memory for dist");
        return NULL;
    }

    // allocate prev array (size node_count), set all to -1
    long long *prev = malloc(g->node_count * sizeof(long long));

    if (prev == NULL) {
        printf("failed to allocate memory for prev");
        return NULL;
    }

    printf("%lld\n", adj->capacity);

    // allocate visited array (size node_count), set all to 0

    // set dist[src_index] = 0

    // create heap and push src with distance 0

    // while heap is not empty:
        // pop the node with smallest distance (u)

        // if visited[u] skip

        // mark visited[u] = 1

        // if u == dst_index break early

        // loop through all neighbours of u in adj[u]:
            // get neighbour index (v)

            // if visited[v] skip

            // calculate alternative distance = dist[u] + edge weight

            // if alternative < dist[v]:
                // update dist[v]
                // update prev[v] = u
                // push v onto heap with new distance

    // reconstruct path by walking prev[] from dst back to src

    // free dist, visited, heap

    // return path
    return NULL;
}