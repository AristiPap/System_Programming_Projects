#include "../includes/DB_Hashes.h"

//function to create bucket of country (node of country list) and create its list of citizen pointers
struct Bucket *CreateBucket(char *country,int id){
   struct Bucket *bu=malloc(sizeof(*bu));
   strcpy(bu->key,country);
   bu->id_country=id;
   bu->citizens=list_create();
   return bu;
}