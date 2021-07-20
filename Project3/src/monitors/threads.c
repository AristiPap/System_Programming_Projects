#include "../../include/my_threads.h"
#include "dir_queries.h"
#include <pthread.h>

//set mutexes and conditional variables 
pthread_mutex_t cb_mtx = PTHREAD_MUTEX_INITIALIZER; //mutex that locks and unlocks for circular buffer actions
pthread_mutex_t mtx_proc = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cb_cond_full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cb_mtx_full = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cb_cond_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cb_mtx_empty = PTHREAD_MUTEX_INITIALIZER;

thread_sec_info *create_thread_args( pthread_t threads[],int num_threads,int country_names,char **countries,char *dir_path,int c_buf_size, int buf_size,int bloom_size,List files_read){    
    thread_sec_info *tm = malloc(sizeof(*tm));
    int iter;

    tm->cb = cb_init(c_buf_size);
    tm->c_buf_size = c_buf_size;
    tm->buf_size = buf_size;
    tm->total_files_num = country_names;
    tm->history_files = files_read;
    strcpy(tm->dir_path,dir_path);
    if(country_names>c_buf_size)
        iter = c_buf_size;
    else
        iter = country_names;
    
    tm->files = countries;
    //fill buffer for the first time with the first iter-file names.
    for(int j=0; j<iter; j++){
        if(tm->cb->size == j)
            break;
        cb_place(tm->cb,tm->files[j]);
        //printf("Going to insert %d -> %d \n",j,tm->total_files_num);
        puts(countries[j]);
        tm->last_file_index = j;
        keep_history_files(countries[j],dir_path,files_read);
    }
    
    int err;
    for (int i = 0; i < num_threads; ++i){
       err =  pthread_create(&threads[i], NULL,monitor_thread_handler ,(void *)tm);
        if(err){
            perror("Thread creation failed");
            exit(1);
        }
    }
    return tm;
}

//arguements of the thread handler are used as parameters in the read_directory function
void *monitor_thread_handler(void *arguements){
    thread_sec_info *tm = (thread_sec_info *) arguements;
    while(1){
        pthread_mutex_lock(&mtx_proc);
        pthread_mutex_lock(&cb_mtx_empty);
        bool exit = cb_empty(tm->cb);
        if(exit == true){
            //remember to unlock the mutexes so when we add new files with add vaccination query, we dont hang.
            if(tm->total_files_num == tm->last_file_index +1){ //no more files left to insert so we break
                pthread_mutex_unlock(&cb_mtx_empty);
                pthread_mutex_unlock(&mtx_proc);
                break;
            }
            else{
                int iter = tm->last_file_index +1;
                for(int i = 0; i<tm->c_buf_size; i++){
                    puts(tm->files[i]);    
                    if(iter < tm->total_files_num){
                        cb_place(tm->cb,tm->files[iter]);
                        tm->last_file_index = iter;
                        keep_history_files(tm->files[iter],tm->dir_path,tm->history_files);
                        iter++;
                    }    
                    else
                        break;
                } 
            }
        }    
        pthread_mutex_unlock(&cb_mtx_empty);
        
        pthread_mutex_lock(&cb_mtx);
        char *country_dir = NULL;
        char *tmp_file_path = cb_remove(tm->cb);
        if (tmp_file_path) {
            country_dir = calloc(strlen(tmp_file_path) + 1, sizeof(char));
            strcpy(country_dir, tmp_file_path);
        }
        
        pthread_mutex_unlock(&cb_mtx);
        
        pthread_mutex_lock(&cb_mtx_full);
        pthread_cond_signal(&cb_cond_full);
        pthread_mutex_unlock(&cb_mtx_full);
        
        if(country_dir!=NULL){
            read_directory(READ_DIR_CMD, country_dir, tm->dir_path,tm->buf_size);                 
            free(country_dir);
        }
        pthread_mutex_unlock(&mtx_proc);
    }
}

void monitor_add_vac(void *arguements, int write_fd,char *dir_path,int bufferSize,List files_read,char *country_name){
    thread_sec_info *tm = (thread_sec_info *) arguements;
    cb_place(tm->cb,country_name);
        
        pthread_mutex_lock(&mtx_proc);
        pthread_mutex_lock(&cb_mtx_full);
        bool exit = cb_full(tm->cb);
        if(exit == true){
            pthread_mutex_unlock(&cb_mtx_full);
            pthread_mutex_unlock(&mtx_proc);
            return;
        } 
        pthread_mutex_unlock(&cb_mtx_full);
        //remove country directory
        pthread_mutex_lock(&cb_mtx);
        char *country_dir = NULL;
        char *tmp_file_path = cb_remove(tm->cb);
        if (tmp_file_path) {
            country_dir = calloc(strlen(tmp_file_path) + 1, sizeof(char));
            strcpy(country_dir, tmp_file_path);
        }
        pthread_mutex_unlock(&cb_mtx);   
        //add Vaciination Records query
        puts(country_dir);
        checkForNewFiles_inCountry(write_fd,dir_path,bufferSize,files_read,country_dir);    
        free(country_dir);                
        pthread_mutex_unlock(&mtx_proc);
}

void join_threads(int num_threads, pthread_t threads[]) {
    int err;
    for (int i = 0; i < num_threads; i++) {
        if ((err = pthread_join(threads[i], NULL))) {
            perror("Error in joining thread");
            exit(1);
        }
    }
}


void destroy_threads(pthread_t threads[], int num_threads) {
    //join thread helper function
    join_threads(num_threads, threads);
    // free mutexes
    pthread_mutex_destroy(&cb_mtx);
    pthread_mutex_destroy(&cb_mtx_empty);
    pthread_mutex_destroy(&cb_mtx_full);
    pthread_mutex_destroy(&mtx_proc);
    // free conditional variables
    pthread_cond_destroy(&cb_cond_full);
    pthread_cond_destroy(&cb_cond_empty);
}

void tm_destructor(thread_sec_info *tm){
    cb_destroy(tm->cb);
      for(int j=0; j<tm->total_files_num; j++)
        free(tm->files[j]);
      free(tm->files);
      free(tm);
}