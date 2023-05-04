#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "proto.h"

void procPing(Ping *pingMsg)
{
}

void procPong(Pong *pongMsg)
{
}

void procUser(User *userMsg)
{
}

int doProto(int sockfd)
{
    Msg msgBuffer;
    int err = 1;

    while( 1 ) {
        if( (err = recvMsg(sockfd, &msgBuffer)) != 1 ) {

            switch( msgBuffer.hdr.type ) {
                case TYPE_PING:
                    procPing(&msgBuffer.payload.ping);
                    break;
                case TYPE_PONG:
                    procPong(&msgBuffer.payload.pong);
                    break;
                case TYPE_USER:
                    procUser(&msgBuffer.payload.user);
                    break;
                default:
                    return 0;
            }
        } else
            return err;
    }
    
    return 1;
}



int main()
{
    Header hdr;
    Msg msg;

    printf("sizeof(Header): %ld\n", sizeof(Header));

    printf("&hdr: %p\n", &hdr);
    printf("&hdr.version: %p\n", &hdr.version);
    printf("&hdr.type: %p\n", &hdr.type);
    printf("&hdr.size8: %p\n", &hdr.size8);

    setPingV1(&msg, 0xbadc0ffe);

    uint32_t algo = getPingV1Algo(&msg);
    printf("algo: 0x%x\n", algo);

    return 0;
}