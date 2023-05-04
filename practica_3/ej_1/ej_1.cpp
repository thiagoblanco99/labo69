#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

typedef struct {
    uint putter; // índice donde pone 
    uint getter; // índice donde saca
    uint max_size; // tamaño máximo de la cola
    sem_t mutex; // semáforo de acceso a la cola
    sem_t empty; // semáforo para getters
    sem_t full; // semáforo para putters
    int buffer[0]; // puntero al inicio de la cola
} Queue_t;

Queue_t *QueueCreate(const char *qname, uint32_t qsize) { //creo un nuevo segmento de memoria compartida de queue_t + el buffer con qsize

    int shmfd = shm_open(qname, O_CREAT | O_RDWR, 0640);
    assert(shmfd > 0);

    ftruncate(shmfd, sizeof(Queue_t) + qsize * sizeof(int));

    Queue_t *cola =(Queue_t *)mmap(NULL, sizeof(Queue_t)+ qsize * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(cola != MAP_FAILED);

    close(shmfd);
    cola->max_size = qsize;
    cola->putter = 0;
    cola->getter = 0;
    sem_init(&cola->mutex, 1, 1);//1 es el valor inicial del semaforo
    sem_init(&cola->empty, 1, 0);// 
    sem_init(&cola->full, 1, qsize);// 
    return cola;
    }
    
Queue_t *QueueAttach(const char *qname){

    struct stat st;
    int shmfd = shm_open(qname,O_RDWR, 0640);
    assert(shmfd >= 0);
    int error = fstat(shmfd, &st);
    assert(error==0);
    Queue_t *cola =(Queue_t *)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(cola != (Queue_t*)-1);
    close(shmfd);
    return cola;
    }

void QueueDetach(Queue_t *q){

    int error = munmap(q, sizeof(Queue_t)+ q->max_size * sizeof(int));
    assert(error == 0);
    }

void QueueDestroy(Queue_t *pQ){

        sem_destroy(&pQ->mutex);
        sem_destroy(&pQ->empty);//esta bien destruir estos semaforos ? 
        sem_destroy(&pQ->full);
        int error = shm_unlink((char*)pQ); // es así o tengo que borrar tambien la parte del buffer que no agregue en Queue_t ?
        assert(error == 0);//el casteo de arriba esta bien ? 
        }

void QueuePut(Queue_t *pQ, int elem){
    
    int lugar = pQ->putter%pQ->max_size;
    sem_wait(&pQ->full);
    sem_wait(&pQ->mutex);
    pQ->buffer[lugar] = elem;
    pQ->putter++;
    sem_post(&pQ->mutex);
    sem_post(&pQ->empty);

    }

int QueueGet(Queue_t *pQ){
    
    int lugar = pQ->getter%pQ->max_size;
    sem_wait(&pQ->empty);
    sem_wait(&pQ->mutex);
    int elem = pQ->buffer[lugar];
    pQ->getter++;
    sem_post(&pQ->mutex);
    sem_post(&pQ->full);
    return elem;

    }

int QueueCnt(Queue_t *pQ){
    sem_wait(&pQ->mutex);
    int cant = pQ->putter%pQ->max_size - pQ->getter%pQ->max_size;
    sem_post(&pQ->mutex);
    return cant;

    }