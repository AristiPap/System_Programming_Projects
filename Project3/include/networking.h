#pragma once 

#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <assert.h>

#define LOWEST_PORT 7615
#define HIGHEST_PORT 65535

struct hostent *find_ip();
int find_port();

