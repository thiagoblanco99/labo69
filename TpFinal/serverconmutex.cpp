
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <time.h>
#include <mutex>
#include <semaphore.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

#include "protocolo.h"

#define PORT 5001

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

class User
{
private:
public:
    // variables a usar//
    std::vector<std::string> salas;
    std::string name;
    int socket;
    sockaddr_in address;
    int mode;
    std::thread t_user;
    //----------------------------
    // mutex a usar
    std::mutex mut_salas;
    std::mutex mut_name;
    std::mutex mut_socket;
    std::mutex mut_address;
    std::mutex mut_mode;
    std::mutex mut_thread;
    //------------------------
    // constructror//
    User(std::string name_in, int socket_in, struct sockaddr_in address_in, std::function<void(User *)> reader_in)
    {
        this->name = name_in;
        this->socket = socket_in;
        this->address = address_in;
        this->t_user = std::thread(reader_in, this);
    }
    //destructor//
    ~User()
    {
        printf("se borra el usuario\n");
        this->t_user.join();
    }
    // getters//
    void setName(std::string name_in)
    {
        std::unique_lock<std::mutex> lck(mut_name);
        this->name = name_in;
    }
    std::string getName()
    {
        std::unique_lock<std::mutex> lck(mut_name);
        return this->name;
    }
    int getSocket()
    {
        std::unique_lock<std::mutex> lck(mut_socket);
        return this->socket;
    }
    int getip()
    { // obtengo ip del usuario
        std::unique_lock<std::mutex> lck(mut_address);
        return this->address.sin_addr.s_addr;
    }
    // agregar salas//
    void addSala(std::string name)
    {
        std::unique_lock<std::mutex> lck(mut_salas);
        this->salas.push_back(name); // completar
    }
    // borrar sala//
    void deleteSala(std::string name)
    {
        std::unique_lock<std::mutex> lck(mut_salas);
        for (int i = 0; i < salas.size(); i++)
        {
            if (salas[i] == name)
            {
                salas.erase(salas.begin() + i);
            }
        }
    }
    // imprimir salas//
    void printSalas()
    {
        std::unique_lock<std::mutex> lck(mut_salas);
        for (int i = 0; i < salas.size(); i++)
        {
            std::cout << salas[i] << std::endl;
        }
    }
};

class Room
{
private:
public:
    // variables a usar//
    std::string name;
    int id;
    std::vector<User *> users;
    User *owner;
    //------------------------
    // mutex a usar
    std::mutex mut_name;
    std::mutex mut_id;
    std::mutex mut_users;
    std::mutex mut_owner;
    //------------------------
    // constructor//
    Room(std::string name_in, User *owner_in)
    {
        this->name = name_in;
        this->users.push_back(owner_in); // lo agrego a los usuarios de la sala al dueño.
        this->owner = owner_in;          // lo pongo como dueño.
        this->owner->addSala(this->name);
    }
    // destructor//
    ~Room()
    {
        printf("se borra la sala\n");
    }
    // getters//
    std::string getName()
    {
        std::unique_lock<std::mutex> lck(mut_name);
        return this->name;
    }
    int getId()
    {
        std::unique_lock<std::mutex> lck(mut_id);
        return this->id;
    }
    std::string getOwner()
    {
        std::unique_lock<std::mutex> lck(mut_owner);
        return this->owner->getName();
    }
    // agregar usuarios//
    void addUser(User *user)
    {
        std::unique_lock<std::mutex> lck(mut_users);
        this->users.push_back(user);
        user->addSala(this->name);
    }
    // imprimir usuarios//
    void printUsers()
    {
        std::unique_lock<std::mutex> lck(mut_users);
        for (int i = 0; i < users.size(); i++)
        {
            std::cout << users[i]->getName() << std::endl;
        }
    }
    // eliminar usuarios//
    void deleteUser(User *user)
    {
        std::unique_lock<std::mutex> lck(mut_users);
        for (int i = 0; i < users.size(); i++)
        {
            if (this->users[i]->getName() == user->getName())
            {
                this->users.erase(this->users.begin() + i);
            }
        }
    }
};

class ChatServer
{
private:
public:
    std::vector<User *> users; // TENGO QUE HACER TODAS LAS FUNCIONES CON ESTOS PUNTEROS.
    std::vector<Room *> rooms; // TENGO QUE HACER TODAS LAS FUNCIONES CON ESTOS PUNTEROS.
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // mutex a usar
    std::mutex mut_users;
    std::mutex mut_rooms;
    // constructor//
    ChatServer()
    {
        if (this->sockfd < 0)
        {
            error("ERROR opening socket");
        }
        int val = 1;
        bzero((char *)&serv_addr, sizeof(serv_addr));
        this->serv_addr.sin_family = AF_INET;
        this->serv_addr.sin_addr.s_addr = INADDR_ANY; // ip de la maquina
        this->serv_addr.sin_port = htons(PORT);
        setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        if (bind(this->sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            error("ERROR on binding");
        }
        listen(this->sockfd, 5);
    }
    // agregar usuario//
    void addUser(std::string name_in, int socket_in, sockaddr_in address_in)
    { // creo un usuario con un nombre cualquiera y lo agrego a la lista de usuarios.
        // std::unique_lock<std::mutex> lck(mut_users);
        PACKAGE pkt;
        User *user = new User(name_in, socket_in, address_in, std::bind(&ChatServer::reader, this, std::placeholders::_1)); // TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A USUARIOS
        this->users.push_back(user);
        printf("se realizo una conexión\n");
    }

    void ChangeUserName(std::string name_in, User *user)
    { //
        // std::unique_lock<std::mutex> lck(mut_users);
        PACKAGE pkt;
        std::cout << "nombre recibido: " << name_in << std::endl;
        int flag = 0;
        int n;
        for (int i = 0; i < users.size(); i++)
        {
            if (users[i]->getName() == name_in)
            {
                std::cout << "el usuario ya existe" << std::endl;
                flag = 1;
                setCreateResponse(&pkt, DENIED_CREATE);
                n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
            }
        }
        if (flag == 0)
        {
            std::cout << "el usuario no existe" << std::endl;
            user->setName(name_in);
            setCreateResponse(&pkt, CORRECT_CREATE);
            n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
        }
    }
    // agregar sala//
    void addRoom(std::string name_in, User *owner_in)
    {
        // std::unique_lock<std::mutex> lck(mut_rooms);

        Room *room = new Room(name_in, owner_in); // TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A SALAS
        this->rooms.push_back(room);
    }

    void createRoom(std::string name_in, User *user)
    {
        // std::unique_lock<std::mutex> lck(mut_rooms);
        PACKAGE pkt;
        std::cout << "nombre de sala recibido: " << name_in << std::endl;
        int flag = 0;
        int n;
        for (int i = 0; i < rooms.size(); i++)
        {
            if (rooms[i]->getName() == name_in)
            {
                std::cout << "la sala ya existe" << std::endl;
                flag = 1;
                setCreateResponse(&pkt, DENIED_CREATE);
                n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
                return;
            }
        }
        if (flag == 0)
        {
            std::cout << "la sala no existe" << std::endl;
            addRoom(name_in, user);
            setCreateResponse(&pkt, CORRECT_CREATE);
            n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
        }
    }
    void connectUserToRoom(std::string name_in, User *user)
    {
        PACKAGE pkt;
        for (int i = 0; i < rooms.size(); i++)
        {
            if (rooms[i]->getName() == name_in)
            {
                for (int j = 0; j < rooms[i]->users.size(); j++)
                {
                    if (rooms[i]->users[j]->getName() == user->getName())
                    {
                        printf("el usuario ya esta en la sala\n");
                        setConnectResponse(&pkt, DENIED_CONNECT);
                        int n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
                        if (n < 0)
                            error("ERROR writing to socket");
                        return;
                    }
                }
                rooms[i]->addUser(user);
                printf("usuario agregado a la sala\n");
                rooms[i]->printUsers();
                setConnectResponse(&pkt, CORRECT_CONNECT);
                int n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
                if (n < 0)
                    error("ERROR writing to socket");
                return;
            }
        }
    }
    void QuitRoom(std::string name_in, User *user) // saca al usuario de la sala
    {
        PACKAGE pkt;

        user->deleteSala(name_in);
        for (int i = 0; i < rooms.size(); i++)
        {
            if (rooms[i]->getName() == name_in)
            {
                rooms[i]->deleteUser(user);
                if (rooms[i]->users.size() == 0) // si la sala queda vacia la elimino
                {
                    delete rooms[i];
                    rooms.erase(rooms.begin() + i);
                }
                printf("usuario eliminado de la sala\n");
                rooms[i]->printUsers();
                setExitResponse(&pkt, CORRECT_EXIT);
                int n = send(user->getSocket(), &pkt, sizeof(pkt), 0);
                if (n < 0)
                    error("ERROR writing to socket");
                return;
            }
        }
    }
    // del pkt extraigo src y dst, el mensaje y lo envio a destino.
    void sendMsgUser(PACKAGE *pkt)
    {
        std::string dst = getDstMensaje(pkt);
        std::string src = getSrcMensaje(pkt);
        std::string msg = getTxtMensaje(pkt);
        MODE_MENSAJE mode = getModeMensaje(pkt);
        std::cout << "src: " << src << std::endl;
        std::cout << "dst: " << dst << std::endl;
        std::cout << "mode: " << mode << std::endl;
        std::cout << "msg: " << msg << std::endl;
        for (int i = 0; i < users.size(); i++)
        {
            if (users[i]->getName() == dst)
            {
                int n = send(users[i]->getSocket(), pkt, sizeof(PACKAGE), 0);
                if (n < 0)
                    error("ERROR writing to socket");
            }

        }
    }
    void sendMsgRoom(PACKAGE *pkt, User *user)
    {
        std::string dst = getDstMensaje(pkt);
        std::string src = getSrcMensaje(pkt);
        std::string msg = getTxtMensaje(pkt);
        std::cout << "src: " << src << std::endl;
        std::cout << "dst: " << dst << std::endl;
        std::cout << "msg: " << msg << std::endl;
        if (rooms.size() == 0) // si no hay salas creadas le mando un mensaje al usuario que no hay salas creadas.
        {
            std::string serv = "Server";
            std::string no_existe = "no hay salas creadas";
            PACKAGE pkt_server;
            std::cout << "no hay salas creadas" << std::endl;
            setMENSAJE(&pkt_server, &serv[0], &src[0], &no_existe[0], no_existe.size());
            setModeMensaje(&pkt_server, MENSAJE_SERVER);
            int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
            if (n < 0)
                error("ERROR writing to socket");
            return;
        }
        for (int i = 0; i < rooms.size(); i++)
        {

            if (rooms[i]->getName() == dst)
            {
                // chequeo si el que mando el mensaje pertenece al room, sino le mando  un mensaje con src=server que diga que no puede mandar el mensaje.
                for (int j = 0; j < rooms[i]->users.size(); j++)
                {
                    if (rooms[i]->users[j]->getName() == src)
                    {
                        break;
                    }
                    else if (j == rooms[i]->users.size() - 1)
                    {
                        std::string serv = "Server";
                        std::string no_pertenece = "no pertenece al room";
                        PACKAGE pkt_server;
                        std::cout << "no pertenece al room" << std::endl;
                        setMENSAJE(&pkt_server, &serv[0], &src[0], &no_pertenece[0], no_pertenece.size());
                        setModeMensaje(&pkt_server, MENSAJE_SERVER);
                        int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
                        if (n < 0)
                            error("ERROR writing to socket");
                        return;
                    }
                }
                for (int j = 0; j < rooms[i]->users.size(); j++)
                {
                    if (rooms[i]->users[j]->getName() == src)
                    {
                        std::cout << "no envio el mensaje al que lo envio" << std::endl;
                    }
                    else
                    {
                        int n = send(rooms[i]->users[j]->getSocket(), pkt, sizeof(PACKAGE), 0);
                        if (n < 0)
                            error("ERROR writing to socket");
                    }
                }
                return;
            }
            else if (i == rooms.size() - 1)
            {
                std::string serv = "Server";
                std::string no_existe = "no existe el room ";
                PACKAGE pkt_server;
                std::cout << "no existe el room" << std::endl;
                setMENSAJE(&pkt_server, &serv[0], &src[0], &no_existe[0], no_existe.size());
                setModeMensaje(&pkt_server, MENSAJE_SERVER);
                int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
                if (n < 0)
                    error("ERROR writing to socket");
                return;
            }
        }
    }

    void sendListaUser(PACKAGE *pkt, User *user)
    {
        std::string lista;
        std::string dst = user->getName();
        lista += "lista de usuarios conectados: \n";
        for (int i = 0; i < users.size(); i++)
        {
            if (users[i]->getName() != user->getName())
            {
                lista += "-";
                lista += users[i]->getName();
                lista += "\n";
            }
        }
        std::cout << "lista: " << lista << std::endl;
        std::string serv = "Server";
        PACKAGE pkt_server;
        setListaResponse(&pkt_server, CORRECT_LISTA);
        int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
        if (n < 0)
            error("ERROR writing to socket");
        clearPacket(&pkt_server);
        setMENSAJE(&pkt_server, &serv[0], &dst[0], &lista[0], lista.size());
        setModeMensaje(&pkt_server, MENSAJE_SERVER);
        n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
        if (n < 0)
            error("ERROR writing to socket");
    }
    void sendListaRooms(PACKAGE *pkt, User *user)
    {
        std::string lista;
        std::string dst = user->getName();
        lista += "lista de salas creadas: \n";
        for (int i = 0; i < rooms.size(); i++)
        {
            lista += "-";
            lista += rooms[i]->getName();
            lista += "\n";
        }
        std::cout << "lista: " << lista << std::endl;
        std::string serv = "Server";
        PACKAGE pkt_server;
        setListaResponse(&pkt_server, CORRECT_LISTA);
        int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
        if (n < 0)
            error("ERROR writing to socket");
        clearPacket(&pkt_server);
        setMENSAJE(&pkt_server, &serv[0], &dst[0], &lista[0], lista.size());
        setModeMensaje(&pkt_server, MENSAJE_SERVER);
        n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
        if (n < 0)
            error("ERROR writing to socket");
    }
    void connection()
    { // hace accept y devuelve sockfd
        // acepto la conección
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(this->sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            error("ERROR on accept");
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), ip, INET_ADDRSTRLEN);
        std::cout << "conectado al ip " << ip << std::endl;
        std::cout << "conectado al puerto " << ntohs(cli_addr.sin_port) << std::endl;
        //------------------------------
        addUser("newUser1", newsockfd, cli_addr); // creo usuario con nombre predeterminado y lo agrego a la lista de usuarios.
        return;
    }

    void disconnectUser(PACKAGE *pkt, User *user)
    {
        PACKAGE pkt_server;
        std::string dst = getDstMensaje(pkt);
        std::string serv = "Server";
        std::string msg = "desconectado";
        setMENSAJE(&pkt_server, &serv[0], &dst[0], &msg[0], msg.size());
        setModeMensaje(&pkt_server, MENSAJE_SERVER);
        int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
        if (n < 0)
            error("ERROR writing to socket");

        for (int i = 0; i < user->salas.size(); i++)
        {
            QuitRoom(user->salas[i], user);
        }

        for (int i = 0; i < users.size(); i++)
        {
            if (users[i]->getName() == user->getName())
            {
                users.erase(users.begin() + i);
            }
        }

        delete user;
    }
    void modeRead(PACKAGE *pkt, User *user)
    {
        printf("size: %d\n", getSize8(&pkt->hdr));
        Type option = getType(&pkt->hdr);
        printf("type: %d\n", option);
        printf("version: %d\n", getVersion(&pkt->hdr));
        std::string name;
        std::string src;
        switch (option)
        {

        case TYPE_REQUEST_CREATE:
            printf("request create ");
            if (getModeCreateRequest(pkt) == REQUEST_USER_CREATE)
            {
                printf("user\n");
                strncpy(&name[0], getNameCreateRequest(pkt), MAX_NAME);
                printf("usuario: %s\n", name.c_str());
                ChangeUserName(name.c_str(), user); // cambia el nombre de usuario.
            }
            else if (getModeCreateRequest(pkt) == REQUEST_ROOM_CREATE)
            {
                printf("room\n");
                strncpy(&name[0], getNameCreateRequest(pkt), MAX_NAME);
                printf("creando sala: %s\n", name.c_str());
                createRoom(name.c_str(), user); // createRoom(name,socket,User)// crea una sala y la agrega a la lista de salas.
            }
            break;
        case TYPE_REQUEST_CONNECT:
            printf("request connect ");
            if (getModeConnectRequest(pkt) == REQUEST_USER_CONNECT)
            {
                printf("user\n");
                // IMPLEMENTAR ESTO *TO DO*
            }
            else if (getModeConnectRequest(pkt) == REQUEST_ROOM_CONNECT)
            {
                printf("room\n");
                // chequeo que la sala exista, si el usuario esta en la sala y si no esta en la sala ejecuto connectUserToRoom para agregarlo
                strncpy(&name[0], getNameConnectRequest(pkt), MAX_NAME);
                printf("conectando a sala: %s\n", name.c_str());
                connectUserToRoom(name.c_str(), user);
            }
            break;
        case TYPE_REQUEST_LISTA:
            printf("request lista ");
            if (getModeListaRequest(pkt) == REQUEST_LISTA_USER)
            {
                printf("user\n");
                sendListaUser(pkt, user);
            }
            else if (getModeListaRequest(pkt) == REQUEST_LISTA_ROOM)
            {
                printf("room\n");
                sendListaRooms(pkt, user);
            }
            break;
        case TYPE_REQUEST_EXIT:
            printf("request exit\n");
            strncpy(&name[0], getNameExitRequest(pkt), MAX_NAME);
            printf("saliendo de sala: %s\n", name.c_str());
            QuitRoom(name.c_str(), user);
            break;
        case TYPE_REQUEST_DISCONNECT:
            printf("request disconnect\n");
            disconnectUser(pkt, user);
            break;
        case TYPE_MENSAJE:
            if (getModeMensaje(pkt) == MENSAJE_USER)
            {
                printf("mensaje a usuario\n");
                sendMsgUser(pkt);
            }
            else if (getModeMensaje(pkt) == MENSAJE_ROOM)
            {
                printf("mensaje a salas\n");
                sendMsgRoom(pkt, user);
            }
            break;
        default:
            printf("no hice nada\n");
            break;
        }
    }
    void reader(User *user)
    {
        int n;
        PACKAGE pkt;

        while (n = read(user->getSocket(), &pkt, sizeof(pkt)))
        {
            modeRead(&pkt, user);
            if (n < 0)
                error("ERROR reading from socket");
        }
        printf("se desconecto un thread\n");
    }
    void work()
    {
        while (1)
        {
            connection();
        }
    }
};

int main()
{

    ChatServer server;
    server.work();
    return 0;
}