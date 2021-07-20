#pragma once
#include "DB_Hashes.h"
#include <time.h>

void CitizenDestructor(citizen_Record *person){
    free(person->ID);
    free(person->name);
    free(person->surname);
    free(person);
}

void DB_Destructor(struct C_Bucket *ht, List countries, List viruses) {
    //free the virus list
    VirusDestructor(viruses);
    //free the hashtable
    CHTDelete(ht, 0);
    //free list of countries
    ListofListsDelete(countries);
}

//free the list of viruses:for each node delete the bloom filter and 2 skiplists
void VirusDestructor(List list) {
    ListNode node = list->dummy;
    ListNode temp = NULL;
    while (node != NULL) {
        temp = node->Next;
        if (node == list->dummy) {
            free(node);
            node = temp;
            list->size--;
            continue;
        } else {
            Destructor((*(filters *)node->info).bf);
            skiplist_destroy((*(filters *)node->info).vaccinated);
            skiplist_destroy((*(filters *)node->info).non_vaccinated);
            free(node->info);
            free(node);
            node = temp;
            list->size--;
        }
    }
    free(list);
}