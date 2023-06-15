#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <time.h>
#include <mutex>
#include <semaphore.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocolo.h"

#define PORT 5001
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main()
{
    int n;
    char buffer[256];
//hago la conexion
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
//-----------------------
//seteo el mensaje
PACKAGE pkt;
char src[MAX_NAME];
char dst[MAX_NAME];
char txt[20];

//seteo txt
strcpy(txt,"hola, es una prueba");
//seteo src
strcpy(src,"juan");
//seteo dst
strcpy(dst,"pedro");
//seteo pkt
setMENSAJE(&pkt,MENSAJE_USER,src,dst,txt,20);
n = write(sockfd,&pkt,sizeof(pkt));
    if (n < 0) 
         error("ERROR writing to socket");


//mando un mensaje al server
/*for(int i=0; i<4;i++){  
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
*/
//---------------------------
//recibo mensaje del server
    bzero(buffer,256);
    n = read(sockfd,buffer,256);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);

//--------------------------
/*PACKAGE pkt;
char src[MAX_NAME];
char dst[MAX_NAME];
char txt[20];
char src2[MAX_NAME];
char dst2[MAX_NAME];
char txt2[20];
//seteo txt
strcpy(txt,"hola");
//seteo src
strcpy(src,"juan");
//seteo dst
strcpy(dst,"pedro");
//seteo pkt
setMENSAJE(&pkt,MENSAJE_USER,src,dst,txt,20);
//leo mensaje
memcpy(txt2,getTxtMensaje(&pkt),getLenMensaje(&pkt));
printf("%s\n",txt2);
//leo src
memcpy(src2,getSrcMensaje(&pkt),16);
printf("%s\n",src2);
//leo dst
memcpy(dst2,getDstMensaje(&pkt),16);
printf("%s\n",dst2);
//leo tipo*/

return 0;
}