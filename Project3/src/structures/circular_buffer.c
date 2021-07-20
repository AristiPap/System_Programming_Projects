#include <stdlib.h>
#include <stdbool.h>
#include "../include/circular_buffer.h"
#include <string.h>

// Create circular buffer
struct cbuf *cb_init(int size)
{
  struct cbuf *cb = malloc(sizeof(struct cbuf));
  cb->size = size;
  cb->info =  malloc((size)*sizeof(char *));
  cb->head = cb->items = 0;
  cb->end = -1;
  return cb;
}

void cb_place(struct cbuf *cb,char *country_name){

  cb->end = (cb->end + 1) % cb->size;
  cb->info[cb->end] = country_name;
  ++cb->items;
}

// Remove and return an item from cb
char *cb_remove(struct cbuf *cb)
{
  int fd = 0;
  char *tmp = cb->info[cb->head];
  cb->head = (cb->head + 1) % cb->size;
  --cb->items;
  return tmp;
}

// Destroy buffer
void cb_destroy(struct cbuf *cb) {
  //for(int i=0; i<cb->size; i++)
  //  free(cb->info[i]);
  free(cb->info);
  free(cb);
}


bool cb_empty(struct cbuf *cb){
  if(cb->items==0)
    return true;
  else
    return false;
}

bool cb_full(struct cbuf *cb){
  if(cb->items== cb->size)
    return true;
  else
    return false;
}