#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
QUEUE_MAX_SIZE = 10;

typedef struct {
    int inicio; // índice de inicio de la cola
    int fin; // índice de fin de la cola
    int contador; // cantidad de elementos en la cola
    int max_size; // tamaño máximo de la cola
    int buffer[QUEUE_MAX_SIZE]; // arreglo de elementos de la cola
    sem_t *mutex; // semáforo de acceso a la cola
    sem_t *empty; // semáforo de entradas libres en la cola
    sem_t *full; // semáforo de elementos en la cola
} Queue_t;

Queue_t *QueueCreate(const char *qname, uint32_t qsize) {
    int shmfd = shm_open(qname, O_CREAT | O_RDWR, 0640);
    assert(shmfd > 0);
    ftruncate(shmfd, sizeof(Queue_t));
    Queue_t *cola =(Queue_t *)mmap(NULL, sizeof(Queue_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(cola != MAP_FAILED);
    close(shmfd);

    cola->max_size = qsize;







    }