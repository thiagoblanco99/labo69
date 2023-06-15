
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

#include "protocolo.h"

#define PORT 5001

void error(const char *msg)
{
    perror(msg);
    exit(1);
}



class User{
    private:
    public:
    //variables a usar//
    std::vector<std::string> salas;
    std::string name;
    int socket;
    sockaddr_in address;
    int mode;
    std::thread t_user;
    //----------------------------
    //mutex a usar
    std::mutex mut_salas;
    std::mutex mut_name;
    std::mutex mut_socket;
    std::mutex mut_address;
    std::mutex mut_mode;
    std::mutex mut_thread;
    //------------------------
    //constructror//
    User(std::string name_in, int socket_in, struct sockaddr_in address_in){
        this->name = name_in;
        this->socket = socket_in;
        this->address = address_in;
    }
    //getters//
    std::string getName(){
        std::unique_lock<std::mutex> lck(mut_name);
        return this->name;
    }
    int getSocket(){
        std::unique_lock<std::mutex> lck(mut_socket);
        return this->socket;
    }
    int getip(){//obtengo ip del usuario
        std::unique_lock<std::mutex> lck(mut_address);
        return this->address.sin_addr.s_addr;
    }
    //agregar salas//
    void addSala(std::string name){
        std::unique_lock<std::mutex> lck(mut_salas);
        this->salas.push_back(name);// completar
    }
    //borrar sala//
    void deleteSala(std::string name){
        std::unique_lock<std::mutex> lck(mut_salas);
        for(int i=0;i<salas.size();i++){
            if(salas[i] == name){
                salas.erase(salas.begin()+i);
            }
        }
    }
    //imprimir salas//
    void printSalas(){
        std::unique_lock<std::mutex> lck(mut_salas);
        for(int i=0;i<salas.size();i++){
            std::cout<<salas[i]<<std::endl;
        }
    }
    
};

class Room{
    private:
    public:
    //variables a usar//
    std::string name;
    int id;
    std::vector<User*> users;
    User* owner;
    //------------------------
    //mutex a usar
    std::mutex mut_name;
    std::mutex mut_id;
    std::mutex mut_users;
    std::mutex mut_owner;
    //------------------------
    //constructor//
    Room(std::string name_in, int id_in, User* owner_in){
        this->name = name_in;
        this->id = id_in;
        this->users.push_back(owner_in);//lo agrego a los usuarios de la sala al dueño.
        this->owner = owner_in; // lo pongo como dueño.
        this->owner->addSala(this->name);
    }
    //getters//
    std::string getName(){
        std::unique_lock<std::mutex> lck(mut_name);
        return this->name;
    }
    int getId(){
        std::unique_lock<std::mutex> lck(mut_id);
        return this->id;
    }
    std::string getOwner(){
        std::unique_lock<std::mutex> lck(mut_owner);
        return this->owner->getName();
    }
    //agregar usuarios//
    void addUser(User* user){
        std::unique_lock<std::mutex> lck(mut_users);
        this->users.push_back(user);
        user->addSala(this->name);
    }
    //imprimir usuarios//
    void printUsers(){
        std::unique_lock<std::mutex> lck(mut_users);
        for(int i=0;i<users.size();i++){
            std::cout<<users[i]->getName()<<std::endl;
        }
    }
    //eliminar usuarios//
    void deleteUser(User* user){
        std::unique_lock<std::mutex> lck(mut_users);
        for(int i=0;i<users.size();i++){
            if(this->users[i]->getName() == user->getName()){
                this->users.erase(this->users.begin()+i);
            }
        }
    }
};

class ChatServer{
    private:
    public:
    std::vector<User*> users;//TENGO QUE HACER TODAS LAS FUNCIONES CON ESTOS PUNTEROS.
    std::vector<Room*> rooms;//TENGO QUE HACER TODAS LAS FUNCIONES CON ESTOS PUNTEROS.
    struct sockaddr_in serv_addr;
    int  sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //mutex a usar
    std::mutex mut_users;
    std::mutex mut_rooms;
    //constructor//
    ChatServer(){
        if (this->sockfd < 0) 
        {
            error("ERROR opening socket");
        }
        int val=1;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        this->serv_addr.sin_family = AF_INET;
        this->serv_addr.sin_addr.s_addr = INADDR_ANY; //ip de la maquina
        this->serv_addr.sin_port = htons(PORT);
        setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        if (bind(this->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        {
            error("ERROR on binding");
        }
        listen(this->sockfd,5);
    }
    //agregar usuario//
    void addUser(std::string name_in, int socket_in, sockaddr_in address_in){
        std::unique_lock<std::mutex> lck(mut_users);

        User* user = new User(name_in, socket_in, address_in); //TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A USUARIOS
        this->users.push_back(user);
    }
    //borrar usuario//
    void deleteUser(User* user){
        std::unique_lock<std::mutex> lck(mut_users);

        for(int i=0;i<user->salas.size();i++){
            for(int j=0;j<rooms.size();j++){
                if(user->salas[i] == rooms[j]->getName()){
                    rooms[j]->deleteUser(user);
                }
            }
        }
        for(int i=0;i<users.size();i++){
            if(users[i]->getName() == user->getName()){ //TENGO QUE BORRAR EL PUNTERO A USER Y HACER DELETE
                users.erase(users.begin()+i);
            }
        }
    }
    //imprimir usuarios//
    void printUsers(){
        std::unique_lock<std::mutex> lck(mut_users);

        for(int i=0;i<users.size();i++){
            std::cout<<users[i]->getName()<<std::endl;
        }
    }
    void addRoom(std::string name_in, int id_in, User* owner_in){
        std::unique_lock<std::mutex> lck(mut_rooms);

        Room* room = new Room(name_in, id_in, owner_in); // TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A SALAS
        this->rooms.push_back(room);
    }
    //imprimir salas//
    void printRooms(){
        std::unique_lock<std::mutex> lck(mut_rooms);

        for(int i=0;i<rooms.size();i++){
            std::cout<<rooms[i]->getName()<<std::endl;
        }
    }

    int connection(struct sockaddr_in* cli_addr){//hace accept y devuelve sockfd
        //acepto la conección
        socklen_t clilen = sizeof(*cli_addr);
        int newsockfd = accept(this->sockfd, (struct sockaddr *) cli_addr, &clilen);
        if (newsockfd < 0) 
        {
            error("ERROR on accept");
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr->sin_addr), ip, INET_ADDRSTRLEN);
        std::cout<<"conectado al ip "<< ip <<std::endl;
        std::cout<<"conectado al puerto "<<ntohs(cli_addr->sin_port)<<std::endl;
        //------------------------------
        return newsockfd;

    }
    void mode(PACKAGE* pkt){
        printf("size: %d\n",getSize8(&pkt->hdr));
        Type option= getType(&pkt->hdr);
        printf("type: %d\n",option);
        printf("version: %d\n",getVersion(&pkt->hdr));
        switch(option){

            case TYPE_REQUEST_CREATE:
                printf("request create ");
                if(getModeCreateRequest(pkt) == REQUEST_USER_CREATE){
                    printf("user\n");
                }
                else if(getModeCreateRequest(pkt) == REQUEST_ROOM_CREATE){
                    printf("room\n");
                }
                break;
            case TYPE_REQUEST_CONNECT:
                printf("request connect ");
                if(getModeConnectRequest(pkt) == REQUEST_USER_CONNECT){
                    printf("user\n");
                }
                else if(getModeConnectRequest(pkt) == REQUEST_ROOM_CONNECT){
                    printf("room\n");
                }
                break;
            case TYPE_REQUEST_LISTA:
                printf("request lista");
                if(getModeListaRequest(pkt) == REQUEST_LISTA_USER){
                    printf("user\n");
                }
                else if(getModeListaRequest(pkt) == REQUEST_LISTA_ROOM){
                    printf("room\n");
                }
                break;
            case TYPE_REQUEST_EXIT:
                printf("request exit");
                break;
            case TYPE_REQUEST_DISCONNECT:
                printf("request disconnect\n");
                break;
            case TYPE_MENSAJE:
                printf("mensaje\n");
                break;
            default:
                printf("error\n");
                break;
        }
    }
    void reader(int sockfd){
        int n;
        PACKAGE pkt;
        char usuarios[MAX_NAME];
        char buffer[20];
        bzero(buffer,20);

        while(n = read(sockfd,&pkt,sizeof(pkt))){
        mode(&pkt);
        strncpy(usuarios,getSrcMensaje(&pkt),MAX_NAME);
        printf("src: %s\n",usuarios);
        strncpy(usuarios,getDstMensaje(&pkt),MAX_NAME);
        printf("dst: %s\n",usuarios);
        uint16_t msglen;
        msglen = getLenMensaje(&pkt);
        printf("len: %d\n",msglen);
        //falta leer el
        strcpy(buffer,getTxtMensaje(&pkt));
        printf("txt: %s\n",buffer);

        if (n < 0) error("ERROR reading from socket");
        n = write(sockfd,"I got your message",18);
        if (n < 0) error("ERROR writing to socket");
        }
        printf("salio del while\n");
    }
    void listener (){
        struct sockaddr_in cli_addr;
        int newsockfd=connection(&cli_addr);
        std::thread t1(&ChatServer::reader,this,newsockfd);
        newsockfd=connection(&cli_addr);
        std::thread t2(&ChatServer::reader,this,newsockfd);
        t1.join();
        t2.join();
    }

};

int main(){

    ChatServer server;
    server.listener();
    return 0;
}