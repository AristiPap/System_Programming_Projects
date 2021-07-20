
#include "../../include/header.h"
#include "../../include/init_monitors.h"
#include "../../include/tm_queries.h"
#include "poll.h"
/* ========================================================================= */

//this function will be used to decode each part of the message sent by the workers to the master at the init phase.
//at this stage the message will be without the header part so it will look something like this:
//COVID-18/France:Iceland:Norway:<bloomfilter bit array>
static int blm_index;

int find_occurences2(char *str, char toSearch){
  int count = 0;
  int i=0;
  while(str[i] != '\0'){
    if(str[i] == toSearch){
      blm_index = i; 
      
      if(toSearch == '/')
        break;
      count++;
    }  
   
    if(str[i]=='=' && toSearch=='=')
      return i;
    if(str[i]=='=' && toSearch==':')
      return count;
    i++;
  }
  return i;
}

static virusNode *Create_VirusNode(char **c_names,int num_cnames,bloom_filter *bf){
  virusNode *vnode = malloc(sizeof(*vnode));
  vnode->country_names = List_create();
  vnode->blf = Create(k_hashes,_bf_size);//bf;
  memcpy(vnode->blf,bf,sizeof(&bf));
  for(int i=0; i<num_cnames; i++){
    ListPushBack(c_names[i],vnode->country_names,(strlen(c_names[i])+1)*sizeof(char));
  }
  return vnode;
}

static void DeleteVirusNode(virusNode *vnode){
  Destructor(vnode->blf);
  ListDelete(vnode->country_names);
  free(vnode);
}

static master_virus *Create_Virus(char *v_name,char **c_names,int num_cnames, bloom_filter *bf,int hashes,int filter_size){
  master_virus *mv = malloc(sizeof(*mv));
  strcpy(mv->virus_name,v_name);
  mv->b_filters = List_create();
  virusNode *v_info = Create_VirusNode(c_names,num_cnames,bf);
  ListPushBack(v_info,mv->b_filters,sizeof(*v_info));
  free(v_info);
  return mv;
}

static void DeleteMVirus(master_virus *mv){
  free(mv);
}

/*receives the message from the monitor that looks like this (VirusName/Country1:Country2:..:Countryn=bloom_filter_bit_array_of_virusname)
The below function takes as a parameter the List of viruses that I save in the master_monitor and does the following steps:
1)decodes the message and saves in variables the virusname,bit array and country name
2)searches the list for the virus
3)if found then goes to the virus node and adds the bloom filter for that virus of the monitor that we received the message from
4)if not found, then we create a new node for the virus List and initialise it with the info of the message received
*/
void init_db(char *msg,List viruses,int hashes,int filter_size, bloom_filter *new_bf){
  if(strcmp(msg,"")==0)
    return;
  bloom_filter *new_blf = Create(k_hashes,_bf_size);
  memcpy(new_blf,new_bf,sizeof(new_bf));
  int num_countries = find_occurences2(msg,':');
  int bloom_index = find_occurences2(msg,'=');
  int num = find_occurences2(msg, '/');
  int j=0;
  while(msg[j]!='/'){
    j++;
  }
  char *virusname = malloc((j + 1)*sizeof(char));
  memcpy(virusname,msg,(j)*sizeof(char));
  virusname[j] = '\0';
 
  char **info = malloc((find_occurences2(msg,':')+1) * sizeof(char *));
  int counter=0;
  //get country names and save them in a 2-d array
  char *token = strtok(msg,"/");
  while(token != NULL) {
    if(counter == num_countries){
      token = strtok(NULL,"=");
      info[counter] = malloc((strlen(token)+1)*sizeof(char));
      strcpy(info[counter],token);
      break;
    }  
    token = strtok(NULL, ":");
    if(token!=NULL){
      info[counter] = malloc((strlen(token)+1)*sizeof(char));
      strcpy(info[counter],token);
    }
    counter++;
  }
    
  ListNode found = ListFind(viruses,virusname,2);
  //if virus node doesnt exist in the viruses list of the master monitor then add it
  if(found==NULL){
    master_virus *new_mv = Create_Virus(virusname,info,counter+1,&new_blf,hashes,filter_size);
    ListPushBack(new_mv,viruses,sizeof(*new_mv));
    free(new_mv);
  }
  //else update it with the new bloom filter
  else{
    virusNode *new_vbf = Create_VirusNode(info,counter+1,&new_blf);
    ListPushBack(new_vbf,(*(master_virus *)found->info).b_filters,sizeof(*new_vbf));
    free(new_vbf);
  }  
  //free allocated memory
  for(int i=0; i<counter+1; i++){
    free(info[i]);
  }
  free(info);
  free(virusname);
  Destructor(new_blf);
}

//travelRequest citizenID date countryFrom countryTo virusName
Date_Info* date_init(char *date,bool accepted){
  Date_Info *dnode = malloc(sizeof(*dnode));
  strcpy(dnode->date,date);
  dnode->acceptance = accepted;
  return dnode;
}

travel_stat_info *TStatInfo(char *country_name){
  travel_stat_info *tnode = malloc(sizeof(*tnode));
  tnode->date_info = List_create();
  tnode->country_name = malloc((strlen(country_name)+1)*sizeof(char));
  strcpy(tnode->country_name,country_name);
  return tnode;
}


virus_travel_stats *VTravelStats(char *country_name, char *virus_name){
  virus_travel_stats *vnode = malloc(sizeof(*vnode));
  vnode->virus_type = malloc((strlen(virus_name)+1)*sizeof(char));
  strcpy(vnode->virus_type,virus_name);
  vnode->travel_stats = List_create();
  travel_stat_info *tnode = TStatInfo(country_name);
  ListPushBack(tnode,vnode->travel_stats,sizeof(*tnode));
  free(tnode);
  return vnode;
}

int travelRequest(int *monitotr_fds, struct travel_monitors *tmonitor_info, int monitor_size, int bufferSize,char *citizen_id, char *date,char *countryFrom,char *countryTo,char *virus_name, List viruses,List viruses_travel_stats)
{
  //  Initialisation for the travelstat query
  ListNode find = ListFind(viruses_travel_stats,virus_name,5);
  if(find == NULL){ 
    virus_travel_stats *vnode = VTravelStats(countryFrom,virus_name);
    ListPushBack(vnode,viruses_travel_stats,sizeof(*vnode));
    find = ListGetLast(viruses_travel_stats);
    free(vnode);
  }
  
  ListNode found = ListFind(viruses,virus_name,3);
  if(found == NULL){
    printf("Entered virus doesn't exist, try again\n");
    return -1;
  }
  else{
    //save list of bloom filters with their respective countries
    List tmp = (*(master_virus *)found->info).b_filters;
    ListNode foundbloom;
    ListNode foundbloomcountry;
    bool flg_found = false;
    //every node of the virus list contains another list of bloom filters (and each bloom filter has a list of the countries that takes data from)
    for(ListNode current = tmp->dummy->Next; current != NULL; current = current->Next){
      List countries_bloom = (*(virusNode *)current->info).country_names;
      for(ListNode current_country = countries_bloom->dummy->Next; current_country != NULL; current_country = current_country->Next){
        if(strcmp(countryFrom,current_country->info)==0){
          flg_found = true;
          break;
        }
      }
      if(flg_found==true){
        bool found_monitor=false;
        bool flg = Search( (*(virusNode *)current->info).blf,citizen_id);
        if(flg == false){
            //if request is rejected then we also update the stats list
            printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
            Date_Info *dnode = date_init(date,false);
            List tmp2 = (*(virus_travel_stats *)find->info).travel_stats;
            ListNode findCountry = ListFind(tmp2,countryFrom,4);
            ListPushBack(dnode,(*(travel_stat_info *)findCountry->info).date_info, sizeof(*dnode));
            free(dnode);
            return -1;
        }
        //if bloom filter returns a positive answer then we have to go search to the monitors structs
        else{
          for(int i=0; i<monitor_size; i++){
            found = ListFind(tmonitor_info[i].countries,countryFrom,2);
            //found monitor that handles the country entered
            if(found!=NULL){
            //in order to know whether the request will be accepted or rejected we'll have to find out from the minitor
              char *msg = malloc((strlen(citizen_id)+strlen(virus_name)+strlen(date)+ strlen(countryFrom) + 4)*sizeof(char));
              snprintf(msg,258,"%s/%s/%s/%s",virus_name,citizen_id,date,countryFrom);
              //Body of Message: virusname/citizen_id/date/country  
              write_msg_to_pipe(monitotr_fds[i], TRAVEL_REQUEST, msg, bufferSize);
              free(msg);
              found_monitor = true;
            }
            else{
              write_msg_to_pipe(monitotr_fds[i], TRAVEL_REQUEST, "no", bufferSize);
            }
          }
          break;
        } 
      }
    }
  }
}

 //function to check if date 2 is between date1 and date3 and if date1 and date3 are in the right order
 bool Compare_dates(char * date1, char *date2, char *date3){
    int day1 , day2, day3, month1, month2, month3, year1, year2, year3;
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
    
    strcpy(_date3,date3);
    date = strtok(_date3,"-");
    day3 = atoi(date);

    date = strtok(NULL,"-");
    month3 = atoi(date);
    
    date = strtok(NULL,"-");
    year3 = atoi(date);
    
    if(year2 < year1 || year2 > year3)
        return false;
    if(year2==year1){
        if(month2<month1)
            return false;
        else if(month1==month2){
            if(day2<day1)
                return false;
        }
    }
    if(year2==year3){
        if(month2>month3)
            return false;
        else if(month1==month3){
            if(day2>day3)
                return false;
        }
    }
    return true;
}

void travelStats(char *virusName,char *date1,char *date2,char *country,List viruses_travel_stats){
  
  int acceptances=0;
  int rejections=0;
  ListNode find = ListFind(viruses_travel_stats,virusName,5);
  if(find == NULL){  
    printf("Virus entered, doesn't exist in the database,please try again...\n");
    return;
  }
  //List of countries that got requests for the given virus
  List tmp2 = (*(virus_travel_stats *)find->info).travel_stats;
  //if no country given then we will iterate all the countries
  if(strcmp(country,"None")==0){
      //outer for loop gives us the country name.Then we will save the list of dates to complete the query
    for(ListNode current = tmp2->dummy->Next; current != NULL; current = current->Next){
      List dates = (*(travel_stat_info *)current->info).date_info;
      //go through all the dates in the database where we had a request
      for(ListNode current2 = dates->dummy->Next; current2 != NULL; current2 = current2->Next){
        //first check if date is between the given dates
        bool ok = Compare_dates(date1,(*(Date_Info *)current2->info).date,date2);
        if(ok==true){
          if((*(Date_Info *)current2->info).acceptance==true)
            acceptances++;
          else
            rejections++;
        }
         
      }
    }
  }
  else{
    ListNode findCountry = ListFind(tmp2,country,4);
    if(findCountry==NULL){
      printf("Country entered doesn't exist in the stats database,please enter a new one that has had some requests made...\n");
      return;
    }
      
    List dates = (*(travel_stat_info *)findCountry->info).date_info;
    if(List_size(dates)==0){
      printf("TOTAL REQUESTS %d\n",0);
      printf("ACCEPTED %d\n",0);
      printf("REJECTED %d\n",0);
      return;
    }
      //go through all the dates in the database where we had a request
      for(ListNode current2 = dates->dummy->Next; current2 != NULL; current2 = current2->Next){
        //first check if date is between the given dates
        bool ok = Compare_dates(date1,(*(Date_Info *)current2->info).date,date2);
        if(ok==true){
          if((*(Date_Info *)current2->info).acceptance==true){
            acceptances++;
          }  
          else
            rejections++;
        }  
      }
  }
  printf("TOTAL REQUESTS %d\n",acceptances+rejections);
  printf("ACCEPTED %d\n",acceptances);
  printf("REJECTED %d\n",rejections);

}

//find which monitor handles the citizen id given as a parameter
void searchVaccinationStatus(struct travel_monitors *tmonitor_info, int monitor_size, int bufferSize,char *citizenID)
{
    int monitor_fds[monitor_size];
    for (int i = 0; i < monitor_size; ++i)  
    {
      monitor_fds[i] = _create_socket();
      int port = tmonitor_info[i].port;// monitor->manager->monitors[i].port;
      // connect to the server
      if (create_connection(monitor_fds[i], _ip_addr_, port) < 0) {
          fprintf(stderr, "Failed to close monitor-%d\n", 1);
          continue;
      }
      write_msg_to_pipe(monitor_fds[i],SEARCH_VAC_STAT,citizenID,bufferSize);
    }
    
}

//monitor created bloom filter after new file was added to directory
//so we bitwise ot the bit arrays and update them in the travelmonitors struct
void update_db(char *msg,List viruses,int hashes,int filter_size, bloom_filter *new_bf){
  if(strcmp(msg,"")==0)
    return;
    
  bloom_filter *new_blf = Create(k_hashes,_bf_size);
  memcpy(new_blf,new_bf,sizeof(new_bf));
  int num_countries = find_occurences2(msg,':');
  int bloom_index = find_occurences2(msg,'=');
  int num = find_occurences2(msg, '/');
  int j=0;
  while(msg[j]!='/'){
    j++;
  }
  char *virusname = malloc((j + 1)*sizeof(char));
  memcpy(virusname,msg,(j)*sizeof(char));
  virusname[j] = '\0';
 
  char **info = malloc((find_occurences2(msg,':')+1) * sizeof(char *));
  int counter=0;
  //get country names and save them in a 2-d array
  char *token = strtok(msg,"/");
  while(token != NULL) {
    if(counter == num_countries){
      token = strtok(NULL,"=");
      info[counter] = malloc((strlen(token)+1)*sizeof(char));
      strcpy(info[counter],token);
      break;
    }  
    token = strtok(NULL, ":");
    if(token!=NULL){
      info[counter] = malloc((strlen(token)+1)*sizeof(char));
      strcpy(info[counter],token);
    }
    counter++;
  }
    
  ListNode found = ListFind(viruses,virusname,2);
  //if virus node doesnt exist in the viruses list of the master monitor then add it
  if(found==NULL){
    master_virus *new_mv = Create_Virus(virusname,info,counter+1,&new_blf,hashes,filter_size);
    ListPushBack(new_mv,viruses,sizeof(*new_mv));
    free(new_mv);
  }
  //else update it the bloom filter
  else{
    List blooms = (*(master_virus *)found->info).b_filters;
    for(ListNode current = blooms->dummy->Next; current != NULL; current = current->Next){
      List countries_per_bloom = (*(virusNode *)current->info).country_names;
      ListNode update = ListFind(countries_per_bloom,info[0],2);
      if(update==NULL)
        continue;
      else{
        for(int i=0; i<ceil(new_bf->bloom_size/8); i++)
          (*(virusNode *)current->info).blf->bit_array[i] = (*(virusNode *)current->info).blf->bit_array[i] | new_bf->bit_array[i]; 
        Destructor(new_blf);
        break;
      }
    }
  }  
  //free allocated memory
  for(int i=0; i<counter+1; i++){
    free(info[i]);
  }
  free(info);
  free(virusname);
}

void CreateMonitorlogs(struct travel_monitors *tmonitor_info,int monitor_size,List stats)
{
  int check;
  char _buf[50];

  char dir_path[] = "logs";         // Create a dir for logs
  if (access(dir_path, F_OK) == 0){  // If dir already exists (due to abnormal previous termination, eg: SIGKILL)
    DIR* dir = opendir(dir_path);
    if (dir) {
        /* Directory exists. */
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)  // Delete contents of subfiles and files
        {
          char *file = entry->d_name;
          if (!strcmp(file, ".") || !strcmp(file, ".."))
            continue;

          char full_path[128];
          snprintf(full_path, 128, "%s/%s", dir_path, file);  // Remove file
          puts(full_path);
          int status = remove(full_path);
        }
      free(dir);
      rmdir(dir_path);
    }
  }
  if (mkdir("logs", 0777) == -1){
    perror("mkdir");
    exit(1);
  }

  struct bucket_entry *entry;
  //for each monitor traverse his country list
  for(int i=0; i<monitor_size; i++){
    snprintf(_buf, 50, "logs/log_file.%d", tmonitor_info[i].w_pid);
    int fd = creat(_buf, 0666);
    if (fd == -1){perror("creat"); exit(EXIT_FAILURE);}
    //first write country names of monitor in its log file
    int accepted = 0;
    int rejected =0;
    for(ListNode current =tmonitor_info[i].countries->dummy->Next; current != NULL; current = current->Next){
      //for each country traverse all the requests per type of vaccination  for the country of the previous loop
      for(ListNode current2 = stats->dummy->Next; current2 != NULL; current2 = current2->Next){
        List virus = (*(virus_travel_stats *)current2->info).travel_stats;
        for(ListNode current_country = virus->dummy->Next; current_country != NULL; current_country = current_country->Next){
            ListNode findCountry = ListFind(tmonitor_info[i].countries,(*(travel_stat_info *)current_country->info).country_name,2);
            //country belongs to the monitor so we traverse the list of acceptances
            if(findCountry!=NULL){
              for(ListNode current_acceptance = (*(travel_stat_info *)current_country->info).date_info->dummy->Next; current_acceptance != NULL; current_acceptance = current_acceptance->Next){
                if((*(Date_Info *)current_acceptance->info).acceptance==true)
                  accepted++;
                else if((*(Date_Info *)current_acceptance->info).acceptance==false)
                  rejected++;
              }
            }
        }
      }
     check = write(fd,current->info,strlen(current->info)); 
     check = write(fd,"\n",1);
    }
    char buf2[128];
    snprintf(buf2,128,"TOTAL TRAVEL REQUESTS %d\nACCEPTED %d\nREJECTED %d",accepted+rejected,accepted,rejected);
    check = write(fd,buf2,strlen(buf2));
    //finished the total of rejections and acceptances for the countries of monitor so we close its log file and create a new one for the next monitor  
    if (close(fd) == -1){
      perror("fclose");
      exit(EXIT_FAILURE);
    }
  }
}