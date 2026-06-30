#include "heap.h"
#include<stdlib.h>

MinHeap* createHeap(long long capcity) {
    MinHeap* heap = malloc(sizeof(MinHeap));
    if (heap == NULL) {
        printf("Failed to allocate memory for heap\n");
        return NULL;
    }

    heap->node = malloc(capcity * sizeof(HeapNode));
    if (heap->node == NULL) {
        printf("Failed to allocate memory for heap node\n");
        free(heap);
        return NULL;
    }

    heap->size = 0;
    heap->capacity = capcity;
    return heap;
}

void swap(HeapNode* a, HeapNode* b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void push(MinHeap* heap, double dist, long long nodeIndex) {
    if (heap->size >= heap->capacity) {
        heap->capacity *= 2;
        HeapNode* newNode = realloc(heap->node, heap->capacity * sizeof(HeapNode));
        if (newNode == NULL) {
            printf("realloc failed at capacity %lld\n", heap->capacity);
            fflush(stdout);
            exit(1);
        }
        heap->node = newNode;
    }

    heap->node[heap->size].dist = dist;
    heap->node[heap->size].nodeIndex = nodeIndex;
    heap->size++;

    long long i = heap->size - 1;
    while (i > 0) {
        long long parent = (i - 1) / 2;
        if (heap->node[parent].dist > heap->node[i].dist) {
            swap(&heap->node[parent], &heap->node[i]);
            i = parent;
        } else break;
    }
}

HeapNode pop(MinHeap* heap) {
    if (heap->size == 0) {
        printf("Cannot pop from empty heap\n");
        HeapNode empty = {-1, -1};
        return empty;
    }
    // save the top value (the minimum)
    HeapNode min = heap->node[0];

    // move the last item to position 0
    heap->node[0] = heap->node[--heap->size];

    // bubble dowm
    long long i = 0;
    while (1) {
        long long left = 2 * i + 1;
        long long right = 2 * i + 2;
        long long smallest = i;

        if (left < heap-> size && heap->node[left].dist < heap->node[smallest].dist) smallest = left;
        if (right < heap-> size && heap->node[right].dist < heap->node[smallest].dist) smallest = right;

        if (smallest != i) {
            swap(&heap->node[smallest], &heap->node[i]);
            i = smallest;
        } else break;
        
    }

    return min;
}

void freeHeap(MinHeap* heap) {
    free(heap->node);
    free(heap);
}