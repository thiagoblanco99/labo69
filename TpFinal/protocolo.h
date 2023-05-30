#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>

static const int MAX_MSG = 65536;
static const int MAX_NAME = 16;
typedef enum {
    VER_1 = 0x8001,
} Version;

typedef enum {
    TYPE_MENSAJE,//pongo si es mensaje para sala o para usuarios, y el payload
    TYPE_REQUEST_CREATE,//digo si quiero crear usuario o sala. Le paso nombre.
    TYPE_RESPONSE_CREATE,//respomnde el servidor si se puede crear.
    TYPE_REQUEST_CONNECT,//pido conexion a sala o usuario.
    TYPE_RESPONSE_CONNECT,//respuesta de la conexion de la sala o del usuario.
    TYPE_REQUEST_LISTA,//pido lista de usuarios y salas.
    TYPE_RESPONSE_LISTA,//envio lista solicitada
    TYPE_REQUEST_EXIT,//digo de salir de la sala o de una comunicacion C2C
    TYPE_RESPONSE_EXIT,//envio checkeo de salida de sesion.
    TYPE_DISCONNECT//cierro la sesion 
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
//MENSAJE ----------------------------------
typedef struct __attribute__((__packed__)) 
{
    uint16_t len16;
    char str[0]; // esto es pegriloso. No se puede hacer sizeof(VString) porque no se sabe el tamaño de str. Se puede hacer sizeof(VString) + len8
} VString;

typedef struct __attribute__((__packed__)) 
{
    char dst[MAX_NAME];
    VString txt;
    char dst[MAX_MSG]; // CHEQUEAR BIEN COMO SE MANDA EL MENSAJE.
} MENSAJE;
//------------------------------------------

//REQUEST-CREATE----------------------------
typedef enum {
    REQUEST_USER,
    REQUEST_ROOM
}MODE_REQUEST_CREATE;

typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_CREATE mode;       
    char name[16];
}REQUEST_CREATE;
//RESPONSE-CREATE----------------------------
typedef enum{
    DENIED_CREATE,
    CORRECT_CREATE
}MODE_RESPONSE_CREATE;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_CREATE mode;
}RESPONSE_CREATE;
//-------------------------------------------

//REQUEST-CONNECTION-------------------------
typedef enum{
    REQUEST_C2C,
    REQUEST_ROOM
}MODE_REQUEST_CONNECT;

typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_CONNECT mode;
    char name[16];
}REQUEST_CONNECT;
//RESPONSE-CONNECTION-------------------------
typedef enum{
    DENIED_CREATE,
    COPRRECT_CREATE
}MODE_RESPONSE_CONNECT;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_CONNECT mode;
}RESPONSE_CONNECT;
//--------------------------------------------

//REQUEST-LISTA------------------------------
typedef enum{
    REQUEST_LISTA_USER,
    REQUEST_LISTA_ROOM
}MODE_REQUEST_LISTA;
typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_LISTA mode;
}REQUEST_CONNECT;
//RESPONSE-LISTA------------------------------
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_CONNECT mode; // TENGO QUE COMPLETAR ESTO PARA QUE MANDE EL STRING DE LA LISTA.
}RESPONSE_CONNECT;
//--------------------------------------------

//REQUEST-EXIT--------------------------------
typedef enum{
    REQUEST_C2C,
    REQUEST_ROOM
}MODE_REQUEST_EXIT;

typedef struct __attribute__((__packed__))
{
    MODE_REQUEST_EXIT mode;
}REQUEST_EXIT;
//RESPONSE-EXIT-------------------------------
typedef enum{
    DENIED_EXIT,
    COPRRECT_EXIT
}MODE_RESPONSE_EXIT;
typedef struct __attribute__((__packed__))
{
    MODE_RESPONSE_EXIT mode;
}RESPONSE_EXIT;
//--------------------------------------------

