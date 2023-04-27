#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd, i = 0;
//    u_char ttl = 2;
    char message[64];

    /* create what looks like an ordinary UDP socket */
    if( (fd = socket(AF_INET, SOCK_DGRAM,0)) < 0 ) {
        perror("socket");
        exit(1);
    }
/*    if( setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0 ) {
        perror("Setting TTL failed");
        exit(1);
    }
*/
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(HELLO_GROUP);
    addr.sin_port=htons(HELLO_PORT);

    /* now just sendto() our destination! */
    while( 1 ) {
        sprintf(message, "hola %03d", i++);
        if( sendto(fd,message,1+strlen(message),0,(struct sockaddr *) &addr, sizeof(addr)) < 0 ) {
            perror("sendto");
            exit(1);
        }
        fputs(".", stdout); fflush(stdout);
        sleep(1);
    }
    return 0;
}
