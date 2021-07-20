#pragma once 
#include "MyLinked_List.h"

void signal_handler_init(); 

void checkusr1(int write_fd,char * input_dir,int bufferSize,List files_read,char *country_name);  
void signals_block();  

void signals_unblock(); 