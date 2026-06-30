#include "dijkstra.h"

long long* dijkstra(Graph *g, AdjList *adj, HashMap *map, long long src_id, long long dst_id) {
    // get src and dst array indices from hashmap

    // allocate dist array (size node_count), set all to DBL_MAX

    // allocate prev array (size node_count), set all to -1

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
}