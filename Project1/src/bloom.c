#include "../includes/bloom.h"
#include "../includes/hashes.h"

//the bloom filter is a representation of the set of all vaccinated persons.

void HashGenerator(struct bloom_filter *_bloom_filter, unsigned char *str) {
    for (unsigned int i = 0; i < _bloom_filter->k; i++) {
        _bloom_filter->hash_values[i] = hash_i(str, i);
    }
}

struct bloom_filter *Create(int hashes, int filter_size) {
    struct bloom_filter *_bloom_filter = malloc(sizeof(*_bloom_filter));
    _bloom_filter->k = hashes;
    _bloom_filter->bloom_size = 8 * filter_size;  // size in bits
    _bloom_filter->bit_array = calloc(ceil(_bloom_filter->bloom_size / 8),sizeof(uint8_t));// calloc instead of malloc because it will
                                                                                        // initialise the bit_array to 0.
    _bloom_filter->hash_values = malloc(_bloom_filter->k * sizeof(unsigned long));
    return _bloom_filter;
}

// insert operation
void Set(struct bloom_filter *_bloom_filter, unsigned char *str) {
    HashGenerator(_bloom_filter, str);
    int index;
    // we must find the 8-bit word that the index we are talking about is placed
    for (int i = 0; i < _bloom_filter->k; i++) {
        index = _bloom_filter->hash_values[i] % _bloom_filter->bloom_size;
        _bloom_filter->bit_array[index / 8] |= (1 << (index % 8));
    }
}
// loookup operation
bool Search(struct bloom_filter *_bloom_filter, unsigned char *str) {
    HashGenerator(_bloom_filter, str);
    int index;
    bool res = true;
    for (int i = 0; i < _bloom_filter->k; i++) {
        index = _bloom_filter->hash_values[i] % _bloom_filter->bloom_size;

        if (_bloom_filter->bit_array[index / 8] & (1 << (index % 8)))
            res = true;
        else {
            //found bit that is 0 so record surely doesn't exist in the bloom filter.
            res = false;
            break;
        }
    }
    return res;
}

void Destructor(struct bloom_filter *_bloom_filter) {
    if (!_bloom_filter) 
        return;

    free(_bloom_filter->hash_values);
    free(_bloom_filter->bit_array);
    free(_bloom_filter);
}