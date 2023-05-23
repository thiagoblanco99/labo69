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

    public:
    // variables que voy a usar    //
        std::vector<int> salas;
        std::string name;
        int socket;
        int id;
    //-----------------------------//
        User(std::string name_in, int socket_in, int id_in){
            name = name_in;
            socket = socket_in;
            id = id_in;
        }
        std::string getName(){
            return name;
        }
        int getSocket(){
            return socket;
        }
        int getId(){
            return id;
        }
};

class Room{
    private:

    public:
    // variables que voy a usar    //
        std::string name;
        int id;
        std::vector<User*> users;
        User* owner;
    //-----------------------------//
        Room(std::string name_in, int id_in, User* owner_in){
            this->name = name_in;
            this->id = id_in;
            this->users.push_back(owner_in);
            this->owner = owner_in;
        }
        ~Room();//tengo que hacer un destructor
        std::string getName(){
            return this->name;
        }
        int getId(){
            return this->id;
        }
        std::vector<User*> getUsers(){
            return this->users;
        }
        void addUser(User* user){
            this->users.push_back(user); // agrego usuario a la lista de la sala  
            user->salas.push_back(this->id);//agrego esta sala a la lista del usuario 
        }
        void removeUser(User* user){
            for(int i=0;i<this->users.size();i++){
                if(this->users[i]->getId() == user->getId()){
                    this->users.erase(this->users.begin()+i);
                }
            }
        }
};

class C2C{
    private:

    public:
    // variables que voy a usar    //
        User* user1;
        User* user2;
        int id;
    //-----------------------------//
        C2C(User* userA, User* userB){
            this->user1 = userA;
            this->user2 = userB;
        }

        ~C2C();//tengo que hacer un destructor


        User* getUser1(){
            return user1;
        }
        User* getUser2(){
            return user2;
        }
        int getId(){
            return id;
        }
};

class ChatServer{
    private:
        std::vector<User> users;
        std::vector<Room> rooms;
        std::vector<C2C> c2cs;
    public:
        ChatServer(){};
        ~ChatServer(){};//tengo que hacer un destructor
        
        void addUser(std::string name_in, int socket_in, int id_in){
            this->users.push_back(User(name_in, socket_in, id_in));//creo un usuario y lo agrego al vector
        }
        
        void removeUser(User user){

            for (int i=0;i<rooms.size();i++){//tengo que hacer que se borre de las salas en las que este suscripto
                if(rooms[i].id == user.salas[i]){
                    rooms[i].removeUser(&user);
                }
            }

            for(int i=0;i<this->users.size();i++){
                if(this->users[i].getId() == user.getId()){
                    this->users.erase(this->users.begin()+i);//busco usuario y lo elimino del vector
                }
            }   
        }
        void addRoom(std::string name_in, int id_in, User* owner_in){
            this->rooms.push_back(Room (name_in, id_in, owner_in));
        }

        void removeRoom(Room room_in){
            for(int i=0;i<this->rooms.size();i++){
                if(this->rooms[i].getId() == room_in.getId()){
                    this->rooms.erase(this->rooms.begin()+i);
                }
            }
        }
        void addC2C(User* userA, User* userB){
            this->c2cs.push_back(C2C(userA, userB));
        }
        void removeC2C(C2C c2c){
            for(int i=0;i<this->c2cs.size();i++){
                if(this->c2cs[i].getId() == c2c.getId()){
                    this->c2cs.erase(this->c2cs.begin()+i);
                }
            }
        }
        std::vector<User> getUsers(){
            return this->users;
        }
        std::vector<Room> getRooms(){
            return this->rooms;
        }
        std::vector<C2C> getC2Cs(){
            return this->c2cs;
        }


};
