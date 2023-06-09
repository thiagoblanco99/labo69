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
constexpr int BUFFER_SIZE = 256;
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void modewrite(int socket, sockaddr_in *server_address, std::string *usuario)
{
    // presento las opciones
    printf("Tipee el numero de la opción que quiere:\n");
    printf("1. Crear Usuario\n");
    printf("2. Crear Sala\n");
    printf("3. Lista de Sala\n");
    printf("4. Enviar Mensaje\n");
    printf("5. Unirse a Sala \n");
    printf("6. Lista de Usuarios\n");
    printf("7. Salir de la sala\n");
    printf("8. desconectarse\n");
    // leo de standar input el entero seleccionado y hago un switch
    int option;
    std::cin >> option;
    PACKAGE pkt;
    std::string name, dst;
    std::string src;
    // copio usuario a src
    memcpy(&src[0], &usuario[0], 16);
    switch (option)
    {
    case 1:
        // creo el usuario
        std::cout << "Ingrese el nombre del usuario a crear: ";
        while (std::cin >> name)
        {
            if (name.length() > 16)
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
        setCreateUser(&pkt, &name[0]);
        // envio el paquete
        if (send(socket, &pkt, sizeof(pkt), 0) == -1)
        {
            std::cerr << "Error sending message\n";
            return;
        }
        else
        {
            memcpy(&usuario[0], &name[0], 16);
        }

        break;
    case 2:
        // creo una sala
        std::cout << "Ingrese el nombre de la sala a crear: ";
        while (std::cin >> name)
        {
            if (name.length() > 16)
            {
                std::cout << "El nombre debe tener menos de 16 caracteres\n";
                std::cout << "Ingrese el nombre de la sala a crear: ";
            }
            else
            {
                break;
            }
        }
        setCreateRoom(&pkt, &name[0]);
        // envio el paquete
        if (send(socket, &pkt, sizeof(pkt), 0) == -1)
        {
            std::cerr << "Error sending message\n";
            return;
        }

        break;
    case 3:
        // pido lista de salas
        break;

    case 4: // enviar mensaje
    default:
        if (src == "default")
        {
            printf("cree un usuario antes de mandar un mensaje\n");
        }
        else
        {
            printf("Escribir a quien le quiere mandar el mensaje:\n");
            std::cin >> dst;
            printf("Escribir el mensaje:\n");
            std::cin >> name;

            setMENSAJE(&pkt, MENSAJE_USER, &src[0], &dst[0], &name[0], sizeof(name));
            if (send(socket, &pkt, sizeof(pkt), 0) == -1)
            {
                std::cerr << "Error sending message\n";
                return;
            }

            break;
        }
        return;
    }
}

int main()
{
    std::string usuario = "default";
    // Establish the socket connection
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));

    // Set up file descriptor sets for select()
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sockfd, &readfds);
    int maxfd = std::max(STDIN_FILENO, sockfd) + 1;

    while (true)
    {
        fd_set tmpfds = readfds;
        int activity = select(maxfd, &tmpfds, nullptr, nullptr, nullptr);
        if (activity == -1)
        {
            std::cerr << "Error in select()\n";
            return 1;
        }

        // Check if there is data to read from the socket
        if (FD_ISSET(sockfd, &tmpfds))
        {
            PACKAGE pkt;
            ssize_t bytesRead = read(sockfd, &pkt, sizeof(pkt));
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
            std::cout << getSrcMensaje(&pkt) << ": " << getTxtMensaje(&pkt) << std::endl;
        }

        // Check if there is input from standard input
        if (FD_ISSET(STDIN_FILENO, &tmpfds))
        {
            std::string input;
            std::getline(std::cin, input);
            if (input.empty())
                break; // Exit the loop if an empty line is entered

            // Check if the input contains "@mode"
            if (input.find("@mode") != std::string::npos)
            {
                modewrite(sockfd, &server_address, &usuario);
                std::string dummy;
                std::getline(std::cin, dummy);
                FD_SET(STDIN_FILENO, &readfds);
                maxfd = std::max(STDIN_FILENO, sockfd) + 1;
            }
            else
            {
                ssize_t bytesSent = send(sockfd, input.c_str(), input.length(), 0);
                if (bytesSent == -1)
                {
                    std::cerr << "Error writing to socket\n";
                    break;
                }
            }
        }
    }

    // Close the socket connection
    close(sockfd);

    return 0;
}