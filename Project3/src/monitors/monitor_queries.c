
#include "../../include/header.h"
#include "../../include/monitor_queries.h"
#include "../../include/dir_queries.h"
#include "../../include/VaccinMonitor_Utilities.h"

extern struct monitor_info m_structs;
/* ========================================================================= */
bool Compare_vaccinationdates(char * date2, char *date1){
  //puts(date1);//actual vaccination date
  //puts(date2);//user input
    int day1 , day2, month1, month2,year1, year2;
    char *date;
    char _date1[11],_date2[11],_date3[11];
      
    strcpy(_date1,date1);
    
    date = strtok(_date1,"-");
    day1 = atoi(date);
    
    date = strtok(NULL,"-");
    month1 = atoi(date);
    
    date = strtok(NULL,"-");
    year1 = atoi(date);
    
    strcpy(_date2,date2);
    date = strtok(_date2,"-");
    day2 = atoi(date);
    
    date = strtok(NULL,"-");
    month2 = atoi(date);
    
    date = strtok(NULL,"-");
    year2 = atoi(date);
    
    //year 2 = user input date
    //year 1 = actual vaccination date
    if(year1>year2)
      return false;
    
    if(year2==year1){
        if(month2>month1)
          return false;
        else if(month1==month2){
            if(day2>day1)
              return false;
            else
              return true;
        }
        else if(month2-6<=month1<=month2)
          return true;
        else 
          return false;
    }
    
    if(year2 < year1){
      //if user was vaccinated a year ago then we check month
      if(month1<month2)
        return false;
      if(month1==month2)
        return false;
      if(month2>=month1+6){
        if(month2 == month1 +6){
          if(day2>day1)
            return false;
          else
            return true;
        }
        return true;
      }    
    }
    if(year2>year1){
      return false;
    }  
        
    return true;
}

void find_vaccination(char *msg, int write_fd, int bufferSize){
  //Body of Message: virusname/citizen_id/date
  //or Body of Message: no
  //printf("message received by monitor:%s\n",msg);
  if(strcmp(msg,"no")==0){
    write_msg_to_pipe(write_fd, TRAVEL_REQUEST,"no",bufferSize);
    return;
  }
  char buf[256];
  char answer[4];
  char *token = strtok(msg,"/");
  char *virus_name = malloc((strlen(token)+1)*sizeof(char));
  strcpy(virus_name,token);
  token = strtok(NULL, "/");
  char *citizen_id = malloc((strlen(token)+1)*sizeof(char));
  strcpy(citizen_id,token);
  token = strtok(NULL, "/");
  char *date = malloc((strlen(token)+1)*sizeof(char));
  strcpy(date,token);
  token = strtok(NULL, "/");
  char *country_name = malloc((strlen(token)+1)*sizeof(char));
  strcpy(country_name,token);
  
  ListNode found = ListFind(m_structs.viruses,virus_name,1);
  if(found==NULL){
    printf("Virus doesn't exist\n");
    write_msg_to_pipe(write_fd, TRAVEL_REQUEST,"no",bufferSize);
    return;
  }
  snode *found2=skiplist_search((*(filters *)found->info).vaccinated,atoi(citizen_id));
  if(found2==NULL){
    strcpy(answer,"NO");
    printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
  }  
  else{
    bool ok = Compare_vaccinationdates(date,found2->date);
    if(ok==true){
      printf("REQUEST ACCEPTED – HAPPY TRAVELS\n");
      strcpy(answer,"YES");
    }
    else{
      printf("REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");
      strcpy(answer,"NO");
    }
  }
  snprintf(buf, 256, "%s/%s/%s/%s",answer,virus_name,date,country_name);
  int total =strlen(virus_name) + strlen(answer) + strlen(date) + strlen(country_name) + 3;
  buf[total] = '\0';
  free(virus_name);
  free(country_name);
  free(citizen_id);
  free(date);
  write_msg_to_pipe(write_fd,TRAVEL_REQUEST,buf,bufferSize);
}

void m_searchVaccinationStatus(char *citizenID, int write_fd, int bufferSize)
{
  //CHTSearch(char *key, struct C_Bucket *ht)
  //puts(citizenID);
  int found = CHTSearch(citizenID,m_structs.citizen_ht);
  if(found==-1){
    write_msg_to_pipe(write_fd, SEARCH_VAC_STAT_RESULT,"no",bufferSize);
    return;
  }  
  puts(citizenID);
  char _buf[128];
  //printf("%s %s %s %s\n%sAGE %d\n",citizenID, (*( struct citizen_Record *)m_structs.citizen_ht[found].value).name,(*( struct citizen_Record *)m_structs.citizen_ht[found].value).surname,(*( struct Bucket *)m_structs.citizen_ht[found].value).key,(*( struct citizen_Record *)m_structs.citizen_ht[found].value).age);
  snprintf(_buf, 128, "%s %s %s %s\nAGE %d\n", citizenID, (*( struct citizen_Record *)m_structs.citizen_ht[found].value).name,(*( struct citizen_Record *)m_structs.citizen_ht[found].value).surname,(*( struct Bucket *)m_structs.citizen_ht[found].value).key,(*( struct citizen_Record *)m_structs.citizen_ht[found].value).age);
  int total = strlen(_buf);
  char *res = malloc(total*sizeof(char));
  memcpy(res,_buf,total*sizeof(char));
  for(ListNode current = m_structs.viruses->dummy->Next; current != NULL; current = current->Next){
    char buf[128];
    snode *vac_found;
    //snode *nonvac_found;
    vac_found = skiplist_search((*(filters *)current->info).vaccinated,atoi(citizenID));
    //nonvac_found = skiplist_search((*(filters *)current->info).non_vaccinated,atoi(citizenID));
    if(vac_found!=NULL){
      snprintf(buf,128, "%s VACCINATED ON %s\n", (*(filters *)current->info).virus_key,vac_found->date);
      //printf("%s VACCINATED ON %s\n",(*(filters *)current->info).virus_key,vac_found->date);
    }
    else{
      snprintf(buf,128, "%s NOT YET VACCINATED\n",(*(filters *)current->info).virus_key);
     //printf("%s NOT YET VACCINATED\n",(*(filters *)current->info).virus_key);
    }
    char *tmp = malloc(total*sizeof(char));
    memcpy(tmp,res,total*sizeof(char));
    free(res);
    res = malloc((total+strlen(buf))*sizeof(char));
    memcpy(res,tmp,total*sizeof(char));
    memcpy(res+total,buf,strlen(buf)*sizeof(char));
    total += strlen(buf);
    free(tmp);
  }
  res[total-1] = '\0';
  write_msg_to_pipe(write_fd, SEARCH_VAC_STAT_RESULT, res,bufferSize);
  free(res);
}