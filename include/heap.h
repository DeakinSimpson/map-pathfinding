#ifndef HEAP_H
#define HEAP_H

#include<stdio.h>

typedef struct {
    double dist;
    long long nodeIndex;
} HeapNode;

typedef struct {
    HeapNode* node;
    long long size;     // the current size of the heap
    long long capacity; // the max capacity of the heap
} MinHeap;

MinHeap* createHeap(long long capcity);
void swap(HeapNode* a, HeapNode* b);
void push(MinHeap* heap, double dist, long long nodeIndex);
HeapNode pop(MinHeap* heap);
void freeHeap(MinHeap* heap);

#endif