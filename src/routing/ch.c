#include "ch.h"
#include "adjacency.h"
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

/*
runs a dijkstra from v outwards to max_dist
*/
static double* local_dijkstra(Graph *g, AdjList *adj, long long src, long long skip_node, double max_dist)
{
    double      *dist;
    long long   *visited;
    MinHeap     *heap;

    dist = malloc(g->node_count * sizeof(double));
    if (dist == NULL) {free(dist); return NULL;}

    visited = malloc(g->node_count * sizeof(long long));
    if (visited == NULL) {free(dist); free(visited); return NULL;}

    for (long long i = 0; i < g->node_count; i++)
    {
        dist[i] = DBL_MAX;
        visited[i] = 0;
    }

    dist[src] = 0;

    heap = createHeap(1024);
    if (heap == NULL) {free(dist); free(visited); return NULL;}

    push(heap, 0, src);

    while (heap->size > 0)
    {
        HeapNode    cur_node;
        long long   u;

        cur_node = pop(heap);
        u = cur_node.nodeIndex;

        if (visited[u] == 1) {continue;}
        if (u == skip_node) {continue;}
        visited[u] = 1;

        for (int i = 0; i < adj[u].count; i++) {
            long long   v;
            double      speed_ms;
            double      time;
            double      alt;

            v = adj[u].edges[i].dst_index;
            if (v == -1 || visited[v] == 1) {continue;}

            speed_ms    = adj[u].edges[i].speed_limit / 3.6;
            time        = adj[u].edges[i].weight / speed_ms;
            alt         = dist[u] + time;

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

/*
Gets the number of shortcuts between nodes that can be made if v was removed
*/
static int edge_difference(Graph *g, AdjList *adj, AdjList *adj_r, long long v)
{
    double max_dist = 0.0;   

    for (int i = 0; i < adj_r[v].count; i++) 
    {
        double u_time = adj_r[v].edges[i].weight / (adj_r[v].edges[i].speed_limit / 3.6);

        for (int j = 0; j < adj[v].count; j++) 
        {
            double w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            double through_v = u_time + w_time;

            if (through_v > max_dist)
            {
                max_dist = through_v;
            }
        }
    }

    int shortcuts = 0;

    for (int i = 0; i < adj_r[v].count; i++) 
    {
        long long u;
        double u_time;
        double *dist;

        u = adj_r[v].edges[i].dst_index;
        u_time = adj_r[v].edges[i].weight / (adj_r[v].edges[i].speed_limit / 3.6);

        dist = local_dijkstra(g, adj, u, v, max_dist);

        for (int j = 0; j < adj[v].count; j++)
        {
            long long w;
            double w_time;

            w = adj[v].edges[j].dst_index;
            w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);

            if (dist[w] > u_time + w_time)
            {
                shortcuts++;
            }
        }

        free(dist);
    }

    int edges_removed = adj_r[v].count + adj[v].count;

    return shortcuts - edges_removed;
}