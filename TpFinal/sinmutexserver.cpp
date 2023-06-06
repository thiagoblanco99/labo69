
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
    //------------------------
    //constructror//
    User(std::string name_in, int socket_in, struct sockaddr_in address_in){
        this->name = name_in;
        this->socket = socket_in;
        this->address = address_in;
    }
    //getters//
    std::string getName(){
        return this->name;
    }
    int getSocket(){
        return this->socket;
    }
    int getip(){//obtengo ip del usuario
        return this->address.sin_addr.s_addr;
    }
    //agregar salas//
    void addSala(std::string name){
        this->salas.push_back(name);// completar
    }
    //borrar sala//
    void deleteSala(std::string name){
        for(int i=0;i<salas.size();i++){
            if(salas[i] == name){
                salas.erase(salas.begin()+i);
            }
        }
    }
    //imprimir salas//
    void printSalas(){
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
        return this->name;
    }
    int getId(){
        return this->id;
    }
    std::string getOwner(){
        return this->owner->getName();
    }
    //agregar usuarios//
    void addUser(User* user){
        this->users.push_back(user);
        user->addSala(this->name);
    }
    //imprimir usuarios//
    void printUsers(){
        for(int i=0;i<users.size();i++){
            std::cout<<users[i]->getName()<<std::endl;
        }
    }
    //eliminar usuarios//
    void deleteUser(User* user){
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

        User* user = new User(name_in, socket_in, address_in); //TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A USUARIOS
        this->users.push_back(user);
    }
    //borrar usuario//
    void deleteUser(User* user){
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
        
        for(int i=0;i<users.size();i++){
            std::cout<<users[i]->getName()<<std::endl;
        }
    }
    void addRoom(std::string name_in, int id_in, User* owner_in){
        Room* room = new Room(name_in, id_in, owner_in); // TENGO QUE ALOCAR MEMORIA PARA LOS PUNTEROS A SALAS
        this->rooms.push_back(room);
    }
    //imprimir salas//
    void printRooms(){
        for(int i=0;i<rooms.size();i++){
            std::cout<<rooms[i]->getName()<<std::endl;
        }
    }

    int connection(struct sockaddr_in* cli_addr){//hace accept y devuelve sockfd
        
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
        return newsockfd;

    }

    void listener(){
    } 
};

int main(){

    ChatServer server;
    struct sockaddr_in cli_addr;
    while(1){
    int newsockfd=0;
    newsockfd=server.connection(&cli_addr);
    if (newsockfd < 0) 
    {
        error("ERROR on accept");
        break;
    }
    if(newsockfd > 0){
        std::cout<<"conectado"<<std::endl;
    }
    }
    return 0;
}