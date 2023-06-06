#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <time.h>
#include <mutex>
#include <semaphore.h>
#include <vector>

class User{
    private:
    std::mutex m_sala_user;
    std::mutex m_mode_user;
    public:
    //variables a usar//
    std::vector<std::string> salas;
    std::string name;
    int socket;
    int id;
    int mode;
    //------------------------
    //constructror//
    User(std::string name_in, int socket_in, int id_in){
        this->name = name_in;
        this->socket = socket_in;
        this->id = id_in;
    }
    //getters//
    std::string getName(){
        return this->name;
    }
    int getSocket(){
        return this->socket;
    }
    int getId(){
        return this->id;
    }
    //agregar salas//
    void addSala(std::string name){
        std::unique_lock<std::mutex> lock(m_sala_user);
        this->salas.push_back(name);// completar
    }
    //imprimir salas//
    void printSalas(){
        std::unique_lock<std::mutex> lock(m_sala_user);
        for(int i=0;i<salas.size();i++){
            std::cout<<salas[i]<<std::endl;
        }
    }
    
};

class Room{
    private:
    std::mutex m_users_room;
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
        std::unique_lock<std::mutex> lock(m_users_room);
        this->users.push_back(user);
    }
    //imprimir usuarios//
    void printUsers(){
        std::unique_lock<std::mutex> lock(m_users_room);
        for(int i=0;i<users.size();i++){
            std::cout<<users[i]->getName()<<std::endl;
        }
    }
    //eliminar usuarios//
    void deleteUser(User* user){
        std::unique_lock<std::mutex> lock(m_users_room);
        for(int i=0;i<users.size();i++){
            if(users[i]->getName() == user->getName()){
                users.erase(users.begin()+i);
            }
        }
    }
};

class ChatServer{
    private:
    std::vector<User> users;
    std::vector<Room> rooms;
    std::mutex m_users_server;
    std::mutex m_rooms_server;
    public:
    ChatServer(){
        this->users = {};
        this->rooms = {};
    }
    //agregar usuario//
    void addUser(std::string name_in, int socket_in, int id_in){
        std::unique_lock<std::mutex> lock(m_users_server);
        User user(name_in,socket_in,id_in);
        this->users.push_back(user);
    }
    //imprimir usuarios//
    void printUsers(){
        std::unique_lock<std::mutex> lock(m_users_server);
        for(int i=0;i<users.size();i++){
            std::cout<<users[i].getName()<<std::endl;
        }
    }
    void addRoom(std::string name_in, int id_in, User* owner_in){
        std::unique_lock<std::mutex> lock(m_rooms_server);
        Room room(name_in,id_in,owner_in);
        this->rooms.push_back(room);
    }
    void printRooms(){
        std::unique_lock<std::mutex> lock(m_rooms_server);
        for(int i=0;i<rooms.size();i++){
            std::cout<<rooms[i].getName()<<std::endl;
        }
    }
};

int main(){

    ChatServer server;

    server.addUser("pepe",1,1);
    server.addUser("juan",2,2);
    server.addUser("luis",3,3);
    server.addUser("maria",4,4);
    server.addUser("jose",5,5);
    server.addUser("pedro",6,6);

    server.printUsers();




}