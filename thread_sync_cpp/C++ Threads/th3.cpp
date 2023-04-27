#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <vector>
#include <stdexcept>
#include <deque>
#include <condition_variable>

using namespace std;

mutex printMtx;

struct Msg {
    int pthid;
    int id;
    // ...
};

class SyncQueue {
    public:
        void put(const Msg &m) {
            unique_lock<mutex> lck{mtx};     // protect operation
            queue.push_back(m);
            cv.notify_one();                 // notify
        }
        Msg get() {
            unique_lock<mutex> lck{mtx};        // acquire mutex
            while( queue.size() == 0)
                cv.wait(lck);                   // release lock and wait 
            auto m = queue.front();
            queue.pop_front();  // get message
            return m;
        }

    private:
        deque<Msg> queue;
        mutex mtx;
        condition_variable cv;
};

void consumer(int cthid, SyncQueue &sq, bool *run) {
    while( *run ) {
        auto m = sq.get();
        lock_guard<mutex> guard(printMtx);
        cout << "c " << cthid << ": { p " << m.pthid << ", " << m.id << "}\n";
    }
}

void producer(int pthid, SyncQueue &sq, int n) {
    Msg m;
    m.pthid = pthid;
    for( int i = 0; i < n; ++i ) {
        m.id = i;
        sq.put(m);
        this_thread::sleep_for(chrono::milliseconds{10});
    }
}


int main()
{
    SyncQueue syncQueue;
    vector<thread> prods;
    vector<thread> cons;

    bool run = true;

    for( int i = 0; i < 2; ++i )
        cons.emplace_back(consumer, i, ref(syncQueue), &run); //pasa syncqueue como referencia.

    for( int i = 0; i < 3; ++i )
        prods.emplace_back(producer, i, std::ref(syncQueue), 10);

    for( auto &t : prods )
        t.join();

    cout << "Listos los consumidores\n";

    run = false;
    for( auto &t : cons )
        syncQueue.put({-1, -1});
    for( auto &t : cons )
        t.join();

    this_thread::sleep_for(chrono::milliseconds{100});
    cout << "Chau\n";

    return 0;
}
