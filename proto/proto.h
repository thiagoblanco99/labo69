#ifndef PROTO_H_
#define PROTO_H_ 1

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>

typedef enum {
    VER_1 = 0x8001,
} Version;

typedef enum {
    TYPE_PING,
    TYPE_PONG,
    TYPE_USER,
} Type;

typedef struct __attribute__((__packed__)) // __attribute__((__packed__)) es para empaquetar bien las estructuras y que no haya agujeros en el medio de la memoria (padding)
{
    uint16_t version;
    uint8_t  type;
    // 1 byte de relleno// aca habria un byte de relleno si no usamos __attribute__((__packed__))
    uint16_t size8; // size8 es el tamaño del mensaje en bytes
} Header;

inline static uint16_t getVersion(const Header *hdr)// inline hace que se compile la funcion en el lugar donde se llama, en vez de hacer un llamado a la funcion
{
    return ntohs(hdr->version);
}

inline static uint16_t getSize8(const Header *hdr)
{
    return ntohs(hdr->size8);
}

typedef struct __attribute__((__packed__)) 
{
    uint8_t len8;
    char str[0]; // esto es pegriloso. No se puede hacer sizeof(VString) porque no se sabe el tamaño de str. Se puede hacer sizeof(VString) + len8
} VString;



typedef struct __attribute__((__packed__)) 
{
    uint32_t algo;
} Ping;

#define AAA_SZ8 64UL // unsigned long iniciado con numero 64.
// static const size_t AAA_SZ8 = 64;

typedef struct __attribute__((__packed__)) 
{
    uint8_t aaa[AAA_SZ8];
} Pong;

typedef struct __attribute__((__packed__)) 
{
    VString username;
} User;



typedef struct __attribute__((__packed__))
{
    Header hdr;

    union __attribute__((__packed__)) { // union permite guardar el mismo espacio de memoria para distintos tipos de datos y el espacio es del tipo mas grande de los que se guardan. Es similar al struct.
        Ping ping;
        Pong pong;
        User user;
    } payload;

} Msg;

inline static void setPingV1(Msg *msg, uint32_t algo)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_PING;
    msg->hdr.size8 = htons(sizeof(Header) + sizeof(Ping));
    msg->payload.ping.algo = htonl(algo);
}

inline static uint32_t getPingV1Algo(Msg *msg)
{
    assert(msg->hdr.type == TYPE_PING);
    return ntohl(msg->payload.ping.algo);
}

inline static void setUserV1(Msg *msg, const char *u)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_USER;
    size_t len8 = strlen(u); // 
    assert(len8 <= sizeof(uint8_t)); // no puede ser mayor a 255
    assert(len8 != 0); // no puede ser 0 porque el usuario no puede ser vacio
    memcpy(msg->payload.user.username.str, u, len8); // copia el usuario en el payload
    msg->payload.user.username.len8 = len8 - 1; // le resta 1 porque el len8 es el tamaño del usuario + 1
    msg->hdr.size8 = htons(sizeof(Header) + sizeof(VString)) + len8; // el tamaño del header + el tamaño del usuario
}

inline static const char *getUserNameV1(Msg *msg)
{
      assert(msg->hdr.type == TYPE_USER);
      return msg->payload.user.username.str;
}


void printType(const Msg *m);

int sendMsg(int sockfd, const Msg *msg);
int recvMsg(int sockfd, Msg *msg);

#endif // PROTO_H_
