//Aristi Papastavrou 
//AM:1115201800154
#pragma once
#include <stddef.h>
typedef void *pVoid;
typedef struct Node* ListNode;
typedef struct list* List;
typedef void *pVoid;

// struct of Node of List
struct Node 
{
  /*void pointer*/
  pVoid info;
  struct Node *Next;
  struct Node *Prev;
};

struct list {
    ListNode dummy;				
    ListNode Last;	
    size_t data_size;
    int size;	
    int ctr_found;
};


int list_size(List list);
List list_create();
pVoid list_node_value(List list, ListNode node);
ListNode ListGetLast(List list);
ListNode ListGetFirst(List list);
ListNode ListFind(List list,char *findData,int flg);
void ListPushBack(pVoid newdata,List list,size_t data_size);
void ListInsertSorted(pVoid newdata,List list);
List list_duplicate(List old_list,List newlist,size_t data_size);
void List_Remove(List list, pVoid key);
void ListDelete(List list);
void ListofListsDelete(List list);