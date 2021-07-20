#include "../../include/File_Utils.h"

//use scandir to get name of subdirectories of input dir in alphabetical order.
List getDirectories(char *input_dir){
    struct dirent **namelist;
    int n;
    int counter=0;
    List monitor_countries = List_create();
    n = scandir("./input_dir", &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            if(strcmp(namelist[counter]->d_name,"inputFile.txt")==0 || strcmp(namelist[counter]->d_name,".")==0 || strcmp(namelist[counter]->d_name,"..")==0){
                free(namelist[counter]);
                counter++;
                continue;
            }
            strcat(namelist[counter]->d_name,"\0");
            //calloc(strlen(src_string)+1, sizeof(char));
            ListPushBack(namelist[counter]->d_name,monitor_countries,(strlen(namelist[counter]->d_name)+1)*sizeof(char));
            free(namelist[counter]);
            counter++;
        }
        free(namelist);
    }    
    return monitor_countries;
}
