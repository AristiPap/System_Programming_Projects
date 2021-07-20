#include "../../include/sockets.h"
#include "poll.h"
#include "signal.h"
#include "../../include/ipc.h"


int _create_socket(){
  int sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    perror("socket @ net.c");

  printf("Opening socket fd: %d\n", sock);
  int enable = 1;
  // Mark the socket's address to be re-usable
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
    perror("setsockopt @ net.c");

  return sock;
}

int create_connection(int sock, in_addr_t ip, int port){
      // set the sockaddr struct for the connect function
      struct sockaddr_in ad;
      memset(&ad, 0, sizeof(struct sockaddr_in));
      ad.sin_family = AF_INET;               
      ad.sin_addr.s_addr = htonl(ip);   
      ad.sin_port = htons(port);            
    
      int connect_fd = -1;
      int tries = 1000;
      while(connect_fd != 0 && tries){
          connect_fd = connect(sock, (struct sockaddr *)&ad, sizeof(ad));   
          if (tries % 10 == 0)
            sleep(1); // every 10 tries sleep for a sec
          tries --;
      }
      if (connect_fd == -1) 
          perror("Connection Failure");
  
      return connect_fd;
}

void bind_listen(int sock,in_addr_t ip, int port){
  //bind will almost be the same as connect reffering to the coding part of the excercise 
  //but will differ to the socket function called
  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = htonl(ip);
  ad.sin_port = htons(port);
  
  int binded = bind(sock,(struct sockaddr *)&ad, sizeof(ad));
  if(binded < 0){
    perror("bind");
    exit(1);
  } 
  
  int listener;
  
  listener = listen(sock, 1);
  if(listener < 0)
    perror("listen failed");
  
  //return binded;
}

int _accept(int sock, struct sockaddr_in *ad, socklen_t *ad_length){
  int fd = -1;
  struct sockaddr_in sa;
  memset(&sa,0,sizeof(sa));
  socklen_t sa_size = sizeof(sa);
  
  fd = accept(sock,(struct sockaddr *)&sa, &sa_size);
  if(fd<0)
    perror("accept failed");
    
  if(ad!=NULL){
    memcpy(ad,&sa,sa_size);
    *ad_length = sa_size;
  }
  return fd;
}
