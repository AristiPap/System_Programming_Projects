#pragma once

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct bloom_filter {
    int bloom_size;
    int k;  // number of hash functions used in the bloom filter
    uint8_t *bit_array;
    unsigned long *hash_values;
} bloom_filter;

//unsigned long *hash_values;
void HashGenerator(struct bloom_filter *_bloom_filter, unsigned char *str);
struct bloom_filter *Create(int hashes, int filter_size);
void Set(struct bloom_filter *_bloom_filter, unsigned char *str);
bool Search(struct bloom_filter *_bloom_filter, unsigned char *str);
void Destructor(struct bloom_filter *_bloom_filter);