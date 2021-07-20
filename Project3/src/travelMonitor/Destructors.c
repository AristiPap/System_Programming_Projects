#include "../../include/Destructors.h"

void DB_Destructor(List B_filters,List T_Stats) {
    //free the virus list
    ListofMaster_Virus(B_filters);   
    ListofVirusTStats(T_Stats);
}

void  ListofVirusNodes(List list){

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
          Destructor((*(virusNode *)node->info).blf);
          ListDelete((*(virusNode *)node->info).country_names);
          free(node->info);
          free(node);
          node = temp;
          list->size--;
      }
  }
  free(list);
}

void ListofMaster_Virus(List list){
    
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
          ListofVirusNodes((*(master_virus *)node->info).b_filters);
          free(node->info);
          free(node);
          node = temp;
          list->size--;
      }
  }
  free(list);
}

void ListofVirusTStats(List list){
    
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
          free((*(virus_travel_stats *)node->info).virus_type);
          ListofTStats((*(travel_stat_info *)node->info).date_info);
          free(node->info);
          free(node);
          node = temp;
          list->size--;
      }
  }
  free(list);
}

void ListofTStats(List list){
    
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
          free((*(travel_stat_info *)node->info).country_name);
          ListDelete((*(travel_stat_info *)node->info).date_info);
          free(node->info);
          free(node);
          node = temp;
          list->size--;
      }
  }
  free(list);
}