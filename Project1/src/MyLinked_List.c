#include "../includes/MyLinked_List.h"
#include "../includes/CommonStructures.h"

List list_create(){
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

int list_size(List list){
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
    if(list_size(list)==0)
        return NULL;
    else{
        return list->dummy->Next;
    }
}

ListNode ListFind(List list,char *findData, int flg){
    
    if(list_size(list)==0)
        return NULL;
    for (ListNode current = list->dummy->Next; current != NULL; current = current->Next){
        if(flg==0){
            if(strcmp((*(struct Bucket*)current->info).key,findData)==0){
                return current;	
            }
        }
        else{
            if(strcmp((*(filters *)current->info).virus_key,findData)==0){
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
    
    if (list_size(list)==0)
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
    if(list_size(old_list)==0){
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
