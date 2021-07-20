
#include <pthread.h>

typedef struct cbuf
{
  char **info;  //will hold the data of the circular buffer
  int size;
  int head;
  int end;
  int items;  //counter of items existing in circular buffer
}cbuf;

// Create circular buffer of fixed size <size>.
struct cbuf *cb_init(int size);

// Insert directories in buffer
void cb_place(struct cbuf *cb,char *country_name);

// Remove directory from buffer
char *cb_remove(struct cbuf *cb);

// Free buffer
void cb_destroy(struct cbuf *cb);

//functions that check if we have a full or empty buffer respectively
bool cb_empty(struct cbuf *cb);
bool cb_full(struct cbuf *cb);
