#include "ch.h"
#include "adjacency.h"
#include "hashmap.h"
#include "heap.h"
#include "stdlib.h"
#include "float.h"

// initialises contraction highrachy
CHGraph *ch_init(Graph *g)
{
    CHGraph *ch_g = malloc(sizeof(CHGraph));
    long long *rank = malloc(g->node_count * sizeof(long long));

    // initialises rank to be -1 (uncontracted) for the whole graph
    for (int i = 0; i < g->node_count; i++)
    {
        rank[i] = -1;
    }

    // sets the contraction ch values
    ch_g->g = g;
    ch_g->rank = rank;

    return ch_g;
}

static double* local_dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src, long long skip_node, double max_dist)
{
    double *dist = malloc(g->node_count * sizeof(double));
    if (dist == NULL) {free(dist); return NULL;}

    long long* visited = malloc(g->node_count * sizeof(long long));
    if (visited == NULL) {free(dist); free(visited); return NULL;}

    for (long long i = 0; i < g->node_count; i++)
    {
        dist[i] = DBL_MAX;
        visited[i] = 0;
    }

    dist[src] = 0;

    MinHeap *heap = createHeap(1024);
    if (heap == NULL) {freeHeap(heap); free(dist); free(visited); return NULL;}

    push(heap, 0, src);

    while (heap->size > 0)
    {
        HeapNode cur_node = pop(heap);
        long long u = cur_node.nodeIndex;

        if (visited[u] == 1) {continue;}
        if (u == skip_node) {continue;}
        visited[u] = 1;

        for (int i = 0; i < adj[u].count; i++) {
            long long v = adj->edges[i].dst_index;
            if (v == -1 || visited[v] == 1) {continue;}

            double speed_ms = adj->edges[i].speed_limit / 3.6;
            double time = adj->edges[i].weight / speed_ms;
            double alt = dist[u] + time;

            if (alt < dist[v]) {
                dist[v] = alt;
                push(heap, alt, v);
            }
        }
    }

    freeHeap(heap);
    free(visited);
    return dist;
}