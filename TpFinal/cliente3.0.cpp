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
#define BUFFER_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// armo clase de cliente
class Cliente
{
private:
    int sockfd;
    sockaddr_in server_address;
    std::string usuario;
    std::string dst;
    fd_set readfds;
    int maxfd;
    std::vector<std::string> salas_recientes;
    std::vector<std::string> usuarios_recientes;
    int mode_msg = 0; // es para recordar si estaba mandando mensajes a usuario o a salas

public:
    PACKAGE pkt_write;
    PACKAGE pkt_read;
    Cliente()
    {
        this->server_address.sin_family = AF_INET;
        this->server_address.sin_port = htons(PORT);
        this->server_address.sin_addr.s_addr = INADDR_ANY;
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (this->sockfd < 0)
            error("ERROR opening socket");
        connect(this->sockfd, (struct sockaddr *)&this->server_address, sizeof(server_address));
        FD_ZERO(&this->readfds);
        FD_SET(STDIN_FILENO, &this->readfds);
        FD_SET(this->sockfd, &this->readfds);
        this->maxfd = std::max(STDIN_FILENO, this->sockfd) + 1;
        this->usuario = "default";
        this->dst = "default";
        std::string nombre;
        printf("Ingrese el nombre del usuario que desea: ");
        while (std::cin >> nombre)
        {
            if (nombre.length() > 16)
            {
                std::cout << "El nombre debe tener menos de 16 caracteres\n";
                std::cout << "Ingrese el nombre del usuario a crear: ";
            }
            else
            {
                break;
            }
        }
        // creo el paquete
        setCreateUser(&this->pkt_write, &nombre[0]);
        // envio el paquete
        if (send(this->sockfd, &this->pkt_write, sizeof(PACKAGE), 0) == -1)
        {
            std::cerr << "Error sending message\n";
            return;
        }
        else
        {
            memcpy(&this->usuario[0], &nombre[0], 16);
        }
    }
    void modeWrite(PACKAGE *pkt)
    {
        // presento las opciones
        printf("Tipee el numero de la opción que quiere:\n");
        printf("1. Cambiar nombre de Usuario\n");
        printf("2. Crear Sala\n");
        printf("3. Lista de Sala\n");
        printf("4. Cambiar destino\n");
        printf("5. Unirse a Sala \n");
        printf("6. Lista de Usuarios\n");
        printf("7. Salir de la sala\n");
        printf("8. desconectarse\n");
        printf("9. volver a modo escucha\n");
        // leo de standar input el entero seleccionado y hago un switch
        std::string optionStr;
        std::string payload;
        int option;
        while (std::cin >> optionStr)
        {
            try
            {
                option = std::stoi(optionStr);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Introduzca una opcion correcta.\n";
                continue;
            }
            break;
        }

        // copio usuario a src
        switch (option)
        {
        case 1:
            // creo el usuario
            std::cout << "Ingrese el nombre del usuario que desea: ";
            while (std::cin >> payload)
            {
                if (payload.length() > 16)
                {
                    std::cout << "El nombre debe tener menos de 16 caracteres\n";
                    std::cout << "Ingrese el nombre del usuario a crear: ";
                }
                else
                {
                    break;
                }
            }
            // creo el paquete
            setCreateUser(pkt, &payload[0]);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }
            else
            {
                memcpy(&this->usuario[0], &payload[0], 16);
            }

            return;
        case 2:
            // creo una sala
            std::cout << "Ingrese el nombre de la sala a crear: ";
            while (std::cin >> payload)
            {
                if (payload.length() > 16)
                {
                    std::cout << "El nombre debe tener menos de 16 caracteres\n";
                    std::cout << "Ingrese el nombre de la sala a crear: ";
                }
                else
                {
                    break;
                }
            }
            setCreateRoom(pkt, &payload[0]);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }

            return;
        case 3:
            // pido lista de salas
            setListaRoom(pkt);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }
            break;
        case 4: // cambiar destino
            if (this->usuario == "default")
            {
                printf("cambie el nombre de usuario antes de mandar un mensaje\n");
                break;
            }
            else
            {
                printf("esta es la lista de usuarios recientes:\n");
                for (int i = 0; i < usuarios_recientes.size(); i++)
                {
                    printf("%s\n", usuarios_recientes[i].c_str());
                }
                printf("esta es la lista de salas recientes:\n");
                for (int i = 0; i < salas_recientes.size(); i++)
                {
                    printf("%s\n", salas_recientes[i].c_str());
                }
                printf("Si desea escribirle a un usuario ingrese 1 \n si desea escribir a una sala ingrese 2:\n");
                std::cin >> mode_msg;
                if (mode_msg == 1)
                {
                    printf("Ingrese el nombre del usuario al que desea escribir: ");
                    std::cin >> this->dst;
                    printf("Escribir y dar enter para enviar\n");
                    setModeMensaje(pkt, MENSAJE_USER);
                }
                else if (mode_msg == 2)
                {
                    printf("Ingrese el nombre de la sala a la que desea escribir: ");
                    std::cin >> this->dst;
                    printf("Escribir y dar enter para enviar\n");
                    setModeMensaje(pkt, MENSAJE_ROOM);
                }
                else
                {
                    printf("opcion invalida\n");
                    break;
                }

                break;
            }
        case 6:
            // pido lista de usuarios
            setListaUser(pkt);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }
            break;
        case 5:
            // unirse a sala
            std::cout << "Ingrese el nombre de la sala a unirse: ";
            while (std::cin >> payload)
            {
                if (payload.length() > 16)
                {
                    std::cout << "El nombre debe tener menos de 16 caracteres\n";
                    std::cout << "Ingrese el nombre de la sala a unirse: ";
                }
                else
                {
                    break;
                }
            }
            setConnectRoom(pkt, &payload[0]);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }
            break;
        case 7:
            // salir de sala
            std::cout << "Ingrese el nombre de la sala a salir: ";
            while (std::cin >> payload)
            {
                if (payload.length() > 16)
                {
                    std::cout << "El nombre debe tener menos de 16 caracteres\n";
                    std::cout << "Ingrese el nombre de la sala a salir: ";
                }
                else
                {
                    break;
                }
            }
            setExitRequest(pkt, &payload[0]);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }
            break;
        case 8:
            // desconectarse
            setDisconnectRequest(pkt);
            // envio el paquete
            if (send(this->sockfd, pkt, sizeof(PACKAGE), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }
            break;
        case 9:
        default:
            printf("salio de las opciones\n");
            return;
        }
        return;
    }

    void modeRead(PACKAGE *pkt)
    {
        // leo que tipo de mensaje es
        switch (getType(&pkt->hdr))
        {
        case TYPE_MENSAJE:
            if (getModeMensaje(pkt) == MENSAJE_USER)
            {
                // si es mensaje de usuario
                // lo imprimo
                std::cout << getSrcMensaje(pkt) << ": " << getTxtMensaje(pkt) << std::endl;
            }
            if (getModeMensaje(pkt) == MENSAJE_ROOM)
            {
                // si es mensaje de sala
                // lo imprimo
                std::cout << "sala " << getDstMensaje(pkt) << ":" << getSrcMensaje(pkt) << ": " << getTxtMensaje(pkt) << std::endl;
                // HAY QUE MODIFICAR ESTO PARA QUE QUEDE MAS LINDO
            }
            if (getModeMensaje(pkt) == MENSAJE_SERVER)
            {
                std::cout << getSrcMensaje(pkt) << ": " << getTxtMensaje(pkt) << std::endl;
            }
            break;

        case TYPE_RESPONSE_CREATE:
            // si es respuesta de creacion de usuario
            // imprimo el resultado
            if (getModeCreateResponse(pkt) == CORRECT_CREATE)
            {
                printf("creado con exito\n");
                break;
            }
            if (getModeCreateResponse(pkt) == DENIED_CREATE)
            {
                printf("Error al crear\n");
                break;
            }
            break;

        case TYPE_RESPONSE_CONNECT:
            // si es respuesta de creacion de sala
            // imprimo el resultado
            if (getModeConnectResponse(pkt) == CORRECT_CONNECT)
            {
                printf("conectado con exito\n");
                break;
            }
            if (getModeConnectResponse(pkt) == DENIED_CONNECT)
            {
                printf("Error al conectarse\n");
                break;
            }
            break;

        case TYPE_RESPONSE_LISTA:
            // si es respuesta de lista de salas
            // imprimo la lista
            if (getModeListaResponse(pkt) == CORRECT_LISTA)
            {
                printf(" Pedido aceptado\n");
                break;
            }
            if (getModeListaResponse(pkt) == DENIED_LISTA)
            {
                printf("Error al pedir lista\n");
                break;
            }
            break;

        case TYPE_RESPONSE_EXIT:
            // si es respuesta de salir de sala
            // imprimo el resultado
            if (getModeExitResponse(pkt) == CORRECT_EXIT)
            {
                std::cout << "Sala abandonada con exito\n";
                break;
            }
            if (getModeExitResponse(pkt) == DENIED_EXIT)
            {
                std::cout << "Error al abandonar sala\n";
                break;
            }
            break;

        case TYPE_RESPONSE_DISCONNECT:
            // si es respuesta de desconectarse
            // imprimo el resultado
            if (getModeDisconnectResponse(pkt) == CORRECT_DISCONNECT)
            {
                std::cout << "Desconectado con exito\n";
            }
            if (getModeDisconnectResponse(pkt) == DENIED_DISCONNECT)
            {
                std::cout << "Error al desconectarse\n";
            }
            break;
        }
    }

    void work()
    {
        while (true)
        {
            fd_set tmpfds = readfds;
            int activity = select(this->maxfd, &tmpfds, nullptr, nullptr, nullptr);
            if (activity == -1)
            {
                std::cerr << "Error in select()\n";
            }

            // Check if there is data to read from the socket
            if (FD_ISSET(this->sockfd, &tmpfds))
            {

                ssize_t bytesRead = read(this->sockfd, &this->pkt_read, sizeof(this->pkt_read));
                if (bytesRead == -1)
                {
                    std::cerr << "Error reading from socket\n";
                    break;
                }
                else if (bytesRead == 0)
                {
                    std::cout << "Connection closed by server\n";
                    break;
                }
                modeRead(&this->pkt_read);
            }

            // Check if there is input from standard input
            if (FD_ISSET(STDIN_FILENO, &tmpfds))
            {
                std::string input;
                std::getline(std::cin, input);
                if (input.empty())
                {
                    continue;
                }
                else
                {
                    clearPacket(&this->pkt_write); // limpio el paquete para que no queden cosas de antes
                    // Busco en el array de entrada si se quiere setear algo
                    if (input.find("@mode") != std::string::npos)
                    {
                        modeWrite(&this->pkt_write);
                        std::string dummy;
                        std::getline(std::cin, dummy);
                        FD_SET(STDIN_FILENO, &this->readfds);
                        this->maxfd = std::max(STDIN_FILENO, this->sockfd) + 1;
                    }
                    else
                    {
                        if (this->dst == "default")
                        {
                            printf("no se puede mandar un mensaje sin antes seleccionar un destinatario\n");
                        }
                        else
                        {
                            if (mode_msg == 1)
                            {
                                setModeMensaje(&this->pkt_write, MENSAJE_USER);
                            }
                            else if (mode_msg == 2)
                            {
                                setModeMensaje(&this->pkt_write, MENSAJE_ROOM);
                            } // esto es para que no se rompa cuando no se selecciono un modo
                            else
                            {
                                printf("no se puede mandar un mensaje sin antes seleccionar un destinatario\n");
                                break;
                            }
                            setMENSAJE(&this->pkt_write, &this->usuario[0], &this->dst[0], &input[0], input.size());
                            // PORQUE NO PUEDO LEER ESTO ? std::cout<<"mode: "<<getModeMensaje(&this->pkt_write)<<std::endl;
                            if (send(this->sockfd, &this->pkt_write, sizeof(this->pkt_write), 0) == -1)
                            {
                                std::cerr << "Error sending message\n";
                            }
                        }
                    }
                }
            }
        }
    }
};

int main()
{
    Cliente cliente;
    cliente.work();
    return 0;
}