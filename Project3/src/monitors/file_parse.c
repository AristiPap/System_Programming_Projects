
#include "../../include/header.h"
#include "../../include/file_parse.h"
#include "../../include/VaccinMonitor_Utilities.h"
/* ========================================================================= */
// Parse a file with patients line by line, and insert every patient to the database.
// Return a string (report) with patient stats. Update valid/invalid records counters.
void parse_file(FILE *fp)
{
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  citizen_Record *person;
  filters *virus;
  //while loop reads a record from the input file and saves in each row of the 2-d dimensional array each word from the file
    while ((read = getline(&line, &len, fp)) != -1) {
      char newString[8][50];
      int j, ctr;
      j = 0;
      ctr = 0;
      for (int i = 0; i <= (strlen(line)); i++) {
        // if space or NULL found, assign NULL into newString[ctr]
        if (line[i] == ' ' || line[i] == '\0') {
          newString[ctr][j] = '\0';
          ctr++;  // for next word
          j = 0;  // index=0 for next word
        } 
        else {
          newString[ctr][j] = line[i];
          j++;
        }
      }
      if(strcmp(newString[6],"NO")==0 && newString[7]!=NULL){
       //  fprintf(fd2,"ERROR IN RECORD: %s, %s, %s, %s, %d\n",newString[0],newString[1],newString[2],newString[3],atoi(newString[4]));
        continue;
      }
      
      if(strcmp(newString[6],"NO")==0)
        insertCitizenRecord(newString[0], newString[1],newString[2],newString[3], atoi(newString[4]),newString[5],newString[6],"None",_bf_size,10,0);  
      else
        insertCitizenRecord(newString[0], newString[1],newString[2],newString[3], atoi(newString[4]),newString[5],newString[6], newString[7],_bf_size,10,0);
      
    }
  if (line) 
      free(line);

}

