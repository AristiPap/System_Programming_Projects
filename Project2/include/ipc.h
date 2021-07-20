#pragma once

#define READ_DIR_CMD 1
#define INIT_DB 2
#define UPDATE_DB 15

#define TRAVEL_REQUEST 3
#define TRAVEL_REQUEST_RES 4

#define SEARCH_VAC_STAT 5
#define SEARCH_VAC_STAT_RESULT 6

#define COUNTRY_CHECK 11
#define COUNTRY_CHECK_RES 12

#define READY 31
#define WORKER_READY 32

#define EXIT 91
#define TRAVEL_STATS 95
#define NONE 99

#define MAX_DIGS 5
#define HEAD (1 + MAX_DIGS) // Bytes reserved for the <header> part of the message


#include "bloom.h"

int msg_len_rd;
// Return code so that the program knows which query to do.
int get_opcode(char *message);

// read message from named pipe
char *get_pipe_message(char *message);

// write message to named pipe
int write_msg_to_pipe(int fd, int opcode, char *message, int bufferSize);
// write message with bloom filter bit array to named pipe
int write_bf_msg(int fd, int opcode, char *message, int bufferSize,int msg_len, bloom_filter *bf);

//block signals till reading is done
int signal_read(int fd, char *buf, int read_num);
int read_bufsize(char *full_msg, int n, int fd, char *buf, int bufferSize);;
char *read_from_pipe(int read_end_fd, int bufferSize,bloom_filter *bf);