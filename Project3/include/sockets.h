#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include "../include/networking.h"

in_addr_t _ip_addr_; // ip address of machine currently in
int _port_;   // port that the app will be listening
int connection_sockfd;  // client-server communication socket

int _create_socket();
int create_connection(int sock, in_addr_t ip, int port);
void bind_listen(int sock,in_addr_t ip, int port);
int _accept(int sock, struct sockaddr_in *ad, socklen_t *ad_length);