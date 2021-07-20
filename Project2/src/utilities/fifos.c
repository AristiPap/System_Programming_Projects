
#include "../../include/header.h"
#include "../../include/fifos.h"
#include <errno.h>

void create_fifo(char *read_ptr,char *write_ptr,int counter,char *fifo_name){
  snprintf(read_ptr, 32, "%s%d%c",fifo_name, counter, 'R');
  snprintf(write_ptr, 32, "%s%d%c",fifo_name, counter, 'W');
  counter++;
  // Create fifos
  if (mkfifo(read_ptr,  PERM_FILE) == -1){perror("mkfifo @ unique_fifo");exit(1);}
  if (mkfifo(write_ptr, PERM_FILE) == -1){perror("mkfifo @ unique_fifo");exit(1);}

}

void named_pipe_init(bool setup,char *read_p, char *write_p)
{
  static char fifo_name[32];
  static int counter = 0;
  
  if(setup==true){
    char pipe_path[] = "pipes"; 
    DIR* dir = opendir(pipe_path);
    if (dir) {
        /* Directory exists. */
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)  // Delete contents of subfiles and files
        {
          char *file = entry->d_name;
          if (!strcmp(file, ".") || !strcmp(file, ".."))
            continue;

          char full_path[128];
          snprintf(full_path, 128, "%s/%s", pipe_path, file);  // Remove file
          puts(full_path);
          int status = remove(full_path);
        }
      free(dir);
      rmdir(pipe_path);
    }
    if (mkdir(pipe_path, 0777) == -1){
      perror("mkdir @ unique_fifo");
      exit(1);
    }
    sprintf(fifo_name, "pipes/");
    return;
  }  
  create_fifo(read_p,write_p,counter,fifo_name);
  counter++;
}
