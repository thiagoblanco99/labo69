#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>

static const uint MAX_MSG = 65000;
static const uint MAX_NAME = 16;
typedef enum{
    VER_1 = 0x8001,
} Version;

typedef enum: uint8_t {
    TYPE_MENSAJE,//pongo si es mensaje para sala o para usuarios, y el payload
    TYPE_REQUEST_CREATE,//digo si quiero crear usuario o sala. Le paso nombre.
    TYPE_RESPONSE_CREATE,//respomnde el servidor si se puede crear.
    TYPE_REQUEST_CONNECT,//pido conexion a sala o usuario.
    TYPE_RESPONSE_CONNECT,//respuesta de la conexion de la sala o del usuario.
    TYPE_REQUEST_LISTA,//pido lista de usuarios y salas.
    TYPE_RESPONSE_LISTA,//envio lista solicitada
    TYPE_REQUEST_EXIT,//digo de salir de la sala o de una comunicacion C2C
    TYPE_RESPONSE_EXIT,//envio checkeo de salida de sesion.
    TYPE_REQUEST_DISCONNECT,//cierro la sesion
    TYPE_RESPONSE_DISCONNECT//acepto cierre de sesión 
} Type;

typedef struct __attribute__((__packed__)) // __attribute__((__packed__)) es para empaquetar bien las estructuras y que no haya agujeros en el medio de la memoria (padding)
{
    uint16_t version;
    uint8_t  type;
    // 1 byte de relleno// aca habria un byte de relleno si no usamos __attribute__((__packed__))
    uint32_t size8; // size8 es el tamaño del mensaje en bytes
} Header;


//MENSAJE ----------------------------------
typedef struct __attribute__((__packed__)) 
{
    uint16_t len16;
    char str[0]; // esto es pegriloso. No se puede hacer sizeof(VString) porque no se sabe el tamaño de str. Se puede hacer sizeof(VString) + len8
} VString;

typedef enum: uint8_t {
    MENSAJE_USER,
    MENSAJE_ROOM,
    MENSAJE_SERVER
}MODE_MENSAJE;

typedef struct __attribute__((__packed__)) 
{
    MODE_MENSAJE mode;
    char src[MAX_NAME];
    char dst[MAX_NAME];
    VString txt;
    //char buffer[MAX_MSG]; // ASI ESTA BIEN ? Y DESPUES CUANDO ESCRIBO EN EL SOCKET HAGO UN TRUNCATE DE ESTO? 
                        //si esta bien, pero podria hacer este buffer fuera del struct
                        // haciendo que el struct tenga un puntero a char en vez de un array de char
} MENSAJE;
//------------------------------------------


//REQUEST-CREATE----------------------------
typedef enum: uint8_t {
    REQUEST_USER_CREATE,
    REQUEST_ROOM_CREATE
}MODE_REQUEST_CREATE;

typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_CREATE mode;       
    char name[MAX_NAME];
}REQUEST_CREATE;
//RESPONSE-CREATE----------------------------
typedef enum: uint8_t{
    DENIED_CREATE,
    CORRECT_CREATE
}MODE_RESPONSE_CREATE;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_CREATE mode;
}RESPONSE_CREATE;
//-------------------------------------------

//REQUEST-CONNECTION-------------------------
typedef enum: uint8_t{
    REQUEST_USER_CONNECT,
    REQUEST_ROOM_CONNECT
}MODE_REQUEST_CONNECT;

typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_CONNECT mode;
    char name[MAX_NAME];
}REQUEST_CONNECT;
//RESPONSE-CONNECTION-------------------------
typedef enum: uint8_t{
    DENIED_CONNECT,
    COPRRECT_CONNECT
}MODE_RESPONSE_CONNECT;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_CONNECT mode;
}RESPONSE_CONNECT;
//--------------------------------------------

//REQUEST-LISTA------------------------------
typedef enum: uint8_t{
    REQUEST_LISTA_USER,
    REQUEST_LISTA_ROOM
}MODE_REQUEST_LISTA;
typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_LISTA mode;
}REQUEST_LISTA;
//RESPONSE-LISTA------------------------------
typedef enum: uint8_t{
    DENIED_LISTA,
    COPRRECT_LISTA
}MODE_RESPONSE_LISTA;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_LISTA mode; // TENGO QUE COMPLETAR ESTO PARA QUE MANDE EL STRING DE LA LISTA.// ESTABA PENSANDO EN QUE MANDE EL PAQUETE LISTA ACCEPTADA Y UN MENSAJE CON LA LISTA  
}RESPONSE_LISTA;
//--------------------------------------------

//REQUEST-EXIT--------------------------------
typedef struct __attribute__((__packed__))
{
    char name[MAX_NAME];
}REQUEST_EXIT;
//RESPONSE-EXIT-------------------------------
typedef enum: uint8_t{
    DENIED_EXIT,
    COPRRECT_EXIT
}MODE_RESPONSE_EXIT;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_EXIT mode;
}RESPONSE_EXIT;
//--------------------------------------------

//REQUEST-DISCONNECT-------------------------------
typedef struct __attribute__((__packed__))
{
    uint8_t mode=1;
}REQUEST_DISCONNECT;
//RESPONSE-DISCONNECT------------------------------
typedef enum: uint8_t{
    DENIED_DISCONNECT,
    COPRRECT_DISCONNECT
}MODE_RESPONSE_DISCONNECT;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_DISCONNECT mode;
}RESPONSE_DISCONNECT;
//-------------------------------------------------


//PACKAGE-WITH-PAYLOAD-----------------------------
typedef struct __attribute__((__packed__))
{
    Header hdr;

    union __attribute__((__packed__)) {
        MENSAJE msg;
        REQUEST_CREATE req_create;
        RESPONSE_CREATE res_create;
        REQUEST_CONNECT req_connect;
        RESPONSE_CONNECT res_connect;
        REQUEST_LISTA req_lista;
        RESPONSE_LISTA res_lista;
        REQUEST_EXIT req_exit;
        RESPONSE_EXIT res_exit;
        REQUEST_DISCONNECT req_disconnect;
        RESPONSE_DISCONNECT res_disconnect;
    } payload;

}PACKAGE;
//-------------------------------------------------

//FUNCIONES----------------------------------------

//FUNCIONES PARA LEER HEADER.
inline static uint16_t getVersion(const Header *hdr)// inline hace que se compile la funcion en el lugar donde se llama, en vez de hacer un llamado a la funcion
{
    return ntohs(hdr->version);
}

inline static uint16_t getSize8(const Header *hdr)
{
    return ntohl(hdr->size8);
}

inline static Type getType(const Header *hdr)
{
    return (Type)hdr->type; 
}

//FUNCIONES PARA MENSAJE
inline static void setMENSAJE(PACKAGE *msg, MODE_MENSAJE mode, char *src, char *dst, char *txt, uint16_t lenmsg)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_MENSAJE;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(MENSAJE) + lenmsg);
    msg->payload.msg.mode = mode;
    msg->payload.msg.src = src;
    msg->payload.msg.dst = dst;
    msg->payload.msg.txt.len16 = htons(lenmsg);
    msg->payload.msg.txt.str = txt;
}

inline static uint8_t getModeMensaje(PACKAGE *msg)
{
    return msg->payload.msg.mode;
}

inline static char* getSrcMensaje(PACKAGE *msg)
{
    return msg->payload.msg.src;
}

inline static char* getDstMensaje(PACKAGE *msg)
{
    return msg->payload.msg.dst;
}

inline static uint16_t getLenMensaje(PACKAGE *msg)
{
    return ntohs(msg->payload.msg.txt.len16);
}

inline static char* getTxtMensaje(PACKAGE *msg)
{
    return msg->payload.msg.txt.str;
}






//----------------------------------------------------------------------------------
//FUNCIONES PARA CREATE
inline static void setCreateUser(PACKAGE *msg, char *nombre)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_CREATE;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_CREATE));
    msg->payload.req_create.mode = REQUEST_USER_CREATE;
    msg->payload.req_create.name = nombre;

}
inline static void setCreateRoom(PACKAGE *msg, char *nombre)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_CREATE;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_CREATE));
    msg->payload.req_create.mode = REQUEST_ROOM_CREATE;
    msg->payload.req_create.name = nombre;
}
inline static uint8_t getModeCreateRequest(PACKAGE *msg)
{
    return msg->payload.req_create.mode;
}

inline static char* getNameCreateRequest(PACKAGE *msg)
{
    return msg->payload.req_create.name;
}

inline static void setCreateResponse(PACKAGE *msg, MODE_RESPONSE_CREATE mode)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_RESPONSE_CREATE;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(RESPONSE_CREATE));
    msg->payload.res_create.mode = mode;char buffer[MAX_MSG];
}
inline static uint8_t getModeCreateResponse(PACKAGE *msg)
{
    return msg->payload.res_create.mode;
}
//-----------------------------------------------------------------------------

//FUNCIONES PARA CONNECT

inline static void setConnectUser(PACKAGE *msg, char *nombre)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_CONNECT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_CONNECT));
    msg->payload.req_connect.mode = REQUEST_USER_CONNECT;
    msg->payload.req_connect.name = nombre;
}

inline static void setConnectRoom(PACKAGE *msg, char *nombre)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_CONNECT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_CONNECT));
    msg->payload.req_connect.mode = REQUEST_ROOM_CONNECT;
    msg->payload.req_connect.name = nombre;
}

inline static uint8_t getModeConnectRequest(PACKAGE *msg)
{
    return msg->payload.req_connect.mode;
}

inline static char* getNameConnectRequest(PACKAGE *msg)
{
    return msg->payload.req_connect.name;
}

inline static uint8_t setConnectResponse(PACKAGE *msg, MODE_RESPONSE_CONNECT mode)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_RESPONSE_CONNECT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(RESPONSE_CONNECT));
    msg->payload.res_connect.mode = mode;
}
inline static uint8_t getModeConnectResponse(PACKAGE *msg)
{
    return msg->payload.res_connect.mode;
}
//-----------------------------------------------------------------------------

//FUNCIONES PARA LISTA

inline static void setListaUser(PACKAGE *msg)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_LISTA;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_LISTA));
    msg->payload.req_lista.mode = REQUEST_LISTA_USER;
}

inline static void setListaRoom(PACKAGE *msg)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_LISTA;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_LISTA));
    msg->payload.req_lista.mode = REQUEST_LISTA_ROOM;
}

inline static uint8_t getModeListaRequest(PACKAGE *msg)
{
    return msg->payload.req_lista.mode;
}

inline static void setListaResponse(PACKAGE *msg, MODE_RESPONSE_LISTA mode)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_RESPONSE_LISTA;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(RESPONSE_LISTA));
    msg->payload.res_lista.mode = mode;
}

inline static uint8_t getModeListaResponse(PACKAGE *msg)
{
    return msg->payload.res_lista.mode;
}

//-----------------------------------------------------------------------------

//FUNCIONES PARA EXIT

inline static void setExitRequest(PACKAGE *msg, char *nombre)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_EXIT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_EXIT));
    msg->payload.req_exit.name = nombre;
}

inline static char* getNameExitRequest(PACKAGE *msg)
{
    return msg->payload.req_exit.name;
}
inline static void setExitResponse(PACKAGE *msg, MODE_RESPONSE_EXIT mode)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_RESPONSE_EXIT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(RESPONSE_EXIT));
    msg->payload.res_exit.mode = mode;
}
inline static uint8_t getModeExitResponse(PACKAGE *msg)
{
    return msg->payload.res_exit.mode;
}
//-----------------------------------------------------------------------------

//FUNCIONES PARA DISCONNECT

inline static void setDisconnectRequest(PACKAGE *msg)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_REQUEST_DISCONNECT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(REQUEST_DISCONNECT));
}

inline static void setDisconnectResponse(PACKAGE *msg, MODE_RESPONSE_DISCONNECT mode)
{
    msg->hdr.version = htons(VER_1);
    msg->hdr.type = TYPE_RESPONSE_DISCONNECT;
    msg->hdr.size8 = htonl(sizeof(Header) + sizeof(RESPONSE_DISCONNECT));
    msg->payload.res_disconnect.mode = mode;
}

inline static uint8_t getModeDisconnectResponse(PACKAGE *msg)
{
    return msg->payload.res_disconnect.mode;
}

//-----------------------------------------------------------------------------

//FUNCIONES PARA MENSAJE


