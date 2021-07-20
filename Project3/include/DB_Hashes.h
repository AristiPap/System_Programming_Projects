#pragma once
#include "CommonStructures.h"
#include "MyLinked_List.h"
#include "hashes.h"

typedef void *pVoid;

typedef enum {
    EMPTY, OCCUPIED, DELETED
} State;

struct C_Bucket{
    char *key; //citizen ID
    citizen_Record *value;
    State state;
    int inserted;
}C_Bucket;

int size;
int _index;
bool h_found;
int size_index;
/*int inserted;
int h_size;
int list_index;*/

struct Bucket *CreateBucket(char *country, int id);

struct C_Bucket *CHTCreate(int flg);
void CHTDelete(struct C_Bucket *ht, int flg);
int CHTSearch(char * key, struct C_Bucket *ht);
struct C_Bucket * CHTInsert(pVoid  _rec,struct C_Bucket *ht);
struct C_Bucket * UpdateCountryPtr(struct C_Bucket *ht,int thesis,struct Bucket *country);
struct C_Bucket *CHTRehash(struct C_Bucket * hashtable, struct C_Bucket *tmp);