
#include "../../include/header.h"
#include "../../include/init_monitors.h"
#include "../../include/tm_queries.h"
#include "../../include/signal_handler.h"

// Flags that show whether a signal of said got_<type> arrived & awaits handling.
// 0 if no signal arrived, else 1.
static volatile sig_atomic_t got_intq, got_usr2, got_chld;

static sigset_t ccode_set;  // Mask with blocked signals during a command process

static void handle_sig_int_quit(int signo) {
  got_intq = 1;
}

static void handle_sig_chld(int signo) {
  got_chld = 1;
}

void TM_Destructor(struct travel_monitors *tmonitor_info, int *monitor_size,DIR *input_dir,bloom_filter *bf,List B_filters,List stats)
{
  CreateMonitorlogs(tmonitor_info,monitor_size,stats);

  //send kill signals to all the monitors
  for (int i = 0; i < monitor_size; ++i){  
    ListDelete(tmonitor_info[i].countries);
    kill(tmonitor_info[i].w_pid, SIGKILL);
  }  
  //wait for the monitors to complete their destruction
  for (int i = 0; i < monitor_size; ++i) 
    wait(NULL);
  
  //after killing all the children we now close the pipes
  for (int i = 0; i < monitor_size; ++i)  
  {
    if (close(tmonitor_info[i].read_fd) == -1)
      exit(EXIT_FAILURE);
    
    if (close(tmonitor_info[i].write_fd) == -1) 
      exit(EXIT_FAILURE);
  }
  
  free(tmonitor_info);
  Destructor(bf);
  DB_Destructor(B_filters,stats);
  
  if (closedir(input_dir) == -1){
    perror("closedir @ cleanup");
    exit(EXIT_FAILURE);
  }
}

/* ========================================================================= */

// Create a logfile and output stats.
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

/* ========================================================================= */
pid_t revive_monitor(struct travel_monitors *tmonitor_info,int monitor_size,int index,int bufferSize,char *input_dir,int hashes,int filter_size,int height){
  char buf[256];
  pid_t pid_monitor = init_monitor_info(tmonitor_info, index, bufferSize, input_dir,hashes,filter_size,height);
  List dirs = getDirectories(input_dir);
  int total=0;
  for(int i=0; i<monitor_size; i++){
    total+=List_size(tmonitor_info[i].countries);
  }
  
  int new_countries = List_size(dirs) - total;
  int total2=0;
  int counter=0;
  char *res = malloc((new_countries * 50 +new_countries) * sizeof(char *));
  for (ListNode current = dirs->dummy->Next; current != NULL; current = current->Next){
    for(int i=0; i<monitor_size; i++){
      ListNode found = ListFind(tmonitor_info[i].countries,current->info,2);
      if(found==NULL)
        counter++;
    } 
    if(counter==monitor_size){
        ListPushBack(current->info,tmonitor_info[index].countries,(strlen(current->info)+1)*sizeof(char));
        char _buf[128];
        char *country = current->info ;
        if(current->Next!=NULL)
          snprintf(_buf,128, "%s/", country);
        else
          snprintf(_buf,128, "%s", country);
        strcpy(res+total, buf);
        total2 += strlen(buf);
    }
  }
  
res[total] = '\0';
write_msg_to_pipe(tmonitor_info[index].write_fd, READ_DIR_CMD, res, sizeof(res));
write_msg_to_pipe(tmonitor_info[index].write_fd, READY, "", bufferSize);
free(res);
return pid_monitor;
}

void signals_unblock(void) {
  sigprocmask(SIG_UNBLOCK, &ccode_set, NULL);
}

// Block signals 
void signals_block(void) {
  sigprocmask(SIG_BLOCK, &ccode_set, NULL);
}

void signals_check(bloom_filter *bf,List B_filters,List stats)
{
  if (got_intq)  // check if received sigint
  {
    TM_Destructor(false, NULL, NULL,bf,B_filters,stats);
    exit(EXIT_FAILURE);
  }

  got_intq = got_usr2 = got_chld = 0;  // Reset 
}

//DEstructor function for the structures of the app
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