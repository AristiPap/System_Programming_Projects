
#include "../../include/header.h"

#include "../../include/dir_queries.h"
#include "../../include/monitor_queries.h"
#include "../../include/signal_handling.h"
#include "../../include/VaccinMonitor_Utilities.h"

static void find_query(int ccode, char *sanitised, char *dir_path, int write_fd, int bufferSize, List files_read,char *country_name)
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
      if(ccode == READ_DIR_CMD)
      read_directory(ccode, country, dir_path, write_fd, bufferSize,files_read);
      return ;
    case TRAVEL_REQUEST:
      rec_id = sanitised;
      find_vaccination(rec_id,write_fd,bufferSize);
      return ;
    case SEARCH_VAC_STAT:
      m_searchVaccinationStatus(sanitised,write_fd,bufferSize);
      return ;
    case COUNTRY_CHECK:
      if(strcmp(sanitised,"")==0){
        return;
      }  
      else{
        strcpy(country_name,sanitised);
        write_msg_to_pipe(write_fd,COUNTRY_CHECK_RES,sanitised,bufferSize);
      }
      return;
  }  
}

int main(int argc, char *argv[])
{
  signal_handler_init();
  signals_block();  // Block signals during setup
  int bufferSize = atoi(argv[1]);  // Process command line args
  char *read_p = argv[2];
  char *write_p = argv[3];
  char *dir_path = argv[4];
  //we will use this variable to save the country from the /addVaccinationRecords query
  char country_name[30];
  
  bloom_filter *bf_worker = Create(k_hashes,_bf_size);
  init_monitor_structs();
  List files_read = List_create();
  
  int read_fd = open(read_p, O_RDONLY);  // Open named pipe for reading
  if (read_fd == -1)
    exit(1);

  int write_fd = open(write_p, O_RDWR | O_NONBLOCK);  // Open named pipe for writing
  if (write_fd == -1)
   exit(1);
 
  signals_unblock();  // Finished setup
  
  //we block all signals each time we process a command or we search for sigusr1
  while (1)
  {
    signals_block();
    checkusr1(write_fd,dir_path,bufferSize,files_read,country_name);
    signals_unblock();

    char *message = read_from_pipe(read_fd, bufferSize,bf_worker);
    //block signals while processing command
    signals_block();

    if (message == NULL) 
      continue;      

    int ccode = get_opcode(message);
    char *sanitised = malloc((strlen(message)+1)*sizeof(char));;
    strcpy(sanitised, get_pipe_message(message));
    sanitised[strlen(message)-1]='\0';
    free(message);
    find_query(ccode, sanitised, dir_path, write_fd, bufferSize, files_read,country_name);
    free(sanitised);
  } 
  exit(0);
}
