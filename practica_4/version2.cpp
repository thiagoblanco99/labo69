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
    char* context=(char*)ctx;
    this_thread::sleep_for(chrono::seconds{rand() % 5 + 1});
    std::cout<<"Hola soy el thread "<<this_thread::get_id()<<" y tengo para decirte "<<*context<<std::endl;
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
    bool Running=false;


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
    static void work(Workserver_t* ws)
    {
        WorkUnit_t wu;
        while(ws->isRunning()){
            
            ws->get(&wu);
            printf("tome un workunit con running %d\n",ws->isRunning());
            if(!ws->isRunning())break;
            printf("ejecutando con running %d\n",ws->isRunning());
            wu.fun(wu.context);
        }

    }
    void start()
    {
        Running=true;
        for(int i=0;i<maxw;i++){
            //esta iniciacion esta bien?
            workers[i]=thread(Workserver_t::work,this);
        }
    }
    public:
    Workserver_t(int nq, int nw)
    {
        maxq=nq;
        maxw=nw;
        queue = new WorkUnit_t[maxq];
        workers = new std::thread[maxw];
        start();
    }
    ~Workserver_t()
    {
        WorkUnit_t  fake;
        Running=false;
        printf("inicio destruccion\n");
        for(int i=0;i<maxw;i++){put(fake);}
        for(int i=0;i<maxw;i++){
            printf("destruyendo thread %d\n",i);
            workers[i].join();
        }
        delete[] queue;
        delete[] workers;
    }
    WorkUnit_t createWorkunit(ProcFunc_t fun, void *ctx)
    {
        WorkUnit_t wu;
        wu.fun=fun;
        wu.context=ctx;
        return wu;
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
    bool isRunning()
    {
        return Running;
    }


};
void FakeUnitgen_t(Workserver_t* ws, char* c, int n)
{
    for(int i=0; i<n; i++){
        ws->put(ws->createWorkunit(f,(void*)c));
    }
}

int main(){
char c='a';
Workserver_t ws(10,10);
FakeUnitgen_t(&ws,&c,10);
cin.get();
printf("terminando...\n");
return 0;

}