#include "astar.h"
#include "hashmap.h"
#include "graph.h"
#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>      // used for DBL_MAX
#include <time.h>
#include <math.h>

double heuristic(Coordinate a, Coordinate b) {
    // one degree in lat and lon is about 111km this is super innacurate however is faster then computing
    double lat = (a.lat - b.lat) * 111.0;
    double lon = (a.lon - b.lon) * 111.0;

    double distance = lat * lat + lon * lon;

    return distance;
}

ResultPath* astar(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id) {
    clock_t t = clock();
    long long src_index = -1;
    long long dst_index = -1;
    double *dist = NULL;
    long long *prev = NULL;
    long long* visited = NULL;
    double *km_dist = NULL;
    MinHeap *heap = NULL;
    ResultPath *result = NULL;

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

    // create heap and push src with distance 0
    heap = createHeap(1024);
    if (!heap) goto cleanup;


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
    push(heap, 0, src_index);
    
    // while heap is not empty:
    while (heap->size > 0)
    {
        // pop the node with smallest distance (u)
        HeapNode cur_node = pop(heap);
        long long u = cur_node.nodeIndex;

        // if visited[u] skip
        if (visited[u] == 1) continue;
        visited[u] = 1; // mark visited[u] = 1
        if (u == dst_index) break; // if u == dst_index break early 

        AdjList *neighbors = &adj[u];
        // loop through all neighbours of u in adj[u]:
        for (int i = 0; i < neighbors->count; i++)
        {
            // get neighbour index (v)
            long long v = neighbors->edges[i].dst_index;;

            // if visited[v] skip
            if (v == -1 || visited[v] == 1) continue;

            // calculate alternative distance = dist[u] + edge weight
            double speed_ms = neighbors->edges[i].speed_limit / 3.6;
            double time = neighbors->edges[i].weight / speed_ms;
            double alt = dist[u] + time;

            // if alternative < dist[v]:
            if (alt < dist[v])
            {
                dist[v] = alt;
                prev[v] = u;
                km_dist[v] = km_dist[u] + neighbors->edges[i].weight;
                
                Coordinate c_coord = {g->nodes[v].lat, g->nodes[v].lon};
                Coordinate d_coord = {g->nodes[dst_index].lat, g->nodes[dst_index].lon};
                
                double dist_heur = heuristic(c_coord, d_coord);
                double speed_ms_heur = 100.0 / 3.6;
                double time_heur = dist_heur / speed_ms_heur;
                double heuristic = dist[v] + time_heur;

                push(heap, heuristic, v);
            }
        }
    }

    if (dist[dst_index] == DBL_MAX) goto cleanup;

    // reconstruct path by walking prev[] from dst back to src
    long long *path = malloc(g->node_count * sizeof(long long));
    long long cur_index = dst_index;
    long long i = 0;

    while (cur_index != -1)
    {
        path[i] = cur_index;
        cur_index = prev[cur_index];
        i++;
    }

    // reverse path so it goes from src to dst
    for (long long l = 0, r = i - 1; l < r; l++, r--)
    {
        long long tmp = path[l];
        path[l] = path[r];
        path[r] = tmp;
    }
    path = realloc(path, (i + 1) * sizeof(long long));

    // return path
    t = clock() - t;

    result = malloc(sizeof(ResultPath));
    result->name = "A*";
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

ResultPath* astar_bidir(Graph *g, AdjList *adj, AdjList *adj_r, HashMap *map, long long src_id, long long dst_id) {
    clock_t t = clock();
    long long   src_index   = -1;   long long   dst_index   = -1;
    double      *dist_f     = NULL; double      *dist_r     = NULL;
    long long   *prev_f     = NULL; long long   *prev_r     = NULL;
    long long   *visited_f  = NULL; long long   *visited_r  = NULL;
    double      *km_dist_f  = NULL; double      *km_dist_r  = NULL;
    MinHeap     *heap_f     = NULL; MinHeap     *heap_r     = NULL;
    long long   *path_f     = NULL; long long   *path_r     = NULL;
    long long   *path       = NULL;
    ResultPath  *result     = NULL;

    // get source index from node id
    src_index = hashmap_get(map, src_id);
    if (src_index == -1) goto cleanup;

    // get destination index from node id
    dst_index = hashmap_get(map, dst_id);
    if (dst_index == -1) goto cleanup;

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

    dist_f[src_index] = 0;
    dist_r[dst_index] = 0;
    km_dist_f[src_index] = 0;
    km_dist_r[dst_index] = 0;


    // create heap and push src with distance 0
    heap_f = createHeap(1024);
    if (!heap_f) goto cleanup;

    heap_r = createHeap(1024);
    if (!heap_r) goto cleanup;

    push(heap_f, 0, src_index);
    push(heap_r, 0, dst_index);

    long long best_meeting = -1;
    double best_cost = DBL_MAX;

    while (heap_f->size > 0 && heap_r->size > 0) 
    {
        double *dist_c;
        double *km_dist_c;
        long long *prev_c;
        long long *visited_c;
        HeapNode cur_node;
        long long u;
        AdjList *neighbors;
        MinHeap *heap_c;
        long long target_index;
        long long *visited_o;

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
            target_index= dst_index;
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
            target_index= src_index;
            visited_o   = visited_f;
        }

        // if visited[u] skip
        if (visited_c[u] == 1)
        {
            continue;
        }

        // mark visited[u] = 1
        visited_c[u] = 1;

        // check if node in other is visited or not
        if (visited_o[u] == 1) 
        {
            if (dist_f[u] + dist_r[u] < best_cost) 
            {
                best_meeting = u;
                best_cost = dist_f[u] + dist_r[u];
            }
            
            /*
            updated this to compare the actual distance rather then the theoretical 

            this is because it was checking 
            */
            if (dist_f[u] + dist_r[u] >= best_cost) {
                break;
            }
        }


        for (int i = 0; i < neighbors->count; i++)
        {
            // get neighbour index (v)
            long long v = neighbors->edges[i].dst_index;;

            // if visited[v] skip
            if (v == -1 || visited_c[v] == 1)
            {
                continue;
            }

            // calculate alternative distance = dist[u] + edge weight
            double speed_ms = neighbors->edges[i].speed_limit / 3.6;
            double time = neighbors->edges[i].weight / speed_ms;
            double alt = dist_c[u] + time;

            // if alternative < dist[v]:
            if (alt < dist_c[v])
            {
                dist_c[v] = alt;
                prev_c[v] = u;
                km_dist_c[v] = km_dist_c[u] + neighbors->edges[i].weight;
                
                Coordinate c_coord = {g->nodes[v].lat, g->nodes[v].lon};
                Coordinate d_coord = {g->nodes[target_index].lat, g->nodes[target_index].lon};
                
                double dist_heur = heuristic(c_coord, d_coord);
                double speed_ms_heur = 100.0 / 3.6;
                double time_heur = dist_heur / speed_ms_heur;
                double heuristic = dist_c[v] + time_heur;

                push(heap_c, heuristic, v);
            }
        }
    }

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

    // create final path
    path = malloc(total * sizeof(long long));
    if (!path) goto cleanup;

    // the first half of path is the path forward
    for (long long j = 0; j < len_f; j++) 
    {
        path[j] = path_f[j];
    }
    
    // the second half of the path is the reverse path
    for (long long j = 0; j < len_r; j++)
    {
        path[len_f + j] = path_r[j];
    }

    // allocate memory for result
    result = malloc(sizeof(ResultPath));
    if (!result) goto cleanup;

    t = clock() - t;

    // assign result variables
    result->name = "Bi-Directional A*";
    result->path_inx = path;
    result->time_in_seconds = best_cost;
    result->distance_in_metres = km_dist_f[best_meeting] + km_dist_r[best_meeting];
    result->load_time_in_seconds = ((double)t / CLOCKS_PER_SEC);

    // free memory
    cleanup:
        freeHeap(heap_f);
        freeHeap(heap_r);
        free(dist_f);
        free(dist_r);
        free(prev_f);
        free(prev_r);
        free(visited_f);
        free(visited_r);
        free(km_dist_f);
        free(km_dist_r);
        free(path_f);
        free(path_r);

    return result;    
}