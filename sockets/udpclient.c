#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h> 

#define BUFLEN 512
#define NPACK 10
#define PORT 9930

void die(char *s)
{
  perror(s);
  exit(1);
}

int main(int argc, char *argv[])
{
    int s, i;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    socklen_t slen=sizeof(serv_addr);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        die("socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(PORT);

    for (i=0; i<NPACK; i++) {
        printf("Sending packet %d\n", i);
        sprintf(buf, "This is packet %d\n", i);
        if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&serv_addr, slen)==-1)
            die("sendto()");
    }

    close(s);
    return 0;
}

