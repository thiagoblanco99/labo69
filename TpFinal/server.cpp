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
        std::vector<std::string> salas;
        std::vector<int> c2c;
        std::string name;
        int socket;
        int id;
        int mode;
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
        void printSalas(){
            for(int i=0;i<salas.size();i++){
                std::cout<<salas[i]<<std::endl;
            }
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
            this->users.push_back(owner_in);//lo agrego a los usuarios de la sala al dueño.
            this->owner = owner_in; // lo pongo como dueño.
            owner_in->salas.push_back(this->getName());// le agrego el id a la lista de salas del dueño.
        }
        //~Room();//tengo que hacer un destructor
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
            user->salas.push_back(this->getName());//agrego esta sala a la lista del usuario 
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
        C2C(User* userA, User* userB, int id_in){
            this->user1 = userA;
            this->user2 = userB;
            this->id = id_in;
            userA->c2c.push_back(this->id);
            userB->c2c.push_back(this->id);
        }

        //~C2C();//tengo que hacer un destructor


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
        //~ChatServer(){};//tengo que hacer un destructor
        
        void addUser(std::string name_in, int socket_in, int id_in){
            this->users.push_back(User(name_in, socket_in, id_in));//creo un usuario y lo agrego al vector
        }
        
        void removeUser(User user){

            /*for (int i=0;i<rooms.size();i++){//tengo que hacer que se borre de las salas en las que este suscripto
                if(rooms[i].name == user.salas[i]){
                    rooms[i].removeUser(&user);
                    break;
                }
            }*/

            for(int i=0; c2cs.size();i++){ //tengo que borrar las conexiones c2c que haya
                if(c2cs[i].getId()==user.c2c[i]){
                    removeC2C(c2cs[i]);
                    break;
                }
            }

            for(int i=0;i<this->users.size();i++){
                if(this->users[i].getId() == user.getId()){
                    this->users.erase(this->users.begin()+i);//busco usuario y lo elimino del vector de usuarios
                    break;
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
        void addC2C(User* userA, User* userB,int id_in){
            this->c2cs.push_back(C2C(userA, userB, id_in));
        }
        void removeC2C(C2C c2c){
            for(int i=0;i<this->c2cs.size();i++){
                if(this->c2cs[i].getId() == c2c.getId()){
                    this->c2cs.erase(this->c2cs.begin()+i);
                }
            }
        }
        void addUser2Room(User* user, int room){
            this->rooms[room].addUser(user);//crear funcion que me diga la posicion de la sala en el vector
            std::cout<<"agregue un usuario a la sala: "<< rooms[room].getName()<<std::endl;
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

int main (){

ChatServer server;

server.addUser("pepe", 1, 1);
server.addUser("juan", 2, 2);
server.addUser("luis", 3, 3);
server.addUser("maria", 4, 4);
server.addUser("jose", 5, 5);

server.addRoom("sala1", 1, &server.getUsers()[0]);
server.addRoom("sala2", 2, &server.getUsers()[1]);

server.addUser2Room(&server.getUsers()[1], 1);
server.addUser2Room(&server.getUsers()[2], 1);
server.addUser2Room(&server.getUsers()[3], 1);


server.addC2C(&server.getUsers()[2], &server.getUsers()[3], 1);
server.addC2C(&server.getUsers()[4], &server.getUsers()[0], 2);

std::cout<<"usuarios: "<<std::endl; 
for(int i=0;i<server.getUsers().size();i++){
    std::cout<<server.getUsers()[i].getName()<<std::endl;
}

std::cout<<"salas: "<<std::endl;
for(int i=0;i<server.getRooms().size();i++){
    std::cout<<server.getRooms()[i].getName();
    std::cout<<" usuarios: "<<std::endl;
    for(int j=0;j<server.getRooms()[i].getUsers().size();j++){
        std::cout<<server.getRooms()[i].getUsers()[j]->getName()<<std::endl;
    }
}

std::cout<<"c2cs: "<<std::endl;
for(int i=0;i<server.getC2Cs().size();i++){
    std::cout<<server.getC2Cs()[i].getUser1()->getName()<<" "<<server.getC2Cs()[i].getUser2()->getName()<<std::endl;
}

std::cout<<"maria esta en las salas: "<<std::endl;
for(int i=0;i<server.getUsers()[3].salas.size();i++){
    server.getUsers()[3].printSalas();// esto no funciona y no se porque :(((((((
}


std::cout<<"el dueño de la sala : "<<server.getRooms()[0].getName()<<std::endl;
std::cout<<"Es "<<server.getRooms()[0].owner->getName()<<std::endl;

std::cout<<"el dueño de la sala : "<<server.getRooms()[1].getName()<<std::endl;
std::cout<<"Es "<<server.getRooms()[1].owner->getName()<<std::endl;













    return 0;
}

