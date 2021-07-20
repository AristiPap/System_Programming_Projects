#include "../../include/networking.h"

struct hostent *find_ip() {
    char hostname[BUFSIZ];
    memset(hostname, 0, BUFSIZ);
    gethostname(hostname, BUFSIZ);
    struct hostent *ad = NULL;
    if ((ad = gethostbyname(hostname)) == NULL)
        perror("Couldn't get ip of current working pc...\n");

    return ad;
}

int find_port(){
    static int port = LOWEST_PORT;
    
    if(port == HIGHEST_PORT)
        perror("No ports to use\n");    
    
    port++;
    
    return port;
}