#include "ch.h"
#include "adjacency.h"
#include "hashmap.h"
#include "heap.h"
#include "stdlib.h"
#include "float.h"
#include "time.h"

#define HOP_LIMIT 5

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
    int         *hops;
    MinHeap     *heap;

    dist = malloc(g->node_count * sizeof(double));
    if (dist == NULL) {free(dist); return NULL;}

    visited = malloc(g->node_count * sizeof(long long));
    if (visited == NULL) {free(dist); free(visited); return NULL;}

    hops = malloc(g->node_count * sizeof(int));
    if (hops == NULL) {free(dist); free(visited); free(hops); return NULL;}

    for (long long i = 0; i < g->node_count; i++)
    {
        dist[i] = DBL_MAX;
        visited[i] = 0;
        hops[i] = INT_MAX;
    }

    dist[src] = 0;
    hops[src] = 0;

    heap = createHeap(1024);
    if (heap == NULL) {free(dist); free(visited); free(hops); return NULL;}

    push(heap, 0, src);

    while (heap->size > 0)
    {
        HeapNode    cur_node;
        long long   u;

        cur_node = pop(heap);
        u = cur_node.nodeIndex;

        if (visited[u] == 1) {continue;}
        if (u == skip_node) {continue;}
        if (hops[u] >= HOP_LIMIT) {continue;}

        visited[u] = 1;

        if (dist[u] > max_dist) break;

        for (int i = 0; i < adj[u].count; i++) {
            long long   v;
            double      speed_ms;
            double      time;
            double      alt;

            v = adj[u].edges[i].dst_index;
            if (v == -1 || visited[v] == 1) {continue;}

            if (adj[u].edges[i].speed_limit == 0) {
                time = adj[u].edges[i].weight;
            } else {
                speed_ms = adj[u].edges[i].speed_limit / 3.6;
                time = adj[u].edges[i].weight / speed_ms;
            }
            
            alt         = dist[u] + time;

            if (alt < dist[v]) {
                dist[v] = alt;
                hops[v]    = hops[u] + 1;
                push(heap, alt, v);
            }
        }
    }

    freeHeap(heap);
    free(visited);
    free(hops);
    
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
        double u_time;
        if (adj_r[v].edges[i].speed_limit == 0) {
            u_time = adj_r[v].edges[i].weight;
        } else {
            double u_speed_ms = adj_r[v].edges[i].speed_limit / 3.6;
            u_time = adj_r[v].edges[i].weight / u_speed_ms;
        }

        for (int j = 0; j < adj[v].count; j++)
        {
            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                double w_speed_ms = adj[v].edges[j].speed_limit / 3.6;
                w_time = adj[v].edges[j].weight / w_speed_ms;
            }
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
        double *dist;

        u = adj_r[v].edges[i].dst_index;

        double u_time;
        if (adj_r[v].edges[i].speed_limit == 0) {
            u_time = adj_r[v].edges[i].weight;
        } else {
            double u_speed_ms = adj_r[v].edges[i].speed_limit / 3.6;
            u_time = adj_r[v].edges[i].weight / u_speed_ms;
        }

        dist = local_dijkstra(g, adj, u, v, max_dist);

        for (int j = 0; j < adj[v].count; j++)
        {
            long long w;

            w = adj[v].edges[j].dst_index;

            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                double w_speed_ms = adj[v].edges[j].speed_limit / 3.6;
                w_time = adj[v].edges[j].weight / w_speed_ms;
            }

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

static int compare_scores(const void *a, const void *b)
{
    NodeScore *na = (NodeScore*)a;
    NodeScore *nb = (NodeScore*)b;

    return na->score - nb->score;
}

// orderes the nodes based on there scores
static long long *ch_ordered_nodes(Graph *g, AdjList *adj, AdjList *adj_r)
{
    NodeScore *node_scores = malloc(g->node_count * sizeof(NodeScore));
    if (node_scores == NULL) return NULL;

    for (int i = 0; i < g->node_count; i++)
    {
        node_scores[i].index = i;
        node_scores[i].score = edge_difference(g, adj, adj_r, i);
    }

    qsort(node_scores, g->node_count, sizeof(NodeScore), compare_scores);

    long long *result = malloc(g->node_count * sizeof(long long));

    if (result == NULL) {free(node_scores); return NULL;}

    for (int i = 0; i < g->node_count; i++)
    {
        result[i] = node_scores[i].index;
    }

    free(node_scores);

    return result;
}

static void ch_contract_node(Graph *g, AdjList *adj, AdjList *adj_r, CHGraph *ch_g, long long v, long long rank)
{
    double max_dist = 0;

    for (int i = 0; i < adj_r[v].count; i++)
    {
        double u_time;
        if (adj_r[v].edges[i].speed_limit == 0) {
            u_time = adj_r[v].edges[i].weight;
        } else {
            u_time = adj_r[v].edges[i].weight / (adj_r[v].edges[i].speed_limit / 3.6);
        }

        for (int j = 0; j < adj[v].count; j++)
        {
            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            }

            double through_v = u_time + w_time;

            if (through_v > max_dist)
            {
                max_dist = through_v;
            }
        }
    }

    for (int i = 0; i < adj_r[v].count; i++)
    {
        long long u = adj_r[v].edges[i].dst_index;

        double u_time;
        if (adj_r[v].edges[i].speed_limit == 0) {
            u_time = adj_r[v].edges[i].weight;
        } else {
            u_time = adj_r[v].edges[i].weight / (adj_r[v].edges[i].speed_limit / 3.6);
        }

        double *dist = local_dijkstra(g, adj, u, v, max_dist);

        for (int j = 0; j < adj[v].count; j++)
        {
            long long w = adj[v].edges[j].dst_index;

            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            }

            // no path exists from v, add to shortcut
            if (dist[w] > u_time + w_time)
            {
                adjlist_add_edge(&adj[u], w, u_time + w_time, 0);
                adjlist_add_edge(&adj_r[w], u, u_time + w_time, 0);
            }
        }

        free(dist);
    }

    ch_g->rank[v] = rank;
}

CHGraph *ch_build(Graph *g, AdjList *adj, AdjList *adj_r)
{
    clock_t t = clock();

    CHGraph *ch_g = ch_init(g);

    long long *order = ch_ordered_nodes(g, adj, adj_r);

    for (int i = 0; i < g->node_count; i++)
    {
        long long v = order[i];
        ch_contract_node(g, adj, adj_r, ch_g, v, i);
    }

    free(order);

    t = clock() - t;

    printf("Time to build contaction hierachy: %fs\n", ((double)t / CLOCKS_PER_SEC));

    return ch_g;
}