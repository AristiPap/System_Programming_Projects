
#include "../../include/header.h"

#include "../../include/dir_queries.h"
#include "../../include/monitor_queries.h"
#include "../../include/VaccinMonitor_Utilities.h"
#include "../../include/sockets.h"
#include "../../include/my_threads.h"

int listener_sock = -1;

static void find_query(int ccode, char *sanitised, char *dir_path, int write_fd, int bufferSize,char *country_name, thread_sec_info *tm,List files_read)
{
  char *rec_id;
  char *country;
  switch(ccode)
  {
    case READY:
      write_msg_to_pipe(write_fd, WORKER_READY, "", bufferSize);  // Send "Ready" signal
      return ;
    case READ_DIR_CMD:
      country = sanitised;
      return ;
    case TRAVEL_REQUEST:
      rec_id = sanitised;
      find_vaccination(rec_id,write_fd,bufferSize);
      return ;
    case SEARCH_VAC_STAT:
      m_searchVaccinationStatus(sanitised,write_fd,bufferSize);
      return ;
    case EXIT:
      ListDelete(files_read);
      DB_Destructor();
      exit(EXIT_SUCCESS);
      return;
    case COUNTRY_CHECK:
      if(strcmp(sanitised,"no")==0){
      //  puts(sanitised);
        write_msg_to_pipe(write_fd,UPDATE_DB,"no",bufferSize);
        return;
      }  
      else{
      //  puts(sanitised);
        strcpy(country_name,sanitised);
        monitor_add_vac(tm,write_fd,dir_path,bufferSize,files_read,country_name);
      }
      return;
  }  
}

static int find_occurence(char *str, char toSearch){
  int count = 0;
  int i=0;
  while(str[i] != '\0'){
    if(str[i] == toSearch)
      count++;  
  }
  return count;
}

int main(int argc, char *argv[]){
//"-p",port_num,"-t",num_threads,"-b",b_size_str ,"-c",cyclic_bufferSize,"-s",filter_size,res,input_dir,NULL;
  char *port = argv[2];
  puts(port);
  int num_threads = atoi(argv[4]);
  int bufferSize = atoi(argv[6]); 
  int cyclic_bufferSize = atoi(argv[8]);
  int blmSize = atoi(argv[10]);
  char *paths = argv[11];
  char *dir_path = argv[12];
  //we will use this variable to save the country from the /addVaccinationRecords query
  char country_name[30];
  //bloom_filter *bf_worker = Create(k_hashes,_bf_size);
  init_monitor_structs();
  List files_read = List_create();
  //create communication socket
  struct hostent *pc = find_ip();
  struct in_addr **ips = (struct in_addr **)pc->h_addr_list;
  _ip_addr_ = ntohl(ips[0]->s_addr);
  _port_ = atoi(port);
  
  char desired_char = '/';
  int thesis=0;
  int StringLength = strlen(argv[11]);
  for (int j=0; j < StringLength; j++) {
      if (argv[11][j] == desired_char) {
          thesis++;
      }
  }

  bloom_filter *bf = Create(6,1000);
  char **country_names = malloc((thesis+1)*sizeof(char *));
  char *token;
  int counter = 0;
  //separate the countries handled by the monitor 
  token = strtok(paths, "/");
  country_names[0] = malloc((strlen(token)+1)*sizeof(char));
  strcpy(country_names[0],token);
  counter++;
   while( token != NULL) {
    token = strtok(NULL, "/");
    country_names[counter] = malloc((strlen(token)+1)*sizeof(char));
    strcpy(country_names[counter],token);
    country_names[counter][strlen(token)]='\0';
    counter++;
    if(counter == thesis +1)
      break;
  }
  //initialise thread pool  
  pthread_t threads[num_threads];
  thread_sec_info *tm = create_thread_args(threads,num_threads,counter,country_names,argv[12],cyclic_bufferSize,bufferSize,_bf_size,files_read);
  /* ################################################################################### */
  listener_sock = _create_socket();
  //bind and listen to the new socket fd
  bind_listen(listener_sock, _ip_addr_, _port_);
    
  int newsockfd;
  if ((newsockfd = _accept(listener_sock, NULL, NULL)) < 0) {fprintf(stderr, "INIT CONNECTION FAILED (BF)\n"); exit(1);}
  
  //send bloom filters to the client to initialise his database
  get_blooms(INIT_DB,newsockfd,bufferSize);
  //inform client that monitor is ready to receive queries
  write_msg_to_pipe(newsockfd, WORKER_READY, "", bufferSize);
  fd_set _read;
  
  int monitor_fds[1];  //fd array will only have one memory cell, because server only gets connections from one client
  int available_monitors = 0;
  while(1){
    connection_sockfd = _create_socket();
    // wait for a connection
    if ((connection_sockfd = _accept(listener_sock, NULL, NULL)) < 0) {
        fprintf(stderr, "CONNECTION FAILED (QUERY)\nexiting..\n");
        break;
    } // connection done
    monitor_fds[0] = connection_sockfd;
    //at this part we receive bloom filters from the monitors
    if (available_monitors != 1)  // Wait for monitors response
    {
      FD_ZERO(&_read);
      for (int i = 0; i < 1; ++i)
      FD_SET(monitor_fds[i], &_read);  
      int check = select(FD_SETSIZE, &_read, NULL, NULL, NULL);
      if (check == -1){ 
        perror("select");
        continue;
      }
      for (int i = 0; i < 1; ++i)
      {  
        if (FD_ISSET(monitor_fds[i], &_read))  
        {
            char *message = read_from_pipe(monitor_fds[0],bufferSize,bf);
            int ccode = get_opcode(message);
            char *sanitised = malloc((strlen(message)+1)*sizeof(char));;
            strcpy(sanitised, get_pipe_message(message));
            sanitised[strlen(message)-1]='\0';
            free(message);
            if(strcmp(sanitised,"exit")==0){
              Destructor(bf);
              destroy_threads(threads,num_threads);
              tm_destructor(tm);
              free(sanitised);
            }
            //execute query that client sent 
            find_query(ccode, sanitised, dir_path, monitor_fds[0], bufferSize,country_name,tm,files_read);
            free(sanitised);
            available_monitors --;
        }
        else
          break;
      }
    }    
  }  
}
