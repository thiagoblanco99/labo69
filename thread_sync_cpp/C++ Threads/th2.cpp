#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

mutex mtx;
int val = 0;

void fincr(int n) {
    for( int i = 0; i < n; ++i ) {
        // unique_lock<mutex> lck(mtx);
        lock_guard<mutex> lck(mtx);
        val += 1;
    }
}

void fdecr(int n) {
    try {
        for( int i = 0; i < n; ++i ) {
            lock_guard<mutex> lck(mtx);
            if( i == 100 )
                throw runtime_error("except");
            val -= 1;
            // si se dispara una excepcion antes del unlock
        }
    } catch( exception &e ) {
        cout << "agarre excepcion: " << e.what() << endl;
    }
}

int main()
{
    vector<thread> v;

    for( int i = 0; i < 50; ++i ) {
        v.emplace_back(fincr, 10000);//pone un thread mas en el vector de threads y lo inicializa con 10000, osea fincr(10000)
        v.emplace_back(fdecr, 10000);
    }

    for( auto &t : v )
        t.join();

    cout << "val: " << val << endl;

    return 0;
}
