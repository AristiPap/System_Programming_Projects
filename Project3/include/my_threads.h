#pragma once
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include "MyLinked_List.h"
#include "circular_buffer.h"
#include "ipc.h"
#include <pthread.h>

typedef struct thread_sec_info{
    int buf_size;
    int c_buf_size;
    char dir_path[30];
    struct cbuf *cb;
    char **files;
    int last_file_index;
    int total_files_num;
    List history_files;
}thread_sec_info; 


typedef void *thread_args;

void join_threads(int num_threads, pthread_t threads[]);
void destroy_threads(pthread_t threads[], int num_threads);
void *monitor_thread_handler(void *arguements);
thread_sec_info *create_thread_args(pthread_t threads[],int num_threads,int country_names,char **countries,char *dir_path,int c_buf_size, int buf_size,int bloom_size,List files_read);
void monitor_add_vac(void *arguements, int write_fd,char *dir_path,int bufferSize,List files_read,char *country_name);
void tm_destructor(thread_sec_info *tm);