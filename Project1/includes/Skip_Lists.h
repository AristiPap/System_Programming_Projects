// Aristi Papastavrou
// AM:1115201800154
#pragma once

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define SKIPLIST_MAX_LEVEL 20
typedef void *pVoid;


typedef struct snode {
    int key; //citizen id
    pVoid value; //pointer to citizen Record
    pVoid date;
    struct snode **forward;

} snode;

typedef struct skiplist {
    int level;
    int size;
    int height;
    struct snode *header;
} skiplist;

skiplist *skiplist_init(int height);
int skiplist_insert(skiplist *list, int key, pVoid value, pVoid _date, size_t data_size);
snode *skiplist_search(skiplist *list, int key);
int skiplist_delete(skiplist *list, int key);
int skiplist_destroy(skiplist *list);
