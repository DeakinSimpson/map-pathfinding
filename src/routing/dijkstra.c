#include <stdio.h>
#include <stdlib.h>
#include <float.h>      // used for DBL_MAX
#include <time.h>
#include "dijkstra.h"
#include "hashmap.h"
#include "graph.h"
#include "heap.h"

ResultPath* dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id) {
    clock_t t = clock();
    // get source index from node id
    long long src_index = hashmap_get(map, src_id);

    if (src_index == -1) {
        printf("failed to get index for src\n");
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

        free(dist);

        return NULL;
    }

    // allocate prev array (size node_count), set all to -1
    long long *prev = malloc(g->node_count * sizeof(long long));

    if (prev == NULL) {
        printf("failed to allocate memory for prev");

        free(prev);
        free(dist);

        return NULL;
    }

    // allocate visited array (size node_count), set all to 0
    long long* visited = malloc(g->node_count * sizeof(long long));

    if (visited == NULL) {
        printf("failed to allocate memory for visited");

        free(prev);
        free(dist);
        free(visited);

        return NULL;
    }

    double *km_dist = malloc(g->node_count * sizeof(double));

    if (km_dist == NULL) {
        printf("failed to allocate memory for km_dist");

        free(prev);
        free(dist);
        free(visited);

        return NULL;
    }


    // setting arrays to their specified values
    for (long long i = 0; i < g->node_count; i++) {
        dist[i] = DBL_MAX;  // sets the distance to DBL_MAX (infinite in theory)
        km_dist[i] = DBL_MAX;
        prev[i] = -1;       // sets all nodes prev node to -1, not traversed
        visited[i] = 0;     // sets all visisted nodes to 0 (false)
    }

    // set dist[src_index] = 0
    dist[src_index] = 0;
    km_dist[src_index] = 0;

    // create heap and push src with distance 0
    MinHeap *heap = createHeap(1024);

    if (heap == NULL) {
        printf("failed to create heap");
        
        free(heap);
        free(prev);
        free(dist);
        free(visited);        

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

        // if u == dst_index break early 
        if (u == dst_index) {
            break;
        }

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

    if (dist[dst_index] == DBL_MAX) {
        printf("no path found between src and dst\n");

        free(heap);
        free(prev);
        free(dist);
        free(visited);
        
        return NULL;
    }

    // reconstruct path by walking prev[] from dst back to src
    long long *path = malloc(g->node_count * sizeof(long long));
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

    // return path
    t = clock() - t;

    ResultPath *result = malloc(sizeof(ResultPath));
    result->path_inx = path;
    result->time_in_seconds = dist[dst_index];
    result->distance_in_metres = km_dist[dst_index];
    result->load_time_in_seconds = ((double)t / CLOCKS_PER_SEC);

    // free dist, visited, heap
    freeHeap(heap);
    free(prev);
    free(dist);
    free(visited);

    return result;
}