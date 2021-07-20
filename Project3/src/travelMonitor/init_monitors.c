#include "../../include/header.h"
#include "../../include/init_monitors.h"

pid_t init_monitor_info(struct travel_monitors *monitors, int index, char *bufferSize_str, char *input_dir,int hashes,int filter_size,int height,uint16_t port)
{
  char read_p[32];  
  char write_p[32]; 
  //named_pipe_init(false, read_p, write_p);  // Create  unique named fifos for the communication between monitor-travelmonitor
  monitors[index].port = find_port();
//store monitor info in the monitors array
  
  //list of countries that each monitor handles
  monitors[index].countries = List_create();
    
  if (monitors[index].read_fd == -1)
    exit(1);
  if (monitors[index].write_fd == -1)
    exit(1);
  
  char port_num[5];
  snprintf(port_num,5,"%d",port);
}

// Initialise as many monitors as the user has defined

void init_monitors(struct travel_monitors *monitors, int monitor_size, int bufferSize,int cyclic_bufferSize,int num_threads,char *input_dir,int hashes,int filter_size,int height)
{
  char b_size_str[15];
  sprintf(b_size_str, "%d", bufferSize);
  
  //create monitors
  for (int i = 0; i < monitor_size; ++i){
    monitors[i].port = find_port();
    printf("PORT: %d\n",monitors[i].port);
    monitors[i].countries = List_create();
  }
  //assign countries
  assign_countries(monitors,monitor_size,input_dir,bufferSize);
  char c_bufsize[5];
  char threads[5];
  char filter_s[5];
  snprintf(c_bufsize,5,"%d",cyclic_bufferSize);
  snprintf(threads,5,"%d",num_threads);
  snprintf(filter_s,5,"%d",filter_size);
  //call exec to start the monitor program
  for (int i = 0; i < monitor_size; ++i){
    int total =0;
    char port_num[5];
    snprintf(port_num,5,"%d",monitors[i].port);
    char *res = malloc((List_size(monitors[i].countries) * 50 + List_size(monitors[i].countries)) * sizeof(char *));
    for (ListNode current = monitors[i].countries->dummy->Next; current != NULL; current = current->Next){
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
    pid_t pid = fork();
    if(pid == -1){
      perror("fork");
      exit(1);
    }  
    if(pid == 0){    
      //printf("-> %hu\n",port);
      //-p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 ... pathn
      execl("./monitorServer", "travelMonitorClient","-p",port_num,"-t",threads,"-b",b_size_str ,"-c",c_bufsize,"-s",filter_s,res,input_dir,NULL);
      perror("execl");
      exit(1);
    }
    free(res);
    monitors[i].w_pid = pid;  // Store monitor's pid
  }
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
    free(res);
  }
  ListDelete(dirs);
}