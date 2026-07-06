#include "ch.h"
#include "adjacency.h"
#include "hashmap.h"
#include "heap.h"
#include "stdlib.h"
#include "float.h"
#include "time.h"

#define HOP_LIMIT 20

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
static double* local_dijkstra(CHGraph *ch_g, AdjList *adj, long long src, long long skip_node, double max_dist, double *dist, long long *visited, int *hops, MinHeap *heap, long long *touched, int *touched_count)
{
    // reset dist from previous call
    for (int i = 0; i < *touched_count; i++) 
    {
        dist[touched[i]] = DBL_MAX;
    }

    *touched_count  = 0;
    heap->size      = 0;
    dist[src]       = 0;
    hops[src]       = 0;
    touched[(*touched_count)++] = src;

    push(heap, 0, src);

    while (heap->size > 0)
    {
        HeapNode  cur_node = pop(heap);
        long long u        = cur_node.nodeIndex;

        if (visited[u] == 1) continue;
        if (u == skip_node)  continue;
        if (dist[u] > max_dist) break;
        if (hops[u] >= HOP_LIMIT) continue;

        visited[u] = 1;

        for (int i = 0; i < adj[u].count; i++) {
            long long v = adj[u].edges[i].dst_index;

            if (v == -1 || visited[v] == 1 || ch_g->rank[v] != -1) continue;

            double time;
            if (adj[u].edges[i].speed_limit == 0) {
                time = adj[u].edges[i].weight;
            } else {
                double speed_ms = adj[u].edges[i].speed_limit / 3.6;
                time = adj[u].edges[i].weight / speed_ms;
            }

            double alt = dist[u] + time;

            if (alt < dist[v]) {
                if (dist[v] == DBL_MAX) {
                    touched[(*touched_count)++] = v;
                }
                dist[v] = alt;
                hops[v] = hops[u] + 1;
                push(heap, alt, v);
            }
        }
    }

    // reset visited and hops only — dist stays valid for the caller to read
    for (int i = 0; i < *touched_count; i++) {
        visited[touched[i]] = 0;
        hops[touched[i]]    = INT_MAX;
    }

    return dist;
}

/*
Gets the number of shortcuts between nodes that can be made if v was removed
*/
static int edge_difference(CHGraph *ch_g, AdjList *adj, AdjList *adj_r, long long v, double *dist, long long *visited, int *hops, MinHeap *heap, long long *touched, int *touched_count)
{
    int shortcuts = 0;

    for (int i = 0; i < adj_r[v].count; i++)
    {
        long long u = adj_r[v].edges[i].dst_index;
        if (u == -1 || u == v || ch_g->rank[u] != -1) continue;

        double u_time;
        if (adj_r[v].edges[i].speed_limit == 0) {
            u_time = adj_r[v].edges[i].weight;
        } else {
            u_time = adj_r[v].edges[i].weight / (adj_r[v].edges[i].speed_limit / 3.6);
        }

        // per-u max_dist: only as far as the farthest outgoing neighbor needs
        double max_dist = 0.0;
        for (int j = 0; j < adj[v].count; j++) {
            long long w = adj[v].edges[j].dst_index;
            if (w == -1 || w == v || w == u || ch_g->rank[w] != -1) continue;
            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            }
            if (u_time + w_time > max_dist) max_dist = u_time + w_time;
        }

        double *l_dist = local_dijkstra(ch_g, adj, u, v, max_dist, dist, visited, hops, heap, touched, touched_count);

        for (int j = 0; j < adj[v].count; j++) {
            long long w = adj[v].edges[j].dst_index;
            if (w == -1 || w == v || w == u || ch_g->rank[w] != -1) continue;

            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            }

            if (l_dist[w] > u_time + w_time) shortcuts++;
        }
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
static long long *ch_ordered_nodes(CHGraph *ch_g, Graph *g, AdjList *adj, AdjList *adj_r, double *dist, long long *visited, int *hops, MinHeap *heap, long long *touched, int *touched_count)
{
    NodeScore *node_scores = malloc(g->node_count * sizeof(NodeScore));
    if (node_scores == NULL) return NULL;

    for (int i = 0; i < g->node_count; i++)
    {
        node_scores[i].index = i;
        node_scores[i].score = edge_difference(ch_g, adj, adj_r, i, dist, visited, hops, heap, touched, touched_count);
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

static void ch_contract_node(AdjList *adj, AdjList *adj_r, CHGraph *ch_g, long long v, long long rank, double *dist, long long *visited, int *hops, MinHeap *heap, long long *touched, int *touched_count)
{
    long long incoming_count = adj_r[v].count;
    long long outgoing_count = adj[v].count;

    for (int i = 0; i < incoming_count; i++)
    {
        long long u = adj_r[v].edges[i].dst_index;
        if (u == -1 || u == v || ch_g->rank[u] != -1) continue;

        double u_time;
        if (adj_r[v].edges[i].speed_limit == 0) {
            u_time = adj_r[v].edges[i].weight;
        } else {
            u_time = adj_r[v].edges[i].weight / (adj_r[v].edges[i].speed_limit / 3.6);
        }

        // per-u max_dist: only search as far as needed for the farthest w
        double max_dist = 0.0;
        for (int j = 0; j < outgoing_count; j++) {
            long long w = adj[v].edges[j].dst_index;
            if (w == -1 || w == v || w == u || ch_g->rank[w] != -1) continue;
            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            }
            if (u_time + w_time > max_dist) max_dist = u_time + w_time;
        }

        double *l_dist = local_dijkstra(ch_g, adj, u, v, max_dist, dist, visited, hops, heap, touched, touched_count);

        for (int j = 0; j < outgoing_count; j++) {
            long long w = adj[v].edges[j].dst_index;
            if (w == -1 || w == v || w == u || ch_g->rank[w] != -1) continue;

            double w_time;
            if (adj[v].edges[j].speed_limit == 0) {
                w_time = adj[v].edges[j].weight;
            } else {
                w_time = adj[v].edges[j].weight / (adj[v].edges[j].speed_limit / 3.6);
            }

            if (l_dist[w] > u_time + w_time) {
                double u_dist = adj_r[v].edges[i].km_weight;
                double w_dist = adj[v].edges[j].km_weight;
                adjlist_add_edge(&adj[u], w, u_time + w_time, u_dist + w_dist, 0);
                adjlist_add_edge(&adj_r[w], u, u_time + w_time, u_dist + w_dist, 0);
            }
        }
    }

    ch_g->rank[v] = rank;
}

CHGraph *ch_build(Graph *g, AdjList *adj, AdjList *adj_r)
{
    CHGraph     *ch_g       = NULL;
    double      *dist       = NULL;
    long long   *visited    = NULL;
    int         *hops       = NULL;
    MinHeap     *heap       = NULL;
    long long   *touched    = NULL;
    long long   *order      = NULL;

    // initialise variables
    clock_t t = clock();

    ch_g = ch_init(g);
    if (!ch_g) goto cleanup;

    dist = malloc(g->node_count * sizeof(double));
    if (!dist) goto cleanup;

    visited = malloc(g->node_count * sizeof(long long));
    if (!visited) goto cleanup;

    hops = malloc(g->node_count * sizeof(int));
    if (!hops) goto cleanup;

    heap = createHeap(1024);
    if (!heap) goto cleanup;

    touched = malloc(g->node_count * sizeof(long long));
    if (!touched) goto cleanup;

    int touched_count = 0;
    for (long long i = 0; i < g->node_count; i++)
    {
        dist[i]    = DBL_MAX;
        visited[i] = 0;
        hops[i]    = INT_MAX;
    }

    order = ch_ordered_nodes(ch_g, g, adj, adj_r, dist, visited, hops, heap, touched, &touched_count);
    if (!order) goto cleanup;

    for (long long i = 0; i < g->node_count; i++)
    {
        long long v = order[i];
        ch_contract_node(adj, adj_r, ch_g, v, i, dist, visited, hops, heap, touched, &touched_count);
    }

    cleanup:
        free(order);
        free(touched);
        freeHeap(heap);
        free(visited);
        free(hops);
        free(dist);

    t = clock() - t;

    printf("Time to build contaction hierachy: %fs\n", ((double)t / CLOCKS_PER_SEC));

    return ch_g;
}

/*
Query a contraction hierachy
*/
ResultPath *ch_query(Graph *g, CHGraph *ch_g, AdjList *adj, AdjList *adj_r, HashMap *map, long long src_id, long long dst_id)
{
    clock_t t = clock();

    long long   src_index   = -1;
    long long   dst_index   = -1;
    double      *dist_f     = NULL;
    double      *dist_r     = NULL;
    long long   *prev_f     = NULL;
    long long   *prev_r     = NULL;
    long long   *visited_f  = NULL;
    long long   *visited_r  = NULL;
    double      *km_dist_f  = NULL;
    double      *km_dist_r  = NULL;
    MinHeap     *heap_f     = NULL;
    MinHeap     *heap_r     = NULL;
    long long   *path_f     = NULL;
    long long   *path_r     = NULL;
    long long   *path       = NULL;
    ResultPath  *result     = NULL;



    // get source index from node id
    src_index = hashmap_get(map, src_id);
    if (src_index == -1) goto cleanup;

    // get destination index from node id
    dst_index = hashmap_get(map, dst_id);
    if (dst_index == -1) goto cleanup;

    // Allocate dist_f, dist_r, prev_f, prev_r, visited_f, visited_r arrays
    // allocate dist array (size node_count), set all to DBL_MAX
    dist_f = malloc(g->node_count * sizeof(double));
    if (!dist_f) goto cleanup;

    dist_r = malloc(g->node_count * sizeof(double));
    if (!dist_r) goto cleanup;

    // allocate prev array (size node_count), set all to -1
    prev_f = malloc(g->node_count * sizeof(long long));
    if (!prev_f) goto cleanup;

    prev_r = malloc(g->node_count * sizeof(long long));
    if (!prev_r) goto cleanup;

    // allocate visited array (size node_count), set all to 0
    visited_f = malloc(g->node_count * sizeof(long long));
    if (!visited_f) goto cleanup;

    visited_r = malloc(g->node_count * sizeof(long long));
    if (!visited_r) goto cleanup;


    km_dist_f = malloc(g->node_count * sizeof(double));
    if (!km_dist_f) goto cleanup;

    km_dist_r = malloc(g->node_count * sizeof(double));
    if (!km_dist_r) goto cleanup;

    // setting arrays to their specified values
    for (long long i = 0; i < g->node_count; i++)
    {
        dist_f[i] = DBL_MAX;
        dist_r[i] = DBL_MAX;
        km_dist_f[i] = DBL_MAX;
        km_dist_r[i] = DBL_MAX;
        prev_f[i] = -1;
        prev_r[i] = -1;
        visited_f[i] = 0;
        visited_r[i] = 0;
    }

    // Create heap_f and heap_r
    // create heap and push src with distance 0
    heap_f = createHeap(1024);

    if (heap_f == NULL)
    if (!heap_f) goto cleanup;

    heap_r = createHeap(1024);
    if (!heap_r) goto cleanup;

    // Push src onto heap_f and dst onto heap_r
    dist_f[src_index] = 0;
    dist_r[dst_index] = 0;
    km_dist_f[src_index] = 0;
    km_dist_r[dst_index] = 0;

    push(heap_f, 0, src_index);
    push(heap_r, 0, dst_index);

    long long best_meeting = -1;
    double best_cost = DBL_MAX;

    // Alternate between forward and backward search each iteration
    while (heap_f->size > 0 && heap_r->size > 0) 
    {
        double      *dist_c     = NULL;
        double      *km_dist_c  = NULL;
        long long   *prev_c     = NULL;
        long long   *visited_c  = NULL;
        AdjList     *neighbors  = NULL;
        MinHeap     *heap_c     = NULL;
        long long   *visited_o  = NULL;
        HeapNode    cur_node    = {-1, -1};        
        long long   u;

        // expand on the side with the best distance
        if (heap_f->node[0].dist <= heap_r->node[0].dist) 
        {
            dist_c      = dist_f;
            km_dist_c   = km_dist_f;
            prev_c      = prev_f;
            visited_c   = visited_f;
            cur_node    = pop(heap_f);
            u           = cur_node.nodeIndex;
            neighbors   = &adj[u];
            heap_c      = heap_f;
            visited_o   = visited_r;
        } else 
        {
            dist_c      = dist_r;
            km_dist_c   = km_dist_r;
            prev_c      = prev_r;
            visited_c   = visited_r;
            cur_node    = pop(heap_r);
            u           = cur_node.nodeIndex;
            neighbors   = &adj_r[u];
            heap_c      = heap_r;
            visited_o   = visited_f;
        }

        // if visited[u] skip
        if (visited_c[u] == 1) {continue;}

        visited_c[u] = 1;

        if (dist_c[u] >= best_cost) break;

        // update best_meeting and best_cost if it is the new best
        if (visited_o[u] == 1)
        {
            if (dist_f[u] + dist_r[u] < best_cost)
            {
                best_meeting = u;
                best_cost = dist_f[u] + dist_r[u];
            }
        }

        // for all neighbouts of u
        for (int i = 0; i < neighbors->count; i++)
        {
            // get neighbour index (v)
            long long v = neighbors->edges[i].dst_index;

            if (v == -1) continue;
            if (ch_g->rank[v] <= ch_g->rank[u]) continue;
            if (visited_c[v] == 1) continue;

            // calculate alternative distance = dist[u] + edge weight
            double time;
            if (neighbors->edges[i].speed_limit == 0) {
                time = neighbors->edges[i].weight;
            } else {
                double speed_ms = neighbors->edges[i].speed_limit / 3.6;
                time = neighbors->edges[i].weight / speed_ms;
            }
            double alt = dist_c[u] + time;

            // if alternative < dist[v]:
            if (alt < dist_c[v])
            {
                dist_c[v] = alt;
                prev_c[v] = u;
                km_dist_c[v] = km_dist_c[u] + neighbors->edges[i].km_weight;
                
                push(heap_c, alt, v);
            }
        }
    }
    // Check meeting point — same as bidirectional A*

    // Reconstruct path using prev_f and prev_r

    if (best_meeting == -1) goto cleanup;

    path_f = malloc(g->node_count * sizeof(long long));
    if (!path_f) goto cleanup;

    long long len_f = 0;
    long long cur = best_meeting;

    // walk backwards to get count
    while (cur != -1) {
        path_f[len_f++] = cur;
        cur = prev_f[cur];
    }

    // reverse so it goes src -> best_meeting
    for (long long l = 0, r = len_f - 1; l < r; l++, r--) {
        long long tmp = path_f[l]; path_f[l] = path_f[r]; path_f[r] = tmp;
    }

    // walk from best meeting to dst
    path_r = malloc(g->node_count * sizeof(long long));
    if (!path_r) goto cleanup;

    long long len_r = 0;
    cur = prev_r[best_meeting];  // skip best_meeting, already in path_f
    while (cur != -1) {
        path_r[len_r++] = cur;
        cur = prev_r[cur];
    }

    // combine into one path
    long long total = len_f + len_r;
    path = malloc(total * sizeof(long long));
    if (!path) goto cleanup;

    for (long long j = 0; j < len_f; j++) 
    {
        path[j] = path_f[j];
    }
    
    for (long long j = 0; j < len_r; j++)
    {
        path[len_f + j] = path_r[j];
    }



    result = malloc(sizeof(ResultPath));
    if (!result) goto cleanup;

    t = clock() - t;

    result->name = "CH-Query*";
    result->path_inx = path;
    result->time_in_seconds = best_cost;
    result->distance_in_metres = km_dist_f[best_meeting] + km_dist_r[best_meeting];
    result->load_time_in_seconds = ((double)t / CLOCKS_PER_SEC);

    // free memory
    cleanup:
        freeHeap(heap_f);   freeHeap(heap_r);
        free(dist_f);       free(dist_r);
        free(prev_f);       free(prev_r);
        free(visited_f);    free(visited_r);
        free(km_dist_f);    free(km_dist_r);
        free(path_f);       free(path_r);

    return result;
}
