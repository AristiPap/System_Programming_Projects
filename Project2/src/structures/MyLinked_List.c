#include "../../include/MyLinked_List.h"
#include "../../include/CommonStructures.h"
#include "../../include/tm_queries.h"

List List_create(){
    // Create stuct
    List list = malloc(sizeof(*list));
    list->size = 0;
    list->ctr_found = 0;
    list->dummy = malloc(sizeof(*list->dummy));
    list->dummy->Next = NULL;		//empty list, dummy has no next node
    // Empty list,so last node is also dummy
    list->Last = list->dummy;
    return list;
}

int List_size(List list){
    return list->size;
}

ListNode ListGetLast(List list) {
    if (list->Last == list->dummy)
        return NULL;	
    else
        return list->Last;
}

//first node after dummy node 
ListNode ListGetFirst(List list){
    if(List_size(list)==0)
        return NULL;
    else{
        return list->dummy->Next;
    }
}


void *List_get(struct list *lis, int index)
{
  if (index > lis->size)
    return NULL;

  ListNode node = lis->dummy->Next;
  for (int i = 1; i < index; ++i)
    node = node->Next;

  return node->info;
}

ListNode ListFind(List list,char *findData, int flg){
    
    if(List_size(list)==0)
        return NULL;
    
    for (ListNode current = list->dummy->Next; current != NULL; current = current->Next){
        if(flg==0){
            if(strcmp((*(struct Bucket*)current->info).key,findData)==0){
                return current;	
            }
        }
        else if(flg==1){
            if(strcmp((*(filters *)current->info).virus_key,findData)==0){
                return current;	
            }
        }
        else if(flg==2){
            if(strcmp(current->info,findData)==0){
                return current;	
            }
        }
        else if(flg==3){
            if(strcmp((*(master_virus *)current->info).virus_name,findData)==0){
                return current;	
            }
        }
        else if(flg==4){
            if(strcmp((*(travel_stat_info *)current->info).country_name,findData)==0){
                return current;	
            }
        }
        else if(flg==5){
            if(strcmp((*(virus_travel_stats *)current->info).virus_type,findData)==0){
                return current;	
            }
        }
    }
    return NULL;
}

void ListPushBack(pVoid newdata,List list,size_t data_size)
{
    //Locate last node
    ListNode current =ListGetLast(list);
    ListNode temp=malloc(sizeof(*temp));
    temp->info = malloc(data_size);
    memcpy(temp->info,newdata,data_size);
    temp->Next = NULL;
    
    if (List_size(list)==0)
    {
        list->dummy->Next=temp;
        temp->Prev=list->dummy;
        list->size++;
        list->Last=temp;
    }
    else{
        current->Next = temp;
        temp->Prev=current;
        list->Last=temp;
        list->size++;
    }
}

/*List list_duplicate(List old_list,List newlist,size_t data_size){
    if(newlist==NULL){
        newlist=list_create();
    }
    if(List_size(old_list)==0){
        return newlist;
    }
    
    for (ListNode current = old_list->dummy->Next; current != NULL; current = current->Next){
        ListPushBack(current->info,newlist,sizeof(data_size));
    }
    return newlist;
}*/

void ListDelete(List list){
    ListNode node = list->dummy;
    ListNode temp = NULL;
    
    while (node != NULL) {
        if(node==list->dummy){
            temp = node->Next;
            free(node);
            node = temp;
            list->size--;
        }
        else{
            temp = node->Next;
            free(node->info);
            free(node);
            node = temp;
            list->size--;
        }
        
    }
    free(list);
}

void ListofListsDelete(List list){
    
    ListNode node = list->dummy;
    ListNode temp = NULL;
    
    while (node != NULL) {
        if(node==list->dummy){
            temp = node->Next;
            free(node);
            node = temp;
            list->size--;
        }
        else{
            temp = node->Next;
            ListDelete((*(struct Bucket *)node->info).citizens);
            free(node->info);
            free(node);
            node = temp;
            list->size--;
        }   
    }    
    free(list);    
}
