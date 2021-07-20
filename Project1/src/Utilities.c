#include "../includes/Utilities.h"

int count(char * str, char ch)
{
    int ctr = 0;
    // check if character in string
    for (int i = 0; i < strlen(str); i++)
        if (str[i] == ch)
            ctr++;
    return ctr;
}

//if user types a command and adds spaces at the end or beginning I trim them so not to get a segmentation fault in main
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

size_t fget_lines(char *filename){
    FILE *fd = fopen(filename, "r"); 
    if (!fd) {
        printf("The file you entered does not exist!\n");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;
    unsigned char c;
    while ((c = fgetc(fd)) != EOF && !feof(fd))
        if(c == '\n')
            count+=1;
    // close file
    fclose(fd);

    return count;
}