#define _GNU_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <cstdlib>
#include <semaphore.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>

typedef struct {
    uint putter; // índice donde pone 
    uint getter; // índice donde saca
    uint32_t max_size; // tamaño máximo de la cola
    sem_t mutex; // semáforo de acceso a la cola
    sem_t empty; // semáforo para getters
    sem_t full; // semáforo para putters
    char nombre[20];
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

    memcpy(cola->nombre,qname,strlen(qname)+1);
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
        int error = shm_unlink((const char*)&pQ->nombre); // es así o tengo que borrar tambien la parte del buffer que no agregue en Queue_t ?
        assert(error == 0);//el casteo de arriba esta bien ? 
        }

void QueuePut(Queue_t *pQ, int elem){
    
    
    sem_wait(&pQ->full);
    sem_wait(&pQ->mutex);
    int lugar = pQ->putter%pQ->max_size;
    pQ->buffer[lugar] = elem;
    pQ->putter++;
    sem_post(&pQ->mutex);
    sem_post(&pQ->empty);

    }

int QueueGet(Queue_t *pQ){
    
    
    sem_wait(&pQ->empty);
    sem_wait(&pQ->mutex);
    int lugar = pQ->getter%pQ->max_size;
    int elem = pQ->buffer[lugar];
    pQ->getter++;
    sem_post(&pQ->mutex);
    sem_post(&pQ->full);
    return elem;

    }

int QueueCnt(Queue_t *pQ){
    sem_wait(&pQ->mutex);
    int cant = pQ->putter - pQ->getter;
    sem_post(&pQ->mutex);
    return cant;

    }

//----------------------------------------------------------------------------
// Ejercicio 6
//----------------------------------------------------------------------------
timeval t_inicio, t_fin;

void padre(int num_bloq, int size_bloq, pid_t pid){
    
    Queue_t *cola = QueueCreate("cola", num_bloq*size_bloq);
    gettimeofday(&t_inicio, NULL);
    for(int i=0; i<num_bloq; i++){
        for(int j=0; j<size_bloq; j++){
            QueuePut(cola,j);
            //printf ("mande el numero %d\n", num);
        }
    }
    //QueueDetach(cola);
    wait(NULL);

    gettimeofday(&t_fin, NULL);
    timeval resultado;
    resultado.tv_sec = t_fin.tv_sec - t_inicio.tv_sec;
    resultado.tv_usec = t_fin.tv_usec - t_inicio.tv_usec;
    float tiempo_total = resultado.tv_sec + resultado.tv_usec / 1000000.0;
    float W = num_bloq*4*size_bloq / tiempo_total;
    printf("Tardo %ld segundos y %ld microsegundos\n El ancho de banda es %f Hz\n",resultado.tv_sec, resultado.tv_usec,W);    QueueDestroy(cola);
    }
    
void hijo(int size_bloq){
    Queue_t *cola = QueueAttach("cola");
    int num = 0;
    while(QueueCnt(cola) > 0){
        num = QueueGet(cola);
        //printf("num: %d\n", num);
    }
    QueueDetach(cola);
    }

int main(int argc, char *argv[]){
	if(argc!=3){
        printf("error: no hay dos argumentos");
        return 1;
    }
    int num_bloq = atoi(argv[1]);
    int size_bloq = (atoi(argv[2]))/4;
    pid_t pid;
    
    pid = fork();
    
    switch( pid ) {
        case 0:
            hijo(size_bloq);
            break;
            
        case -1:
            perror("error en el fork:");
            break;
            
        default:
            padre(num_bloq, size_bloq, pid);
            break;
    }

	return 0;
}