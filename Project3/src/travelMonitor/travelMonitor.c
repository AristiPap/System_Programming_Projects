
#include "../../include/header.h"
#include "../../include/tm_queries.h"
#include "../../include/init_monitors.h"
#include "../../include/DB_Hashes.h"
#include "../../include/VaccinMonitor_Utilities.h"
#include "../../include/CommandEditing.h"
#include "../../include/sockets.h"
#include "../../include/Destructors.h"



int get_msg_from_monitor(int opcode, char *sanitised, int *total, int *available_monitors,List viruses,List viruses_travel_stats,bloom_filter *new_bf,struct travel_monitors *tmonitor_info,int monitor_index)
{
  switch(opcode)
  {
    case WORKER_READY:  // Ready check
      ++(*available_monitors);
      return 0;

    case INIT_DB:  //Insert in db bloom filters and info after monitor processed its data
      init_db(sanitised,viruses,k_hashes,_bf_size,new_bf);
      return 0;
    
    case UPDATE_DB:  // Update the db with new bloom filters and info after user added new files in given directory 
      if(strcmp(sanitised,"no")==0)
        return 0;
      puts(sanitised);  
      update_db(sanitised,viruses,k_hashes,_bf_size,new_bf);
      return 0;
    
    //monitor that controls the country given in the /addVaccinationRecords command returns message that he is ready.
    case COUNTRY_CHECK_RES:
      //puts(sanitised);
      return COUNTRY_CHECK;
      
    case TRAVEL_REQUEST_RES:
      if(strcmp(sanitised,"no")==0)
        return 0;
      bool flg;
      //after we have received a report we check whether its positive/negative for the travel stats query
      char *token = strtok(sanitised,"/");
      char *action = malloc((strlen(token)+1)*sizeof(char));
      strcpy(action,token);
      if(strcmp(action,"YES")==0){
        flg=true;
      }  
      else
        flg=false;
      token = strtok(NULL, "/");
      char *virus_name = malloc((strlen(token)+1)*sizeof(char));
      strcpy(virus_name,token);
      token = strtok(NULL, "/");
      char *date = malloc((strlen(token)+1)*sizeof(char));
      strcpy(date,token);
      token = strtok(NULL, "/");
      char *country_name = malloc((strlen(token)+1)*sizeof(char));
      strcpy(country_name,token);
      
      ListNode find = ListFind(viruses_travel_stats,virus_name,5);
      if(find==NULL){
        virus_travel_stats *vnode = VTravelStats(country_name,virus_name);
        ListPushBack(vnode,viruses_travel_stats,sizeof(*vnode));
        find = ListGetLast(viruses_travel_stats);
        free(vnode);
      }
      Date_Info *dnode = date_init(date,flg);
      List tmp2 = (*(virus_travel_stats *)find->info).travel_stats;
      ListNode findCountry = ListFind(tmp2,country_name,4);
      ListPushBack(dnode,(*(travel_stat_info *)findCountry->info).date_info, sizeof(*dnode));
      //deallocate memory
      free(dnode);
      free(virus_name);
      free(date);
      free(action);
      free(country_name);
      return 0;
      
    case SEARCH_VAC_STAT_RESULT:
      printf("%s\n",sanitised);
      return 0;
  }
}



// Process a user command. If needed, forward the request to the required monitors.
// Returns 1 if the command given was /exit, else 0.
int get_command(char *cp,char *line, int ccode, int *available_monitors, int *total, int monitor_size, struct travel_monitors *tmonitor_info, int bufferSize, List B_filters,List viruses_travel_stats,bloom_filter *bf,DIR *input_dir,int *monitor_fds)
{
  //variables that will be used to get the info from the command given by the user
  char *stok_save;
  char *command;
  char ID[5];
  char *_vname;
  char date1[11];
  char date2[11];
  char cname1[30];
  char cname2[30];
  char name[13];
  char lastname[13];
  int age;
  char action[4];
  int flg;
  strtok_r(line, " \n", &stok_save);  // Initialize strtok_r
  int search_monitor_fds[monitor_size];
  fd_set _read;
  switch(ccode)
  {
    case TRAVEL_STATS:
    /* /travelRequest 430 22-10-2017 Greece Spain COVID-18
    /travelStats COVID-18 22-10-2017 22-10-2022 */
    
    // /travelStats virusName date1 date2 [country]
      if(count(cp,' ')==3)
        flg=0;
      else
        flg=1;
      command = strtok(cp," ");
      command = strtok(NULL," ");
      _vname =command;
      command = strtok(NULL," ");
      strcpy(date1,command);
      command = strtok(NULL," ");
      strcpy(date2,command);
      
      if(flg==0){
      //then no country given as a parameter
        travelStats(_vname,date1,date2,"None",viruses_travel_stats);
      }
      else{
        command = strtok(NULL," ");
        //strcpy(cname1,command);
        char *cname = malloc((strlen(command)+1)*sizeof(char));
        strcpy(cname,command);
        cname[strlen(command)-1] = '\0';
        travelStats(_vname,date1,date2,cname,viruses_travel_stats);
        free(cname);
      }
      return -2;
    
    case TRAVEL_REQUEST:
      //create sockets  
      for (int i = 0; i < monitor_size; ++i)  
      {
        search_monitor_fds[i] = _create_socket();
        int port = tmonitor_info[i].port;// monitor->manager->monitors[i].port;
        // connect to the server
        if (create_connection(search_monitor_fds[i], _ip_addr_, port) < 0) {
            fprintf(stderr, "Failed to close monitor-%d\n", 1);
            continue;
        }
      }
      command = strtok(NULL," ");
      strcpy(ID,command);
      command = strtok(NULL," ");
      strcpy(date1,command);
      command = strtok(NULL," ");
      strcpy(cname1,command);
      command = strtok(NULL," ");
      strcpy(cname2,command);
      command = strtok(NULL," ");
      char *vname = malloc((strlen(command)+1)*sizeof(char));
      strcpy(vname,command);
      vname[strlen(command)-1] = '\0';
      int fd = travelRequest(search_monitor_fds,tmonitor_info,monitor_size,bufferSize,ID,date1,cname1,cname2,vname,B_filters,viruses_travel_stats);
      free(vname);
      //virus wasnt found
      if(fd==-1){
        for(int j=0; j<monitor_size; j++){
          shutdown(search_monitor_fds[j], SHUT_RDWR);
        }
        return -2;
      }
      *available_monitors = 0;
      while (1){
          if (available_monitors != monitor_size)  // Wait for monitors response
          {
            FD_ZERO(&_read);
            for (int i = 0; i < monitor_size; ++i)
              FD_SET(search_monitor_fds[i], &_read);  
  
            int check = select(FD_SETSIZE, &_read, NULL, NULL, NULL);
            if (check == -1){ 
              perror("select");
              continue;
            }
            for (int i = 0; i < monitor_size; ++i)
            {  
              if (FD_ISSET(search_monitor_fds[i], &_read))  // Find the pipe that has data to read from
              {
                char *msg = read_from_pipe(search_monitor_fds[i], bufferSize,bf);
                int opcode = get_opcode(msg);  // Operation code
                char *sanitised = malloc((msg_len_rd +1)*sizeof(char));     // Decoded message
                strcpy(sanitised, get_pipe_message(msg));
                sanitised[msg_len_rd] = '\0';
                if(strcmp(sanitised,"no")==0){
                  available_monitors++;
                  free(msg);
                  free(sanitised);
                  continue;
                }  
                free(msg);
                int flg = get_msg_from_monitor(TRAVEL_REQUEST_RES, sanitised, &total, &available_monitors,B_filters,viruses_travel_stats,bf,tmonitor_info,i);
                //this is for the addvaccinationquery 
                free(sanitised);
                return -2;
              }
            }
          }
          else
            break;
        }  
    
      for(int j=0; j<monitor_size; j++){
        shutdown(search_monitor_fds[j], SHUT_RDWR);
      }
      return -2;
      
    case SEARCH_VAC_STAT:
      command = strtok(NULL," ");
      char *_id = malloc((strlen(command)+1)*sizeof(char));
      strcpy(_id,command);
      _id[strlen(command)-1] = '\0';
      for (int i = 0; i < monitor_size; ++i)  
      {
        search_monitor_fds[i] = _create_socket();
        int port = tmonitor_info[i].port;
        // connect to the server
        if (create_connection(search_monitor_fds[i], _ip_addr_, port) < 0) {
            fprintf(stderr, "Failed to close monitor-%d\n", 1);
            continue;
        }
        write_msg_to_pipe(search_monitor_fds[i],SEARCH_VAC_STAT,_id,bufferSize);
      }
      free(_id);
      *available_monitors = 0; 

      while (1){
          if (available_monitors != monitor_size)  // Wait for monitors response
          {
            FD_ZERO(&_read);
            for (int i = 0; i < monitor_size; ++i)
              FD_SET(search_monitor_fds[i], &_read);  

            int check = select(FD_SETSIZE, &_read, NULL, NULL, NULL);
            if (check == -1){ 
              perror("select");
              continue;
            }
            for (int i = 0; i < monitor_size; ++i)
            {  
              if (FD_ISSET(search_monitor_fds[i], &_read))  // Find the pipe that has data to read from
              {
                char *msg = read_from_pipe(search_monitor_fds[i], bufferSize,bf);
                int opcode = get_opcode(msg);  // Operation code
                char *sanitised = malloc((msg_len_rd +1)*sizeof(char));     // Decoded message
                strcpy(sanitised, get_pipe_message(msg));
                sanitised[msg_len_rd] = '\0';
                if(strcmp(sanitised,"no")==0){
                  available_monitors++;
                  free(msg);
                  free(sanitised);
                  continue;
                }  
                free(msg);
                int flg = get_msg_from_monitor(opcode, sanitised, &total, &available_monitors,B_filters,viruses_travel_stats,bf,tmonitor_info,i);
                //this is for the addvaccinationquery 
                free(sanitised);
                for(int j=0; j<monitor_size; j++){
                  shutdown(search_monitor_fds[j], SHUT_RDWR);
                }
                return -2;
              }
            }
          }
          else
            break;
        }  
  
      for(int j=0; j<monitor_size; j++){
        shutdown(search_monitor_fds[j], SHUT_RDWR);
      }
      return -2;
      
    case COUNTRY_CHECK:
      *available_monitors = 0;
      //create socket connections with the monitors
      for (int i = 0; i < monitor_size; ++i) {
        search_monitor_fds[i] = _create_socket();
        int port = tmonitor_info[i].port;
        // connect to the server
        if (create_connection(search_monitor_fds[i], _ip_addr_, port) < 0) {
            fprintf(stderr, "Failed to close monitor-%d\n", 1);
            continue;
        }
      }
      command = strtok(NULL," ");
      char *countryname = malloc((strlen(command)+1)*sizeof(char));
      strcpy(countryname,command);
      countryname[strlen(command)-1] = '\0';
      //find monitor that handles the country given by the user
      for(int i=0; i<monitor_size; i++){
        ListNode found = ListFind(tmonitor_info[i].countries,countryname,2);
        if(found!=NULL){
          write_msg_to_pipe(search_monitor_fds[i],COUNTRY_CHECK,countryname,bufferSize);
        }
        else{
          write_msg_to_pipe(search_monitor_fds[i],COUNTRY_CHECK,"no",bufferSize);
        }
      } 
      free(countryname);
      *available_monitors = 0; 

      while (1){
          if (available_monitors != monitor_size)  // Wait for monitors response
          {
            FD_ZERO(&_read);
            for (int i = 0; i < monitor_size; ++i)
              FD_SET(search_monitor_fds[i], &_read);  

            int check = select(FD_SETSIZE, &_read, NULL, NULL, NULL);
            if (check == -1){ 
              perror("select");
              continue;
            }
            for (int i = 0; i < monitor_size; ++i)
            {  
              if (FD_ISSET(search_monitor_fds[i], &_read))  // Find the pipe that has data to read from
              {
                char *msg = read_from_pipe(search_monitor_fds[i], bufferSize,bf);
                int opcode = get_opcode(msg);  // Operation code
                char *sanitised = malloc((msg_len_rd +1)*sizeof(char));     // Decoded message
                strcpy(sanitised, get_pipe_message(msg));
                sanitised[msg_len_rd] = '\0';
                if(strcmp(sanitised,"no")==0){
                  available_monitors++;
                  free(msg);
                  free(sanitised);
                  continue;
                }  
                free(msg);
                int flg = get_msg_from_monitor(opcode, sanitised, &total, &available_monitors,B_filters,viruses_travel_stats,bf,tmonitor_info,i);
                //this is for the addvaccinationquery 
                free(sanitised);
                //for(int j=0; j<monitor_size; j++){
                //  shutdown(search_monitor_fds[j], SHUT_RDWR);
                //}
                return -2;
              }
            }
          }
          else
            break;
        }  
  
      for(int j=0; j<monitor_size; j++){
        shutdown(search_monitor_fds[j], SHUT_RDWR);
      }
      return -2;     
    case EXIT:
      for (int i = 0; i < monitor_size; ++i) {
        search_monitor_fds[i] = _create_socket();
        int port = tmonitor_info[i].port;
        // connect to the server
        if (create_connection(search_monitor_fds[i], _ip_addr_, port) < 0) {
            fprintf(stderr, "Failed to close monitor-%d\n", 1);
            continue;
        }
        write_msg_to_pipe(search_monitor_fds[i],EXIT,"exit",bufferSize);
      }
      int status;
      int ret;
      errno = 0;
      while ((ret=wait(&status)) != -1)
          ;
      // if the error is anything else but "no child processes" then print it
      if (errno && errno != 10) {perror("wait");}

      // close all connection-sockets
      for(int j=0; j<monitor_size; j++){
        close(search_monitor_fds[j]);
      }
      CreateMonitorlogs(tmonitor_info,monitor_size,viruses_travel_stats);
      for(int i=0; i<monitor_size; i++){
        ListDelete(tmonitor_info[i].countries);
      }
      //free all the main memory
      closedir(input_dir);
      DB_Destructor(B_filters,viruses_travel_stats);
      Destructor(bf);
      free(tmonitor_info);
      return -3;
    case 13:
      help();
      return -2;
    default:
      printf("ERROR: %d: Incorrect menu option, type -h if you want to see the menu option!\n", -1);
      return -2;
  }
}

int main(int argc, char *argv[]){

  if(argc!=13){
    printf("You entered wrong input.You have to type:./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads %d\n ",argc);
    return 0;
  }
  int monitor_size, bufferSize; 
  monitor_size = atoi(argv[2]);
  bufferSize = atoi(argv[4]);
  int c_buf_size = atoi(argv[6]);
  int bf_size = atoi(argv[8]);
  char *files_path = argv[10];
  int num_threads = atoi(argv[12]);
  //for more than 1000 bytes of bloom filter size there appears to be a bug as sometimes the sending of the bloom filters fails
  //so if user gives more than 1000 I change it s that the programmes works fine
  if(bf_size>1000)
    bf_size=1000;
  
  int p=0;
  char *command;
  int s_height = 10;
  char *line=NULL;
  size_t len = 0;
  ssize_t read;
  int available_monitors = 0;  // # of workers ready to receive commands
  int total = 0;   
  int ccode;
    
  DIR *input_dir = opendir(files_path);
  char cp[258];
  //this will be a list of lists where every node will be a virus and will contain a list of the bloom filters that 
  //are about this virus
  List B_filters = List_create();
  //List that holds the virus stats for every country
  List viruses_travel_stats = List_create();
  bloom_filter *m_bf = Create(k_hashes,_bf_size);
  // Structures to store worker info
  struct travel_monitors *tmonitor_info;  
  tmonitor_info = calloc(monitor_size, sizeof (struct travel_monitors));
  
  //find ip of machine and get new port
  struct hostent *pc = find_ip();
  struct in_addr **ips = (struct in_addr **)pc->h_addr_list;
  _ip_addr_ = ntohl(ips[0]->s_addr);
  _port_ = LOWEST_PORT+1;
  //create monitors
  init_monitors(tmonitor_info, monitor_size, bufferSize,c_buf_size,num_threads,files_path,k_hashes,_bf_size,10);
  //try to connect with the monitor-size number servers
  int monitor_fds[monitor_size];
  for(int i=0; i<monitor_size; i++){
    int sock;
    sock = _create_socket();
    monitor_fds[i] = sock;
    int port = tmonitor_info[i].port;
    create_connection(sock,_ip_addr_,port);
  }
 
  fd_set _read; 
  //at this part we receive bloom filters from the monitors
  while (1){
      if (available_monitors != monitor_size)  // Wait for monitors response
      {
        FD_ZERO(&_read);
        for (int i = 0; i < monitor_size; ++i)
          FD_SET(monitor_fds[i], &_read);  

        int check = select(FD_SETSIZE, &_read, NULL, NULL, NULL);
        if (check == -1){ 
          perror("select");
          continue;
        }
        for (int i = 0; i < monitor_size; ++i)
        {  
          if (FD_ISSET(monitor_fds[i], &_read))  // Find the pipe that has data to read from
          {
            char *msg = read_from_pipe(monitor_fds[i], bufferSize,m_bf);
            int opcode = get_opcode(msg);  // Operation code
            char *sanitised = malloc((msg_len_rd +1)*sizeof(char));     // Decoded message
            strcpy(sanitised, get_pipe_message(msg));
            sanitised[msg_len_rd] = '\0';
            free(msg);
            int flg = get_msg_from_monitor(opcode, sanitised, &total, &available_monitors,B_filters,viruses_travel_stats,m_bf,tmonitor_info,i);
            //this is for the addvaccinationquery 
            free(sanitised);
          }
        }
      }
      else
        break;
    }  
  //we close the connections so that we dont run out of socket fds
  for(int j=0; j<monitor_size; j++){
    shutdown(monitor_fds[j], SHUT_RDWR);
  }
  //get commands from the user and execute the queries
  while(1){
      int len = 258;
      char line[len];
      printf("Please enter command:\n");
      if (fgets(line, len, stdin) == NULL)  // Get user input  
        continue;
      
      strcpy(cp,line);
      command  = strtok(line," ");
      ccode = find_command(command);
      if(strcmp(command,"\exit")==0)
        break;
      //exit
      int flg = get_command(cp,line, ccode, &available_monitors, &total, monitor_size, tmonitor_info, bufferSize,B_filters,viruses_travel_stats,m_bf,input_dir,monitor_fds);
      if(flg==-3)
        break; 
  }
  exit(EXIT_SUCCESS);
}

