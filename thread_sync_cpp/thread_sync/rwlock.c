#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXNTHREADS 100

void *reader(void *), *writer(void *);

int nloop = 1000,
    nreaders = 50,
    nwriters = 40;

struct {
   pthread_rwlock_t  rwlock;
   pthread_mutex_t   rcountlock;
   int               nreaders;
   int               nwriters;
} shared = { PTHREAD_RWLOCK_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };

int main()
{
    int i;
    pthread_t tid_readers[MAXNTHREADS],
              tid_writers[MAXNTHREADS];

    pthread_mutex_init(&shared.rcountlock, NULL);
    pthread_rwlock_init(&shared.rwlock, NULL);
    

    /* create all the reader and writer threads */
    for (i = 0; i < nreaders; i++)
        pthread_create(&tid_readers[i], NULL, reader, NULL);
    printf("readers created\n");
    
    for (i = 0; i < nwriters; i++)
        pthread_create(&tid_writers[i], NULL, writer, NULL);
    printf("writers created\n");
    
    /* wait for all the threads to complete */
    for (i = 0; i < nreaders; i++)
        pthread_join(tid_readers[i], NULL);
    printf("readers done\n");
    
    for (i = 0; i < nwriters; i++)
        pthread_join(tid_writers[i], NULL);
    printf("writers done\n");

    return 0;
}

void *reader(void *arg)
{
    int i;
        
    sleep(1);
        
    for (i = 0; i < nloop; i++) {
        if (pthread_rwlock_rdlock(&shared.rwlock) < 0)
            perror("rdlock");

        if (pthread_mutex_lock(&shared.rcountlock) < 0)
            perror("mutex lock");
        
        shared.nreaders++;        /* shared by all readers; must protect */
        if (pthread_mutex_unlock(&shared.rcountlock) < 0)
            perror("mutex unlock");

        if (shared.nwriters > 0)
            fprintf(stderr, "reader: %d writers found\n", shared.nwriters), exit(1);

        if (pthread_mutex_lock(&shared.rcountlock) < 0)
            perror("mutex lock");
            
        shared.nreaders--;        /* shared by all readers; must protect */
        if (pthread_mutex_unlock(&shared.rcountlock) < 0)
            perror("mutex unlock");

        if (pthread_rwlock_unlock(&shared.rwlock) < 0)
            perror("unlock");
    }
    
    return NULL;
}

void *writer(void *arg)
{
    int i;

    sleep(1);
    
    for (i = 0; i < nloop; i++) {
        if (pthread_rwlock_wrlock(&shared.rwlock) < 0)
            perror("wrlock");
        shared.nwriters++;        /* only one writer; need not protect */

        if (shared.nwriters > 1)
            fprintf(stderr, "writer: %d writers found\n", shared.nwriters), exit(1);
            
        if (shared.nreaders > 0)
            fprintf(stderr, "writer: %d readers found\n", shared.nreaders), exit(1);

        shared.nwriters--;        /* only one writer; need not protect */
        if (pthread_rwlock_unlock(&shared.rwlock) < 0)
            perror("unlock");
    }
    
    return NULL;
}

