#include "../../include/header.h"
#include "../../include/init_monitors.h"

pid_t init_monitor_info(struct travel_monitors *monitors, int index, char *bufferSize_str, char *input_dir,int hashes,int filter_size,int height)
{
  char read_p[32];  
  char write_p[32]; 
  named_pipe_init(false, read_p, write_p);  // Create  unique named fifos for the communication between monitor-travelmonitor

//store monitor info in the monitors array
  monitors[index].write_fd = open(read_p, O_RDWR | O_NONBLOCK);  
  monitors[index].read_fd = open(write_p, O_RDONLY | O_NONBLOCK);
  //list of countries that each monitor handles
  monitors[index].countries = List_create();
    
  if (monitors[index].read_fd == -1)
    exit(1);
  if (monitors[index].write_fd == -1)
    exit(1);
  
  //get monitors pid
  pid_t pid = fork();
  if(pid == -1){
    perror("fork");
    exit(1);
  }  
  if(pid == 0){    
    execl("./Monitor", "travelMonitor", bufferSize_str, read_p, write_p, input_dir, NULL);
    perror("execl");
    exit(1);
  }
  monitors[index].w_pid = pid;  // Store his pid
  return pid;
}


// Initialise as many monitors as the user has defined
void init_monitors(struct travel_monitors *monitors, int monitor_size, int bufferSize, char *input_dir,int hashes,int filter_size,int height)
{
  named_pipe_init(true, NULL, NULL);  // Setup fifos

  char b_size_str[15];
  sprintf(b_size_str, "%d", bufferSize);

  for (int i = 0; i < monitor_size; ++i)
    init_monitor_info(monitors, i, b_size_str, input_dir,hashes,filter_size, height);
}

static void getFileDirectories(char *dir,char *names[])
{
    struct dirent **namelist;
    int n;
    int i=0;
    int counter=0;
    n = scandir(dir, &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else{
         while (n--) {
            if(strcmp(namelist[counter]->d_name,"inputFile.txt")==0 || strcmp(namelist[counter]->d_name,".")==0 || strcmp(namelist[counter]->d_name,"..")==0){
                counter++;
                continue;
            }
            names[i] = namelist[counter]->d_name;
            counter++;
            i++;
        }
    }    
    
}
// Assign countries (dirs) located in <input_dir> to workers in <monitors>.
// Map every country to the PID associated with it in <ht_workers>. 
void assign_countries(struct travel_monitors * monitors, int monitor_size,DIR *input_dir, int bufferSize)
{
  int curr_w = 0;  // Current worker index
  struct dirent *entry; 
  
  List dirs = getDirectories(input_dir);
  
  for (ListNode current = dirs->dummy->Next; current != NULL; current = current->Next){
    ListPushBack(current->info,monitors[curr_w].countries,(strlen(current->info)+1)*sizeof(char));
    curr_w = (curr_w + 1) % monitor_size;  // Assign dirs in round-robin fashion
  }
  for(int j=0; j<monitor_size; j++){
    int total =0;
    char *res = malloc((List_size(monitors[j].countries) * 50 + List_size(monitors[j].countries)) * sizeof(char *));
    for (ListNode current = monitors[j].countries->dummy->Next; current != NULL; current = current->Next){
      char buf[128];
      char path[256];
      char *country = current->info ;
      if(current->Next!=NULL)
        snprintf(buf,128, "%s/", country);
      else
        snprintf(buf,128, "%s", country);
      strcpy(res+total, buf);
      total += strlen(buf);
    }
  
    res[total] = '\0';
    write_msg_to_pipe(monitors[j].write_fd, READ_DIR_CMD, res, sizeof(res));
    free(res);
  }
  for (int i = 0; i < monitor_size; ++i)  // Send End of Task / Availability check
    write_msg_to_pipe(monitors[i].write_fd, READY, "", bufferSize);
  
  ListDelete(dirs);
}