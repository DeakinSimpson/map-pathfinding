#include <stdio.h>
#include <stdlib.h>
#include <float.h>      // used for DBL_MAX
#include "dijkstra.h"
#include "hashmap.h"
#include "graph.h"
#include "heap.h"

long long* dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id) {
    // get source index from node id
    long long src_index = hashmap_get(map, src_id);

    if (src_index == -1) {
        printf("failed to get index for src");
        return NULL;
    }
    printf("src_index: %lld, edge count: %lld\n", src_index, adj[src_index].count);

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
    long long* visited = malloc(g->node_count * sizeof(long long));

    if (visited == NULL) {
        printf("failed to allocate memory for visited");
        return NULL;
    }

    // setting arrays to their specified values
    for (long long i = 0; i < g->node_count; i++) {
        dist[i] = DBL_MAX;  // sets the distance to DBL_MAX (infinite in theory)
        prev[i] = -1;       // sets all nodes prev node to -1, not traversed
        visited[i] = 0;     // sets all visisted nodes to 0 (false)
    }

    // set dist[src_index] = 0
    dist[src_index] = 0;

    // create heap and push src with distance 0
    MinHeap *heap = createHeap(1024);

    if (heap == NULL) {
        printf("failed to create heap");
        return NULL;
    }

    push(heap, 0, src_index);

    // while heap is not empty:
    while (heap->size > 0) {
        // pop the node with smallest distance (u)
        HeapNode cur_node = pop(heap);
        long long u = cur_node.nodeIndex;

        // if visited[u] skip
        if (visited[u] == 1) {
            continue;
        }

        // mark visited[u] = 1
        visited[u] = 1;

        //! if u == dst_index break early (POSSIBLY IMPLEMENT LATER)

        AdjList *neighbors = &adj[u];
        // loop through all neighbours of u in adj[u]:
        for (int i = 0; i < neighbors->count; i++) {
            // get neighbour index (v)
            long long v = neighbors->edges[i].dst_index;;

            // if visited[v] skip
            if (v == -1 || visited[v] == 1) {
                continue;
            }

            // calculate alternative distance = dist[u] + edge weight
            double alt = dist[u] + neighbors->edges[i].weight;

            // if alternative < dist[v]:
            if (alt < dist[v]) {
                dist[v] = alt;
                prev[v] = u;
                push(heap, alt, v);
            }
        }
    }

    // reconstruct path by walking prev[] from dst back to src
    long long *path = malloc(g->node_count * sizeof(long long));
    long long cur_index = dst_index;
    long long i = 0;    
    while (cur_index != -1) {
        printf("index = %lld\n", cur_index);
        path[i] = cur_index;
        cur_index = prev[cur_index];
        i++;
    }
    printf("final index = %lld\n", cur_index);

    // free dist, visited, heap
    free(heap);
    free(dist);
    free(visited);

    // return path
    return path;
}