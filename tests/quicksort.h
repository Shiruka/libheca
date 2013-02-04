#ifndef QSORT_DEF_H_
#define QSORT_DEF_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define CHUNK_SIZE 128
struct chunk {
    int d[CHUNK_SIZE];
};
static struct chunk *arr;

struct recurse_sort_data {
    unsigned long left;
    unsigned long right;
    int nesting;
};

static inline void set_chunk(struct chunk *ch, int d)
{
    int i;

    for (i = 0; i < CHUNK_SIZE; i++)
        ch->d[i] = d;
}

static inline int cmp(struct chunk *a, struct chunk *b)
{
    return a->d[0] > b->d[0];
}

static inline void swap(unsigned long a, unsigned long b)
{
    int tmp = arr[a].d[0];

    set_chunk(&arr[a], arr[b].d[0]);
    set_chunk(&arr[b], tmp);
}

int quicksort(void *, unsigned long);

#endif

