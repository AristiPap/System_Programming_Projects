#include "../includes/Skip_Lists.h"
#include "../includes/CommonStructures.h"

//if it returns 1 then we keep going up a level at insertion.
//If it returns 0 we break and we dont go up a level
static int coin_toss(){
    return rand()%2;
}

skiplist *skiplist_init(int height) {
    skiplist *list=malloc(sizeof(*list));
    snode *header =malloc(sizeof(struct snode));
    list->header = header;
    header->key = INT_MAX;
    header->forward = malloc(sizeof(snode*) * (height + 1));
    for (int i = 0; i <= height; i++) {
        header->forward[i] = list->header;
    }
    list->level = 1;
    //max height of skip list.Its the log of the number of records in the input file
    list->height = height;
    list->size = 0;
    list->header = header;
    return list;
}

int skiplist_insert(skiplist *list, int key, pVoid value, pVoid _date, size_t data_size){
    snode **update;
    update=calloc((list->height+1),sizeof(snode*)); 
    snode *x = list->header;
    int i, level;
    //we start from the highest level of the skip list and iterate through the level while key is greater than key of node next to current
    //Otherwise we insert current in update and move one level down.
    for (i = list->level; i >= 1; i--) {
        while (x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];
    //key already exists,so update value and date(if we are at the vaccinated skipped list)
    if (key == x->key) {
        x->value = malloc(data_size);
        if(_date!=NULL){
            x->date = malloc((strlen(_date)+1)*sizeof(*x->date));
            strcpy(x->date,_date);
        }
        else
            x->date=NULL;
        memcpy(x->value,value,data_size);
        free(update);
        return 0;
    } else {
        x = malloc(sizeof(*x));
        x->key = key;
        x->value = malloc(data_size);
        if(_date!=NULL){
            x->date = malloc((strlen(_date)+1)*sizeof(*x->date));
            strcpy(x->date,_date);
        }
        else
            x->date=NULL;
        memcpy(x->value,value,data_size);
        x->forward = malloc(sizeof(*x->forward));
        //we call the coin_toss function.As long as it returns 1 then we add the key to a higher level.
        //if not then break the loop.
        for(int k=1; k<=list->height; k++){
            level=coin_toss();
            if(level==0 && k>1)
                break;
            if(level==1){
                if(k>list->level){
                    list->level++;
                    update[list->level]=list->header;
                }
            }    
            x->forward = realloc(x->forward, (k+1)*sizeof(*x->forward));
            x->forward[k] = update[k]->forward[k];
            update[k]->forward[k] = x;
            if(level==0 && k==1)
                break;
        }
        list->size++;
    }
    free(update);
    return 0;
}

//moving top down from the current top level to the base level=1 we search for the key
//if at the current level iterating, we find key that is of higher value than the key of search, we go down one level.
//Finally we check if the key that we found is equal to the key that we are searching for.
snode *skiplist_search(skiplist *list, int key) {

    snode *n = list->header;

    for (int i=list->level; i >= 1; i--) {
        while (n->forward[i]->key < key)
            n = n->forward[i];
    }

    if (n->forward[1]->key == key)
        return n->forward[1];
    else
        return NULL;
    
    return NULL;
}

static void skiplist_node_free(snode *n) {

    if (n) {
        free(n->value);
        if(n->date!=NULL)
            free(n->date);
        free(n->forward);
        free(n);
    }
}

int skiplist_delete(skiplist *list, int key) {
    snode *current = list->header;  
    // create update array and initialize it 
    snode **update;
    update=calloc((list->height+1),sizeof(snode*)); 
    //we start from the highest level of the skip list and iterate through the level while key is greater than key of node next to current
    //Otherwise we insert current in update and move one level down.
    for(int i = list->level; i >= 1; i--) 
    { 
        while(current->forward[i] != NULL  && current->forward[i]->key < key) 
            current = current->forward[i]; 
        update[i] = current; 
    } 
  
    current = current->forward[1]; 
    if(current != NULL && current->key == key) 
    { 
        for(int i=1;i<=list->level;i++) 
        { 
            if(update[i]->forward[i] != current) 
                break;   
            update[i]->forward[i] = current->forward[i]; 
        } 
        skiplist_node_free(current);
        list->size--;
        // Remove levels having no elements  
        while(list->level>1 && list->header->forward[list->level] == 0) 
            list->level--; 
    } 
    free(update);
}

int skiplist_destroy(skiplist *list){
    if(list->size==0){
        free(list->header->forward);
        free(list->header);
        free(list);
        return 0;
    
    }
    else{
        int iter=list->size;
        snode *node = list->header; 
        node = node->forward[1]; 
        snode *next = node->forward[1];    
        for(int i=0; i<iter; i++)  
        { 
            skiplist_delete(list,node->key);
            node=next;
            next=node->forward[1];
        }  
        free(list->header->forward);
        free(list->header);
        free(list);   
    }
}
