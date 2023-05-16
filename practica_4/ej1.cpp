#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <time.h>
#include <mutex>


using namespace std;
typedef void (*ProcFunc_t)(void *ctx);
typedef unsigned int WorkUnitId;

//typedef struct {
//Time_t submitTime; // submission time
//Time_t startProcTime; // processing start time
//Time_t endProcTime; // processing end time
//} WorkUnitStat_t;

typedef struct {
WorkUnitId id;
ProcFunc_t fun;
void *context;
//WorkUnitStat_t stats;
} WorkUnit_t;

void f(void *ctx) {                 // function
    char context=(char)ctx;
    this_thread::sleep_for(chrono::microseconds{rand() % 100 + 1});
    printf("Hola soy el thread %d y tengo para decirte %s", this_thread::get_id(),context);
}


class Workserver_t {

private:
    WorkUnit_t *queue;
    std::thread *workers;
    int maxq;
    int maxw;
    int putter=0;
    int getter=0;
    mutex m;
    mutex empty;
    mutex full;
    Workserver_t(int nq, int nw)
    {
        maxq=nq;
        maxw=nw;
        queue = new WorkUnit_t[nq];
        workers = new std::thread[nw];
    }
    ~Workserver_t()
    {
        //delete[] queue;
        //delete[] workers;
    }
    
    void   CreateWork_unit(ProcFunc_t f, void *ctx)
    {
    WorkUnit_t *wu = new WorkUnit_t;
    wu->fun = f;
    wu->context = ctx;
    //wu->id = 0;
    }
    
    void  DestroyWork_unit(WorkUnit_t *wu)
    {
        delete wu;
    }

    void putWork_unit()
    {
        m.lock();
        while(putter){
        }
        m.unlock();
    }

    void getWork_unit()
    {

    }

};
