#include "rtree.h"
#include "graph.h"
#include "stdlib.h"
#include "stdio.h"
#include "float.h"
#include <math.h>
#include <time.h>

/*
merges two halves back into one
*/
void merge(long long *indicies, long long left, long long mid, long long right, Graph *g, int sort_by_lon) {
    // get the length of the left and right half
    long long len_left = mid - left;
    long long len_right = right - mid;

    // create temporary arrays to hold the two halves
    long long *tmp_left = malloc(len_left * sizeof(long long));

    if (tmp_left == NULL) {
        printf("failed to allocate memory for tmp_left");
        return;
    }

    long long *tmp_right = malloc(len_right * sizeof(long long));

    if (tmp_right == NULL) {
        printf("failed to allocate memory for tmp_right");
        free(tmp_left);
        return;
    }

    // copy the two halves into the temporary arrays
    for (long long i = 0; i < len_left; i++) {
        tmp_left[i] = indicies[left + i];
    }

    for (long long i = 0; i < len_right; i++) {
        tmp_right[i] = indicies[mid + i];
    }

    long long i = 0;
    long long j = 0;
    long long k = left;

    while (i < len_left && j < len_right) {
        // get the value on the left to compare
        double val_left;

        if (sort_by_lon) {
            val_left = g->nodes[tmp_left[i]].lon;
        } else {
            val_left = g->nodes[tmp_left[i]].lat;
        }

        // get the value on the right to compare
        double val_right;

        if (sort_by_lon) {
            val_right = g->nodes[tmp_right[j]].lon;
        } else {
            val_right = g->nodes[tmp_right[j]].lat;
        }

        // merge the two halves
        if (val_left <= val_right) {
            indicies[k] = tmp_left[i];
            i++;
            k++;
        } else {
            indicies[k] = tmp_right[j];
            j++;
            k++;
        }
    }
    
    // copy remaining values
    while (i < len_left) {
        indicies[k++] = tmp_left[i++];
    }

    while (j < len_right) {
        indicies[k++] = tmp_right[j++];
    }

    // free malloc arrays
    free(tmp_left);
    free(tmp_right);
}

/*
will give indicies array which will be an index to the order of g

you can loop through g[indicies[i]] in order to get the sorted version

this 
*/
void merge_sort(long long *indicies, long long left, long long right, Graph *g, int sort_by_lon) {
    if (right - left <= 1) {
        return;
    }

    long long mid = (left + right) / 2;

    // sort left half
    merge_sort(indicies, left, mid, g, sort_by_lon);

    // sort right half
    merge_sort(indicies, mid, right, g, sort_by_lon);

    // merge the two halves
    merge(indicies, left, mid, right, g, sort_by_lon);
}

/*
gets the MBR for a node in a given graph
*/
void compute_mbr(RTreeNode *node, Graph *g) {
    node->mbr.min_lat = DBL_MAX;
    node->mbr.max_lat = -DBL_MAX;
    node->mbr.min_lon = DBL_MAX;
    node->mbr.max_lon = -DBL_MAX;

    if (node->is_leaf)
    {
        for (int i = 0; i < node->children_count; i++)
        {
            double lat = g->nodes[node->entries[i]].lat;
            double lon = g->nodes[node->entries[i]].lon;

            if (lat < node->mbr.min_lat) node->mbr.min_lat = lat;
            if (lat > node->mbr.max_lat) node->mbr.max_lat = lat;
            if (lon < node->mbr.min_lon) node->mbr.min_lon = lon;
            if (lon > node->mbr.max_lon) node->mbr.max_lon = lon;            
        }
    } else
    {
        for (int i = 0; i < node->children_count; i++)
        {
            MinimumBoundingRectangle *c = &node->children[i]->mbr;

            if (c->min_lat < node->mbr.min_lat) node->mbr.min_lat = c->min_lat;
            if (c->max_lat > node->mbr.max_lat) node->mbr.max_lat = c->max_lat;
            if (c->min_lon < node->mbr.min_lon) node->mbr.min_lon = c->min_lon;
            if (c->max_lon > node->mbr.max_lon) node->mbr.max_lon = c->max_lon;
        }
    }
}

RTree* rtree_build(Graph *g)
{
    /*
        --- step 1 ---
        sory all nodes by longitude
        [A, B, C, D, E, F, G, H, I]
    */

    // create the array indicies, which is the index to the graph g
    long long *indicies = malloc(g->node_count * sizeof(long long));

    if (indicies == NULL) {
        printf("failed to allocate memory for indicies");
        free(indicies);
        return NULL;
    }

    for (long long i = 0; i < g->node_count; i++) {
        indicies[i] = i;
    }

    // sort by longitute
    merge_sort(indicies, 0, g->node_count, g, 1);
    
    /*
        --- step 2 ---
        split into column based on MAX_CHILDREN
        [A, B, C] [D, E, F] [G, H, I]
    */

    /*
    get the toal leaves (each leaf holds MAX_CHILDREN)

    to get a even distribution we get the square root as it splits it into a grid of leafes, this reduces overlap

    this results in the total number of slices needed to get a balances tree
    */
    RTree *tree = NULL;

    long long total_leaves = (g->node_count + MAX_CHILDREN - 1) / MAX_CHILDREN;

    long long total_nodes = total_leaves;
    long long level_count = total_leaves;
    while (level_count > 1)
    {
        level_count = (level_count + MAX_CHILDREN -1) / MAX_CHILDREN;
        total_nodes += level_count;
    }

    RTreeNode *pool = malloc(total_nodes * sizeof(RTreeNode));
    long long pool_used = 0;

    long long num_slice = (long long)ceil(sqrt((double)total_leaves));
    long long slice_size = num_slice * MAX_CHILDREN;
    
    // create an array of pointers to leaves
    RTreeNode **leaves = malloc(total_leaves * sizeof(RTreeNode*));
    long long leaf_count = 0;

    for (long long slice = 0; slice < num_slice; slice++) {
        /*
            --- step 3 ---
            within each longitude slice, sory by latitude
            [A, B, C] sorted south to north
        */
        long long start = slice * slice_size;
        long long end = (slice * slice_size) + slice_size;

        if (end > g->node_count) {
            end = g->node_count;
        }

        merge_sort(indicies, start, end, g, 0);

        /*
            --- steo 4 ---
            group every MAX_CHILDREN nodes into leaves

            compute minimum bounding range
        */
        long long num_of_leaves = (end - start + MAX_CHILDREN - 1) / MAX_CHILDREN;
        for (long long leaf = 0; leaf < num_of_leaves; leaf++) {
            RTreeNode *cur_leaf = &pool[pool_used++];

            cur_leaf->is_leaf = 1;

            // get start and end position of the leaf, used to allocate choldren
            long long leaf_start = start + leaf * MAX_CHILDREN;
            long long leaf_end = leaf_start + MAX_CHILDREN;

            // make sure leaf_end is not larger the slice
            if (leaf_end > end) {
                leaf_end = end;
            }

            // assign the children count to leaf
            cur_leaf->children_count = leaf_end - leaf_start;

            // assign the children to the leaf node
            for (long long child = 0; child < cur_leaf->children_count; child++) {
                cur_leaf->entries[child] = indicies[leaf_start + child];
            }

            compute_mbr(cur_leaf, g);

            // add leaf to the leaf array
            leaves[leaf_count++] = cur_leaf;
        }
    }
    /*
        --- step 5 ---

        group leaves into internal nodes

        computer their mbr's

        repeat until at root
    */
    RTreeNode **cur_level = leaves;
    long long leaf_level_count = total_leaves;

    while (leaf_level_count > 1) {
        long long new_count = (leaf_level_count + MAX_CHILDREN - 1) / MAX_CHILDREN;
        RTreeNode **new_level = malloc(new_count * sizeof(RTreeNode*));

        if (new_level == NULL) {
            printf("failed to allocate memory for new_level\n");
            return NULL;
        }

        for (long long i = 0; i < new_count; i++) {
            RTreeNode *internal_node = &pool[pool_used++];

            internal_node->is_leaf = 0;
            internal_node->children_count = 0;

            long long current_pos = i * MAX_CHILDREN;
            
            // add children nodes to internal_node
            for (int j = 0; (j < MAX_CHILDREN) && (current_pos + j < leaf_level_count); j++) {
                internal_node->children[j] = cur_level[current_pos + j];
                internal_node->children_count++;
            }

            compute_mbr(internal_node, g);

            new_level[i] = internal_node;
        }

        free(cur_level);
        cur_level = new_level;
        leaf_level_count = new_count;
    }

    tree = malloc(sizeof(RTree));
    if (!tree) goto cleanup;

    tree->root = cur_level[0];
    tree->size = g->node_count;
    tree->pool = pool;

    cleanup:
        free(indicies);
        free(cur_level);

    return tree;
}

// gets the minimum distance from mbr to node squared
double mbr_min_dist_sq(Coordinate coord, MinimumBoundingRectangle *mbr) {
    // set distances to 0
    double dlat = 0.0, dlon = 0.0;

    if      (coord.lat < mbr->min_lat) dlat = mbr->min_lat - coord.lat;
    else if (coord.lat > mbr->max_lat) dlat = coord.lat - mbr->max_lat;
    if      (coord.lon < mbr->min_lon) dlon = mbr->min_lon - coord.lon;
    else if (coord.lon > mbr->max_lon) dlon = coord.lon - mbr->max_lon;

    return dlat * dlat + dlon * dlon;
}

/*
finds the closest distance squared

this is done because performing a
square root operation on every node
can become very costly, as we are
only looking to compare the distance
and not get the actual distance we
can just compare the squared distance
*/
void rtree_nearest_node(RTreeNode *node, Coordinate coord, Graph *g, AdjList *adj, long long *best_index, double *best_dist) {
    if (node->is_leaf == 1) {
        for (int i = 0; i < node->children_count; i++) {
            long long idx = node->entries[i];
            
            // check if the value has edges
            if (adj[idx].count == 0) {
                continue;
            }

            // get distance in lat and lon
            double dlat = g->nodes[idx].lat - coord.lat;
            double dlon = g->nodes[idx].lon - coord.lon;

            // get distance
            double d = dlat * dlat + dlon * dlon;
            
            // check if it is closer
            if (d < *best_dist) {
                *best_dist = d;
                *best_index = idx;
            }
        }
    } else {
        // check each node in the node if its internal
        for (int i = 0; i < node->children_count; i++) {
            // only do this if the node is within the square
            double d = mbr_min_dist_sq(coord, &node->children[i]->mbr);

            // if the distance is better then make it best
            if (d < *best_dist) {
                rtree_nearest_node(node->children[i], coord, g, adj, best_index, best_dist);
            }
        }
    }
}

long long rtree_nearest(RTree *tree, Coordinate coord, Graph *g, AdjList *adj) {
    clock_t t = clock();

    long long best_index = -1;
    double best_dist = DBL_MAX;

    rtree_nearest_node(tree->root, coord, g, adj, &best_index, &best_dist);

    t = clock() - t;
    printf("time taken to find node rtree: %lld, %fs\n", best_index, (double)t / CLOCKS_PER_SEC);

    return best_index;
}

// void rtree_range(RTree *rtree, MinimumBoundingRectangle mbr, long long *results, long long *count) {

// }

// frees a rtree node
void rtree_free_node(RTreeNode *node) {
    if (node->is_leaf == 0) {

        for (int i = 0; i < node->children_count; i++) {

            rtree_free_node(node->children[i]);

        }
    }

    free(node);
}

// free entire rtree recursively
void rtree_free(RTree *rtree) {
    rtree_free_node(rtree->pool);
    free(rtree);
}