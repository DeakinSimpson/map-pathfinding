#include "rtree.h"
#include "graph.h"
#include "stdlib.h"
#include "stdio.h"

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

// RTree* rtree_build(Graph *g) {
//     /*
//         --- step 1 ---
//         sory all nodes by longitude
//         [A, B, C, D, E, F, G, H, I]
//     */

//     /*
//         --- step 2 ---
//         split into column based on MAX_CHILDREN
//         [A, B, C] [D, E, F] [G, H, I]
//     */

//     /*
//         --- step 3 ---
//         within each longitude slice, sory by latitude
//         [A, B, C] sorted south to north
//     */

//     /*
//         --- steo 4 ---
//         group every MAX_CHILDREN nodes into leaves

//         compute minimum bounding range
//     */

//     /*
//         --- step 5 ---

//         group leaves into internal nodes

//         computer their mbr's

//         repeat until at root
//     */
// }



// long long rtree_nearest(RTree rtree, Coordinate coord) {

// }

// void rtree_range(RTree *rtree, MinimumBoundingRectangle mbr, long long *results, long long *count) {

// }

// void rtree_free(RTree *rtree) {

// }