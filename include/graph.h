#ifndef GRAPH_H
#define GRAPH_H

// an enum storing all the roadtypes possible for osm
enum RoadType {
    Motorway,
    MotorwayLink,
    Trunk,
    TrunkLink,
    Primary,
    PrimaryLink,
    Secondary,
    SecondaryLink,
    Tertiary,
    TertiaryLink,
    Unclassified,
    Residential,
    LivingStreet,
    Service
};

// define the Node structure
typedef struct {
    long long id;
    double lat;
    double lon;
} Node;

// define the Edge structure
typedef struct {
    long long src;
    long long dst;
    double weight;
    enum RoadType road_type;
    int one_way;
    int speed_limit;
} Edge;

// define the Graph stucture, containing all Nodes and Edges
typedef struct {
    Node* nodes;
    Edge* edges;
    long long node_count;
    long long edge_count;
} Graph;

typedef struct {
    double lat;
    double lon;
} Coordinate;

typedef struct {
    long long *path_inx;
    double time_in_seconds;
    double distance_in_metres;
    double load_time_in_seconds;
} ResultPath;

#include "adjacency.h"

// define functions for graph_c
Graph* graph_create(long long node_capacity, long long edge_capacity);
void graph_free(Graph* g);
Graph* graph_load(const char* path);
double to_rad(double deg);
double haversine(Coordinate coord1, Coordinate coord2);
void result_path_free(ResultPath *rp);

#endif