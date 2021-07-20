#include "../includes/DB_Hashes.h"
#include "../includes/types.h"

/* Implementation of a linear probing hashtable.According to theory the sizes 
   of the hashtable (when rehashing have to be prime numbers.Also after experiments it has been
   found that the most effective sizes for the hashtable are the ones included in the prime_sizes array in the types.h file)
*/

struct C_Bucket *CHTCreate(int flg) {
   //size index is a global variable, in order to get the next size for rehashing from the prime_sizes array
    size_index=0;
    size=prime_sizes[size_index];
    struct C_Bucket *ht = malloc(size*sizeof(*ht));
    for (int i = 0; i < size; i++){ 
      ht[i].key = malloc(sizeof(char *));
      strcpy(ht[i].key," ");
      ht[i].state = EMPTY;
    }
    
   ht->inserted=0;
   return ht;
}

struct C_Bucket * UpdateCountryPtr(struct C_Bucket *ht,int thesis,struct Bucket *country){
   if(thesis == -1)
      return ht;
   ht[thesis].value->country = country;
   return ht;
}

struct C_Bucket * CHTInsert(pVoid _rec,struct C_Bucket *ht) {
   int index_1 = hash_i((*(citizen_Record *)_rec).ID,1)%size;
   int pos;
   bool in_map = false;
   //if record in hashtable, we get the index of its position and return
   int tmp=CHTSearch((*(citizen_Record *)_rec).ID,ht);
   if(tmp!=-1){
      _index=tmp;
      //record found in ht
      h_found=true;
      return ht;
   }
   
   // we start searching for an empty space to insert from the hashed value-index of the key.We do linear probing, returning to the start of the array.
   for (pos = hash_i((*(citizen_Record *)_rec).ID,1)%size; ht[pos].state == OCCUPIED;	pos = (pos + 1) % size) {
      if (strcmp(ht[pos].key,(*(citizen_Record *)_rec).ID) == 0) {      
         in_map = true;
         _index=pos;
         break;
      }
   }
   if (in_map) {
      h_found=true;
      _index=pos;
      return ht;
   } 
   else {
      //insert new citizen
      h_found=false;
      ht[pos].key = realloc(ht[pos].key,strlen((*(citizen_Record *)_rec).ID)*sizeof(char *));
      strcpy(ht[pos].key,(*(citizen_Record *)_rec).ID);
      ht[pos].value=_rec;
      ht[pos].state = OCCUPIED;
      _index=pos;
   
      ht->inserted++;
      //if below statement is true then rehash
      //I found out that it's more effective to reahash when you have reached the 80% of occupied cells
      if(ht->inserted>=((int)(0.8 * size))){
         size_index++;
         int tmp=size;
         size=prime_sizes[size_index];
         ht =realloc(ht,size*sizeof(*ht));
         for(int j=tmp; j<size; j++){
            ht[j].key = malloc(sizeof(char *));
            strcpy(ht[j].key," ");
            ht[j].state = EMPTY;
         }
      }   
      return ht;
   }
}

int CHTSearch(char *key, struct C_Bucket *ht){

   int index_1 = hash_i(key,1)%size;
   for(int i=0; i<size; i++){
      if(strcmp(ht[i].key,key)==0)
         return i;
   }
   return -1;
}

void CHTDelete(struct C_Bucket *ht, int flg){    
   for(int i=0; i<size; i++){
      if(ht[i].state!=EMPTY){
         free(ht[i].key);
         free(ht[i].value->ID);
         free(ht[i].value->name);
         free(ht[i].value->surname);
         free(ht[i].value);
      }   
      if(ht[i].state==EMPTY)
         free(ht[i].key);
   }
   free(ht);
}