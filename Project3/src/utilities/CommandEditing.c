#include "../../include/ipc.h"
#include "../../include/CommandEditing.h"
#include <inttypes.h>
#include <stdint.h> /* fixed-width integer types */
#include <stdlib.h> /* strtol */
#include <stdbool.h>
#include <errno.h>

int find_command(char *_command){
    int size=strlen(_command);
    char *command=malloc((size+1)*sizeof(char));
    strcpy(command,_command);
    command[size]='\0';
    
    if(_command == NULL || _command[0]=='\0'){
      free(command);
      return -1;
    }    
        
    if(strcmp(command,"/exit\n")==0){
      free(command);
      return EXIT;
    }    
        
    if(strcmp(command,"/travelRequest")==0){
      free(command);
      return TRAVEL_REQUEST;
    }    
            
    if(strcmp(command,"/travelStats")==0){
      free(command);
      return TRAVEL_STATS;
    }    
    
    if(strcmp(command,"/addVaccinationRecords")==0){
      free(command);
      return COUNTRY_CHECK;
    }    
        
    if(strcmp(command,"/searchVaccinationStatus")==0){
      free(command);
      return SEARCH_VAC_STAT;
    }    
    
    if(strcmp(command,"-h\n")==0){
      free(command);
      return 13;
    }  
    free(command);   
    return -1;
}

void help(){
    printf("The commands you can enter are the following:\n");
    printf("~ /travelRequest citizenID date countryFrom countryTo virusName\n~/travelStats virusName date1 date2 [country]\n~/addVaccinationRecords country\n~/searchVaccinationStatus citizenID\n~/exit\n");
}

int count(char * str, char ch)
{
    int ctr = 0;
    // check if character in string
    for (int i = 0; i < strlen(str); i++)
        if (str[i] == ch)
            ctr++;
    return ctr;
}

char *trim(char *str)
{
    char *leading_sp = str;
    char *ending_sp = NULL;
    int len = strlen(str);
    
    ending_sp = str + len;

    while( (char) *leading_sp == ' ') 
        ++leading_sp; 
    if( ending_sp != leading_sp ){
        while((char)*(--ending_sp) == ' ' && ending_sp != leading_sp ) {}
    }

    if( leading_sp != str && ending_sp == leading_sp )
            *str = '\0';
    else if( str + len - 1 != ending_sp )
            *(ending_sp + 1) = '\0';

    ending_sp = str;
    if( leading_sp != str ){
        while( *leading_sp ) 
            *ending_sp++ = *leading_sp++; 
        *ending_sp = '\0';
    }

    return str;
}

str_to_uint16(const char *str, uint16_t *res)
{
  char *end;
  errno = 0;
  intmax_t val = strtoimax(str, &end, 10);
  if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0')
    return false;
  *res = (uint16_t) val;
  return true;
}