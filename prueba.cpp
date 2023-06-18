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


int main(){
std::vector<std::string> lista;
lista.push_back("hola");
lista.push_back("hola2");
lista.push_back("hola3");
lista.erase(lista.begin());

std::cout<<"esto es size de lista "<<lista.size()<<std::endl;
return 0;
}