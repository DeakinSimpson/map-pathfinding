#include <stdio.h>
#include <stdlib.h>
#include <float.h>      // used for DBL_MAX
#include <time.h>
#include "dijkstra.h"
#include "hashmap.h"
#include "graph.h"
#include "heap.h"

ResultPath *dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id, int early_break) {
    long long   src_index   = -1;
    long long   dst_index   = -1;
    double      *dist       = NULL;
    long long   *prev       = NULL;
    long long   *visited    = NULL;
    double      *km_dist    = NULL;
    MinHeap     *heap       = NULL;
    long long   *path       = NULL;
    ResultPath  *result     = NULL;
    
    clock_t t = clock();

    // get source index from node id
    src_index = hashmap_get(map, src_id);
    if (src_index == -1) goto cleanup;

    // get destination index from node id
    dst_index = hashmap_get(map, dst_id);
    if (dst_index == -1) goto cleanup;

    // allocate dist array (size node_count), set all to DBL_MAX
    dist = malloc(g->node_count * sizeof(double));
    if (!dist) goto cleanup;

    // allocate prev array (size node_count), set all to -1
    prev = malloc(g->node_count * sizeof(long long));
    if (!prev) goto cleanup;

    // allocate visited array (size node_count), set all to 0
    visited = malloc(g->node_count * sizeof(long long));
    if (!visited) goto cleanup;

    km_dist = malloc(g->node_count * sizeof(double));
    if (!km_dist) goto cleanup;

    // setting arrays to their specified values
    for (long long i = 0; i < g->node_count; i++)
    {
        dist[i] = DBL_MAX;  // sets the distance to DBL_MAX (infinite in theory)
        km_dist[i] = DBL_MAX;
        prev[i] = -1;       // sets all nodes prev node to -1, not traversed
        visited[i] = 0;     // sets all visisted nodes to 0 (false)
    }

    // set dist[src_index] = 0
    dist[src_index] = 0;
    km_dist[src_index] = 0;

    // create heap and push src with distance 0
    heap = createHeap(1024);
    if (!heap) goto cleanup;

    push(heap, 0, src_index);

    // while heap is not empty:
    while (heap->size > 0) {
        // pop the node with smallest distance (u)
        HeapNode cur_node = pop(heap);
        long long u = cur_node.nodeIndex;

        if (visited[u] == 1) continue; // if visited[u] skip
        if (early_break == 1 && u == dst_index) break; // if u == dst_index break early 

        visited[u] = 1; // mark visited[u] = 1

        AdjList *neighbors = &adj[u];
        // loop through all neighbours of u in adj[u]:
        for (int i = 0; i < neighbors->count; i++) {
            // get neighbour index (v)
            long long v = neighbors->edges[i].dst_index;;
            if (v == -1 || visited[v] == 1) continue; // if visited[v] skip

            // calculate alternative distance = dist[u] + edge weight
            double speed_ms = neighbors->edges[i].speed_limit / 3.6;
            double time = neighbors->edges[i].weight / speed_ms;
            double alt = dist[u] + time;

            // if alternative < dist[v]:
            if (alt < dist[v]) {
                dist[v] = alt;
                prev[v] = u;
                km_dist[v] = km_dist[u] + neighbors->edges[i].weight;
                push(heap, alt, v);
            }
        }
    }

    if (dist[dst_index] == DBL_MAX) goto cleanup;

    // reconstruct path by walking prev[] from dst back to src
    path = malloc(g->node_count * sizeof(long long));
    if (!path) goto cleanup;

    long long cur_index = dst_index;
    long long i = 0;

    while (cur_index != -1) {
        path[i] = cur_index;
        cur_index = prev[cur_index];
        i++;
    }

    // reverse path so it goes from src to dst
    for (long long l = 0, r = i - 1; l < r; l++, r--) {
        long long tmp = path[l];
        path[l] = path[r];
        path[r] = tmp;
    }

    path = realloc(path, (i + 1) * sizeof(long long));
    if (!path) goto cleanup;

    // return path
    t = clock() - t;

    result = malloc(sizeof(ResultPath));


    if (early_break) {
        result->name = "Dijkstra Early Break";
    } else {
        result->name = "Dijkstra Full";
    }

    result->path_inx = path;
    result->time_in_seconds = dist[dst_index];
    result->distance_in_metres = km_dist[dst_index];
    result->load_time_in_seconds = ((double)t / CLOCKS_PER_SEC);

    // free dist, visited, heap
    cleanup:
        freeHeap(heap);
        free(prev);
        free(dist);
        free(visited);

    return result;
}