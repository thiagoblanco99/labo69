#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <time.h>
#include <mutex>
#include <semaphore.h>
#include <condition_variable>

using namespace std;
typedef void (*ProcFunc_t)(void *ctx);
typedef unsigned int WorkUnitId;

void f(void *ctx) {                 // function
    char context=(char)ctx;
    this_thread::sleep_for(chrono::microseconds{rand() % 100 + 1});
    printf("Hola soy el thread %d y tengo para decirte %s", this_thread::get_id(),context);
}

typedef struct {
WorkUnitId id;
ProcFunc_t fun;
void *context;
} WorkUnit_t;

class Workserver_t {

private:
    mutex mtx;
    condition_variable putcv; 
    condition_variable getcv;
    int maxq;//numero maximo de elementos en la cola
    int maxw;//numero maximo de threads
    unsigned int putter=0;
    unsigned int getter=0;
    WorkUnit_t *queue;
    std::thread *workers;

    WorkUnit_t createWorkunit(ProcFunc_t fun, void *ctx)
    {
        WorkUnit_t wu;
        wu.fun=fun;
        wu.context=ctx;
        return wu;
    }
    void get(WorkUnit_t* wu)
    {
        unique_lock<mutex> lck(mtx);
        while(getter%maxq==putter%maxq){
            getcv.wait(lck);
        }
        *wu=queue[getter%maxq];
        getter++;
        putcv.notify_one();
    }
    void work()
    {
        WorkUnit_t wu;
        while(true){
            get(&wu);
            wu.fun(wu.context);
        }

    }
    void start()
    {
        for(int i=0;i<maxw;i++){
            workers[i]=thread(Workserver_t::work,this);
        }
    }
    public:
    Workserver_t(int nq, int nw)
    {
        maxq=nq;
        maxw=nw;
        queue = new WorkUnit_t[nq];
        workers = new std::thread[nw];
        start();
    }
    ~Workserver_t()
    {
        //tengo que agarrar todos los threads y vaciarlos
        delete[] queue;
        delete[] workers;
    }
    void put(WorkUnit_t wu)
    {
        unique_lock<mutex> lck(mtx);
        while((putter+1)%maxq==getter%maxq){
            putcv.wait(lck);
        }
        queue[putter%maxq]=wu;
        putter++;
        getcv.notify_one();
    }
};