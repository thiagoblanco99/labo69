#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "proto.h"

void printType(const Msg *msg)
{
    printf("Type: %x\n", msg->hdr.type);
}

// Envia un Msg, el Msg est'a en un buffer lineal,
// reintento hasta mandarlo entero
// retorna 0 si cerr'o el socket, -1 en caso de error
// retorna 1 si est'a ok
int sendMsg(int sockfd, const Msg *msg)
{
    size_t toSend = ntohs(msg->hdr.size8);
    ssize_t sent;
    uint8_t *ptr = (uint8_t *) msg;

    while( toSend ) {
        sent = send(sockfd, ptr, toSend, 0);
        if( (sent == -1 && errno != EINTR) || sent == 0 )
            return sent;
        toSend -= sent;
        ptr += sent;
    }
    return 1;
}

// Recibe un Msg, no sabemos el tama~no, hay que recibir
// primero el header, y despu'es el payload cuando sepamos el sz
// reintento hasta recibirlo entero
// retorna 0 si cerr'o el socket, -1 en caso de error
// retorna 1 si est'a ok
int recvMsg(int sockfd, Msg *msg)
{
    size_t toRecv = sizeof(Header);
    ssize_t recvd;
    uint8_t *ptr = (uint8_t *) &msg->hdr;
    int headerRecvd = 0;

    while( toRecv ) {
        recvd = recv(sockfd, ptr, toRecv, 0);
        if( (recvd == -1 && errno != EINTR) || recvd == 0 )
            return recvd;
        toRecv -= recvd;
        ptr += recvd;
        if( toRecv == 0 && headerRecvd == 0) {
            headerRecvd = 1;
            ptr = (uint8_t *) &msg->payload;
            toRecv = ntohs(msg->hdr.size8) - sizeof(Header);
        }
    }
    return 1;
}
