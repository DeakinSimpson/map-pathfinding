#ifndef RTREE_H
#define RTREE_H

#include "graph.h"

/*
Minimum bounding box for the R-Tree

If the node is outside of this area, it will not be included in the tree
*/
typedef struct {
    double min_lat;
    double max_lat;
    double min_lon;
    double max_lon;
} MinimumBoundingRectangle;

/*
each node in the tree has its own Minimum Bounding Area for all the nodes that it contains

each node is either a leaf, or an internal node (not a leaf)

children count is the number of children that this node has

MAX_CHILDREN is the maximum number of children nodes that the node can have
    if this is too small the tree will be deep

    if it is too big the tree will be too shallow

all internal nodes contain other R-tree nodes

whereas leaf nodes contain map/road nodes
*/
#define MAX_CHILDREN 16

typedef struct RTreeNode {
    MinimumBoundingRectangle mbr;
    int is_leaf;
    int children_count;

    struct RTreeNode *children[MAX_CHILDREN];   // used if internal node (not leaf)
    long long entries[MAX_CHILDREN];            // used if leaf node
} RTreeNode;

/*
root is just a pointer to the root of the tree

size is the total number of nodes in the map tree 

(this is because some can get disregarded if they are outside the MBR)
*/
typedef struct {
    RTreeNode *root;
    long long size;
} RTree;

void merge(long long *indicies, long long left, long long mid, long long right, Graph *g, int sort_by_lon);
void merge_sort(long long *indicies, long long left, long long right, Graph *g, int sort_by_lon);

RTree* rtree_build(Graph *g);
long long rtree_nearest(RTree *tree, Coordinate coord, Graph *g, AdjList *adj);
// void rtree_range(RTree *rtree, MinimumBoundingRectangle mbr, long long *results, long long *count);
void rtree_free_node(RTreeNode *node);
void rtree_free(RTree *rtree);


#endif