
#include "../../include/header.h"
#include "../../include/ipc.h"
#include <bits/types.h>

int get_opcode(char *message) {
  return (int)message[0];
}

// Return the actual message transmitted.
char *get_pipe_message(char *message) {
  return &(message[HEAD]);
}

//Send message to pipe
int write_msg_to_pipe(int fd, int opcode, char *message, int bufferSize)
{
  int length = strlen(message);
  char *full_msg = malloc((length + HEAD + 1) * sizeof(char));

  // Compose the full message
  full_msg[0] =(char)opcode;
  snprintf(full_msg+1, MAX_DIGS+1, "%0*d", MAX_DIGS, length);
  snprintf(full_msg+HEAD, length+1, "%s", message);

  length = strlen(full_msg);
  char buf[bufferSize];

  int total_sent = 0;
  int bytes_to_write=0;
  //find how many parts we have to write to the pipe according to the buffersize 
  int times = length / bufferSize;
  int temp_len = length;
  
  for(int i=0; i<=times + 1; i++){
    if(bufferSize<=temp_len)
      bytes_to_write = bufferSize;
    else
      bytes_to_write = temp_len;
    
    strncpy(buf, full_msg + total_sent, bytes_to_write);  // Copy bytes from message to buffer

    int check = write(fd, buf, bytes_to_write);
    if (check < 0) {
      perror("write @ write_msg_to_pipe");
      free(full_msg);
      exit(1);
    }
    total_sent+=check;
    if(total_sent>=length)
      break;
    temp_len = temp_len - check;
  }

  free(full_msg);
  return 0;
} 


static int find_occurences3(char *str, char toSearch){
  int count = 0;
  int i=0;
  while(str[i] != '\0'){
    if(str[i] == toSearch){
      count++;
    }  
    if(str[i]=='=' && toSearch=='=')
      return i;
    i++;
  }
  return i+1;
}

//same as before
int write_bf_msg(int fd, int opcode, char *message, int bufferSize,int msg_len,bloom_filter *bf)
{
  char *full_msg = malloc((msg_len + HEAD + 1) * sizeof(char)); // <body> + <header> + \0
  int blf_index = find_occurences3(message,'=');
  int my_bufferSize = blf_index +1+ HEAD;
  // Compose the full message
  full_msg[0] = opcode;
  snprintf(full_msg+1, MAX_DIGS+1, "%0*d", MAX_DIGS, msg_len);
  memcpy(&(full_msg[HEAD]),message,msg_len*sizeof(char));

  msg_len += HEAD;
  char buf[my_bufferSize];

  int total_sent = 0;
  int count=0;
  int _index = 0;
  while (total_sent < msg_len)
  {
    if(count==1){
        char _buf[bf->bloom_size];
        memcpy(_buf,bf->bit_array,ceil(bf->bloom_size/8)*sizeof(uint8_t));
  
        int written = write(fd, bf->bit_array, (int)ceil(bf->bloom_size/8));
        if (written < 0) {
          perror("write @ write_msg_to_pipe");
          exit(1);
        }
  
        total_sent += msg_len;
    }
    else{
      int diff = msg_len - total_sent;  // We might need to write less bytes
      int write_num = (diff < my_bufferSize) ? diff : my_bufferSize;  // than <bufferSize>
  
      strncpy(buf, full_msg + total_sent, write_num);  // Copy bytes from message to buffer
      int written = write(fd, buf, write_num);
      if (written < 0) {
        perror("write @ write_msg_to_pipe");
        exit(1);
      }
      count++;

      total_sent += written;
    }  
  }

  free(full_msg);
  return 0;
}

int signal_read(int fd, char *buf, int read_num){
  int data_read = read(fd, buf, read_num);
  if (data_read == -1)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
      return 0;         // Ignore signals till we have finished reading from fd
    else{
      perror("read @ read_from_pipe");
      exit(1);
    }
  }

  return data_read;
}

char *read_from_pipe(int fd, int bufferSize,bloom_filter *bf)
{
  char buf[bufferSize];
  memset(buf, 0, bufferSize);

  char head_msg[HEAD+1];  // <header> + \0

  if (read_bufsize(head_msg, 1, fd, buf, bufferSize) == -1)  // Read <opcode>
    return NULL;
    
  // Read rest of <header>
  read_bufsize(head_msg + 1, MAX_DIGS, fd, buf, bufferSize);

  head_msg[HEAD] = '\0';
  
  int bytes_to_read = atoi(head_msg+1); // Get the # bytes to read
  // if bytes to read > bloom filter size then we are at the init stage and we have to read uint8_t bit array
  if(bytes_to_read>(int)ceil(bf->bloom_size/8)){
    
    bytes_to_read = bytes_to_read - (int)ceil(bf->bloom_size/8);
    char *full_msg = calloc((HEAD + bytes_to_read + 1), sizeof(char));
    strcpy(full_msg, head_msg);
  
    //read message that comes before the bloom filter
    read_bufsize(full_msg + HEAD, bytes_to_read, fd, buf, bufferSize);

    full_msg[HEAD + bytes_to_read ] = '\0';

    msg_len_rd=bytes_to_read ;
    
     //read bloom filter bit array
    int data_read;
    int chars_read = 0;  // Chars read so far
    int n =  (int)ceil(bf->bloom_size/8);
    char _buf[n];
    ////////////////////////////////////
    while (chars_read < n)
    {
      data_read = signal_read(fd, _buf, n);
      if (data_read == -1)
        return -1;
      if (data_read == 0)
        continue;
      memcpy(bf->bit_array,_buf, n*sizeof(uint8_t));
      return full_msg;
      chars_read += n;
    }
    
    return full_msg;
  }
  else{
    char *full_msg = calloc((HEAD + bytes_to_read + 1), sizeof(char));
    strcpy(full_msg, head_msg);
    
    //read rest of message
    read_bufsize(full_msg + HEAD, bytes_to_read, fd, buf, bufferSize);
  
    full_msg[HEAD + bytes_to_read] = '\0';
  
    msg_len_rd=bytes_to_read;
    return full_msg;
  }
}


int read_bufsize(char *full_msg, int n, int fd, char *buf, int bufferSize)
{
  int diff, read_num, data_read;
  int chars_read = 0;  // Chars read so far

  while (chars_read < n)
  {
    diff = n - chars_read;  // If less bytes than <bufferSize> remain, read less.
    read_num = (diff < bufferSize) ? diff : bufferSize;

    data_read = signal_read(fd, buf, read_num);
    if (data_read == -1)
      return -1;
    if (data_read == 0)
      continue;

    strncpy(full_msg + chars_read, buf, read_num);
    chars_read += data_read;
  }

  return chars_read;
}
