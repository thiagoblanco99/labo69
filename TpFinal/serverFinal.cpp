
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
#include <condition_variable>

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
    std::thread t_user;
    //----------------------------
    // mutex a usar
    std::mutex mut_user;
    //------------------------
    // constructror//
    User(std::string name_in, int socket_in, struct sockaddr_in address_in, std::function<void(User *)> reader_in)
    {
        this->name = name_in;
        this->socket = socket_in;
        this->address = address_in;
        this->t_user = std::thread(reader_in, this);
    }
    // destructor//
    ~User()
    {
        printf("se borra el usuario\n");
        this->t_user.join();
        close(this->socket);
    }
    // getters//
    void setName(std::string name_in)
    {

        this->name = name_in;
    }
    std::string getName()
    {
        return this->name;
    }
    int getSocket()
    {
        return this->socket;
    }
    int getip()
    { // obtengo ip del usuario
        return this->address.sin_addr.s_addr;
    }
    // agregar salas//
    void addSala(std::string name)
    {
        this->salas.push_back(name); // completar
    }
    // borrar sala//
    void deleteSala(std::string name)
    {
        for (int i = 0; i < salas.size(); i++)
        {
            if (salas[i] == name)
            {
                salas.erase(salas.begin() + i);
                return;
            }
        }
    }
    // imprimir salas//
    void printSalas()
    {
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
    std::vector<User *> users;
    User *owner;
    //------------------------
    // mutex a usar
    std::mutex mut_room;
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
        return this->name;
    }
    std::string getOwner()
    {
        return this->owner->getName();
    }
    // agregar usuarios//
    void addUser(User *user)
    {
        this->users.push_back(user);
        user->addSala(this->name);
    }
    // imprimir usuarios//
    void printUsers()
    {
        for (int i = 0; i < users.size(); i++)
        {
            std::cout << users[i]->getName() << std::endl;
        }
    }
    // eliminar usuarios//
    void deleteUser(User *user)
    {
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
    std::vector<Room *> rooms; // TENGO QUE HACER TODAS LAS FUNCIONES CON ESTOS PUNTEROS.std::vector<User *> disconnections;//todo lo que meta en este vector es para desconectar
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // VARIABLES PARA HACER UN QUEUE DE THREADS A JOINEAR
    std::vector<User *> disconnections;
    unsigned int maxq = 10; // numero maximo de elementos en la cola
    std::condition_variable putcv;
    std::condition_variable getcv;
    unsigned int putter = 0;
    unsigned int getter = 0;
    std::thread joiner;
    bool isRunning = true;
    //---------------------------------------------------
    // mutex a usar
    std::mutex mut_users;
    std::mutex mut_rooms;
    std::mutex mut_disconnections;
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
        // habilito el thread joiner
        joiner = std::thread(&ChatServer::joinerWork, this);
    }
    // hago destructor
    ~ChatServer()
    {
        isRunning = false;
        joiner.join();
        close(this->sockfd);
    }
    // agregar usuario//
    void addUser(std::string name_in, int socket_in, sockaddr_in address_in)
    { // creo un usuario con un nombre cualquiera y lo agrego a la lista de usuarios.
        PACKAGE pkt;
        User *user = new User(name_in, socket_in, address_in, std::bind(&ChatServer::reader, this, std::placeholders::_1)); // TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A USUARIOS
        this->users.push_back(user);
        printf("se realizo una conexión\n");
    }

    void ChangeUserName(std::string name_in, User *user)
    { //
        std::unique_lock<std::mutex> lck_user(user->mut_user, std::defer_lock);
        std::unique_lock<std::mutex> lck_users(mut_users, std::defer_lock);
        std::lock(lck_user, lck_users);
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
        Room *room = new Room(name_in, owner_in); // TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A SALAS
        this->rooms.push_back(room);
    }

    void createRoom(std::string name_in, User *user)
    {
        std::unique_lock<std::mutex> lck_rooms(mut_rooms, std::defer_lock);
        std::unique_lock<std::mutex> lck_user(user->mut_user, std::defer_lock);
        std::lock(lck_rooms, lck_user);
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
        std::unique_lock<std::mutex> lck_rooms(mut_rooms, std::defer_lock);
        std::unique_lock<std::mutex> lck_user(user->mut_user, std::defer_lock);
        std::lock(lck_rooms, lck_user);
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
        std::unique_lock<std::mutex> lck_rooms(mut_rooms, std::defer_lock);
        std::unique_lock<std::mutex> lck_user(user->mut_user, std::defer_lock);
        std::lock(lck_rooms, lck_user);

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
    void sendMsgUser(PACKAGE *pkt, User *user)
    {
        std::unique_lock<std::mutex> lck(mut_users);
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
                std::unique_lock<std::mutex> lck(users[i]->mut_user);
                int n = send(users[i]->getSocket(), pkt, sizeof(PACKAGE), 0);
                if (n < 0)
                    error("ERROR writing to socket");
                return;
            }
            else if (i == users.size() - 1)
            {
                std::string serv = "Server";
                std::string no_existe = "no existe el usuario ";
                PACKAGE pkt_server;
                std::cout << "no existe el usuario" << std::endl;
                setMENSAJE(&pkt_server, &serv[0], &src[0], &no_existe[0], no_existe.size());
                setModeMensaje(&pkt_server, MENSAJE_SERVER);
                std::unique_lock<std::mutex> lck(user->mut_user);
                int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0); // esta linea hay que corregirla
                if (n < 0)
                    error("ERROR writing to socket");
            }
        }
    }
    void sendMsgRoom(PACKAGE *pkt, User *user)
    {
        std::unique_lock<std::mutex> lck(mut_rooms);

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
            std::unique_lock<std::mutex> lck(user->mut_user);
            int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
            if (n < 0)
                error("ERROR writing to socket");
            return;
        }
        for (int i = 0; i < rooms.size(); i++)
        {
            std::unique_lock<std::mutex> lck(rooms[i]->mut_room);
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
                        std::unique_lock<std::mutex> lck(user->mut_user);
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
                        std::unique_lock<std::mutex> lck(users[j]->mut_user);
                        int n = send(rooms[i]->users[j]->getSocket(), pkt, sizeof(PACKAGE), 0);
                        if (n < 0)
                            error("ERROR writing to socket");
                        users[j]->mut_user.unlock();
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
            rooms[i]->mut_room.unlock();
        }
    }

    void sendListaUser(PACKAGE *pkt, User *user)
    {
        std::unique_lock<std::mutex> lck_users(mut_users, std::defer_lock);
        std::unique_lock<std::mutex> lck_user(user->mut_user, std::defer_lock);
        std::lock(lck_users, lck_user);
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
        std::unique_lock<std::mutex> lck_rooms(mut_rooms, std::defer_lock);
        std::unique_lock<std::mutex> lck_user(user->mut_user, std::defer_lock);
        std::lock(lck_rooms, lck_user);

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

    bool disconnectUser(User *user)
    {
        std::unique_lock<std::mutex> lck(user->mut_user);
        PACKAGE pkt_server;
        std::string dst = user->getName();
        std::string serv = "Server";
        std::string msg = "desconectado...";
        setMENSAJE(&pkt_server, &serv[0], &dst[0], &msg[0], msg.size() - 1); // si saco el uno se printean unos caracteres random y no se porque.
        setModeMensaje(&pkt_server, MENSAJE_SERVER);
        int n = send(user->getSocket(), &pkt_server, sizeof(PACKAGE), 0);
        if (n < 0)
            error("ERROR writing to socket");
        put(user);    // en fila para desconectar al usuario
        return false; // aca seguro tenga que poner algo que avise que este thread no escuche más y haga que termine el proceso.
    }
    bool modeRead(PACKAGE *pkt, User *user)
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
            return false;
        case TYPE_MENSAJE:
            if (getModeMensaje(pkt) == MENSAJE_USER)
            {
                printf("mensaje a usuario\n");
                sendMsgUser(pkt, user);
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
        return true;
    }
    void reader(User *user)
    {
        int n;
        bool vivo = true;
        PACKAGE pkt;

        while (n = read(user->getSocket(), &pkt, sizeof(pkt)))
        {
            vivo = modeRead(&pkt, user);
            if (!vivo)
                break;
            if (n < 0)
                error("ERROR reading from socket");
        }
        printf("se desconecto un thread\n");
        disconnectUser(user); // en fila para desconectar al usuario
    }
    void work()
    {

        while (1)
        {
            connection();
        }
    }
    // implemento algoritmos de queue para hacer los join y delete de los threads de usuarios.
    void put(User *user)
    {
        printf("antes del mutex\n");
        std::unique_lock<std::mutex> lck(mut_disconnections);
        while (getter % maxq == (putter + 1) % maxq)
        {
            printf("despues del mutex 1\n");
            putcv.wait(lck);
        }
        disconnections.push_back(user);
        printf("pongo tarea\n");
        putter++;
        getcv.notify_one();
    }
    void get()
    {
        std::unique_lock<std::mutex> lck_discon(mut_disconnections);
        while (getter % maxq == putter % maxq)
        {
            getcv.wait(lck_discon);
        }
        std::unique_lock<std::mutex> lck_users(mut_users);
        // tengo que borrar el usuario de las salas que haya

        printf("tomo tarea\n");
        for (int i = 0; i < disconnections[getter % maxq]->salas.size(); i++)
        {
            QuitRoom(disconnections[getter % maxq]->salas[i], disconnections[getter % maxq]);
        }

        for (int i = 0; i < users.size(); i++)
        {
            if (users[i]->getName() == disconnections[getter % maxq]->getName())
            {
                users.erase(users.begin() + i);
            }
        }
        delete disconnections[getter % maxq];
        printf("borre el cliente\n");
        getter++;
        putcv.notify_one();
    }

    void joinerWork()
    {
        while (isRunning)
        {
            printf("joiner is running");
            get();
        }
    }
};

int main()
{

    ChatServer server;
    server.work();
    return 0;
}

// CUANDO UN USUARIO ESTA CONECTADO A UNA SALA Y SALE DE LA SALA, 
//SE ROMPE CUANDO EL SERVIDOR INTENTA MANDAR UN MENSAJE AL SOCKET QUE NO EXISTE.