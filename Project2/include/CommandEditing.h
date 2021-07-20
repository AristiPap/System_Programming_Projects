#pragma once
#include <string.h>

//keep the /command ... part of the user input so that we know which query to search in
int find_command(char *_command);
//help function in case user types wrong command
void help();
int count(char * str, char ch);
//if user types a command and adds spaces at the end or beginning I trim them so not to get a segmentation fault in main
char *trim(char *str);