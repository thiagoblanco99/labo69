#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <time.h>
#include <mutex>
#include <semaphore.h>

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
    unsigned int putter=0;
    unsigned int getter=0;
    mutex m;
    sem_t empty;
    sem_t full;
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
    
    void   CreateWork_unit(ProcFunc_t f, void *ctx, WorkUnit_t *wu)
    {
    wu->fun = f;
    wu->context = ctx;
    //wu->id = 0;
    printf("se creo un trabajo \n");
    }
    
    //void  DestroyWork_unit(WorkUnit_t *wu)
    //{
    //    delete wu;
    //}

    void putWork_unit(ProcFunc_t f, void *ctx)
    {
        sem_wait(&full);
        m.lock();
        CreateWork_unit(f,ctx,&queue[putter]);
        putter=(putter+1)%maxq;
        m.unlock();
        sem_post(&empty);
    }

    void getWork_unit(WorkUnit_t *wu)
    {
        sem_wait(&empty);
        m.lock();
        wu=&queue[getter];
        getter=(getter+1)%maxq;
        m.unlock();
        sem_post(&full);
    }

};
