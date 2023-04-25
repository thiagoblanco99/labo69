#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

void f() {                 // function
    string s = "Hello ";
    for( auto c : s ) {
        cout << c;
        this_thread::sleep_for(chrono::microseconds{1});
    }
}

struct F {                 // function object
    void operator()() {
        string s = "Parallel world!\n";
        for( auto c : s ) {
            cout << c;
            this_thread::sleep_for(chrono::microseconds{1});
        }
    }
};

int main()
{
    std::thread t1{f};     // se ejecuta f() en un thread separado
    std::thread t2{F{}};   // se ejecuta F()() en un thread separado 

    t1.join();             // wait for t1
    t2.join();             // wait for t2

    cout<<"chau\n";
    return 0;
}
