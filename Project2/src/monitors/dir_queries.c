
#include "../../include/header.h"

#include "../../include/dir_queries.h"
#include "../../include/file_parse.h"

static void get_file_names(char *names[], int total, DIR *dir)
{
  struct dirent *entry;
  int curr_file = 0;
  while ((entry = readdir(dir)) != NULL) // Store every file name in array <names>
  {
    char *f_name = entry->d_name;
    if (!strcmp(f_name, ".") || !strcmp(f_name, ".."))
      continue;

    names[curr_file++] = f_name;//strdup(f_name);
  }
  rewinddir(dir);
}

/* ========================================================================= */

// Read each file of a directory and send a report for each file to the parent.

int find_occurences(char *str, char toSearch){
  int count = 0;
  int i=0;
  while(str[i] != '\0'){
  
    if(str[i] == toSearch)
      count++;
    
    i++;
  }
  return count;
}

int numOfFiles(char *dir){
    struct dirent **namelist;
    int n;
    int counter=0;
    int i=0;
    n = scandir(dir, &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else{
         while (n--) {
            if(strcmp(namelist[i]->d_name,"inputFile.txt")==0 || strcmp(namelist[i]->d_name,".")==0 || strcmp(namelist[i]->d_name,"..")==0){
                free(namelist[i]);
                i++;
                continue;
            }
            free(namelist[i]);
            i++;
            counter++;
        }
        free(namelist);
    }    
    return counter;
}

void read_directory(int opcode, char *country, char *input_dir, int write_fd, int bufferSize,List files_read)
{
  int send_opcode = INIT_DB;
  if(opcode == READ_DIR_CMD){
    int num_countries = find_occurences(country,'/') + 1;
    char *country_names[num_countries];
    char *token; 
     /* get the first token */
    token = strtok(country, "/");
    country_names[0] = token;
    int counter = 1; 
     /* walk through other tokens */
    while( token != NULL ) {
      token = strtok(NULL, "/");
      country_names[counter] = token;
      counter++;
    }
    
    for(int j=0; j<num_countries; j++){
      char path[256];   // Compose path for dir to open
      
      snprintf(path, 256, "%s/%s", input_dir, country_names[j]);
      DIR *dir = opendir(path);
      
      if (dir == NULL){
        perror("opendir @ read_directory");
        exit(1);
      }
       
      int total_files = numOfFiles(path);
      char *file_names[total_files];
      get_file_names(file_names, total_files, dir);
      
      for (int i = 0; i < total_files; ++i)  // Parse every file in the dir
      {
        char file_path[256];
        snprintf(file_path, 256, "%s/%s", path, file_names[i]);
        ListPushBack(file_path,files_read,(strlen(file_path)+1)*sizeof(char));
        get_report(file_path);
      }
    }
    get_blooms(send_opcode,write_fd,bufferSize);
  }
}

/* ========================================================================= */

// Return the report of a new file located at <f_path>.
void get_report(char *f_path)
{
  FILE *fp = fopen(f_path, "r");
  if (fp == NULL){perror("fopen @ get_report"); exit(1);};

   parse_file(fp);

  if (fclose(fp) == -1){perror("fclose @ get_report"); exit(1);}
  //return report;
}

// Send a <report> of a new file to the parent.
void send_report(int opcode, char *report, int write_fd, int bufferSize,int msg_len, bloom_filter *bf) {
  write_bf_msg(write_fd, opcode, report, bufferSize,msg_len,bf);
  free(report); 
}

/* ========================================================================= */

// Check for updated files in the directories assigned to the worker.
// Send a report for each file found to the parent.
void checkForNewFiles_inCountry(int write_fd, char *input_dir, int bufferSize,List files_read,char *country_name)
{
  struct dirent *entry;
  int curr_file = 0;
  char path[256];
  snprintf(path,256,"%s/%s",input_dir,country_name);
  DIR *dir = opendir(path);
  while ((entry = readdir(dir)) != NULL) 
  { 
    char *f_name = entry->d_name;
    if (!strcmp(f_name, ".") || !strcmp(f_name, ".."))
      continue;

    snprintf(path,256,"%s/%s/%s",input_dir,country_name,f_name);
    ListNode found = ListFind(files_read,path,2);
    if(found==NULL){
      char file_path[256];
      ListPushBack(path,files_read,(strlen(path)+1)*sizeof(char));
      get_report(path);
      updated_blooms(UPDATE_DB,getppid(),write_fd,bufferSize,country_name);
    }
  }  
}

/* ========================================================================= */
