#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define NUM_THREADS 10

pthread_barrier_t barrier;

void *thr_func(void *arg)
{
    long tid;
    int ret;

    tid = (long)arg;

    printf("hello from thr_func, thread id: %ld\n", tid);

    if (tid == 0)
        sleep(3);

    ret = pthread_barrier_wait(&barrier);

    printf("%ld done\n", tid);

    if (ret == PTHREAD_BARRIER_SERIAL_THREAD)
        printf("thread %ld got magic BARRIER_SERIAL\n", tid);
    else
    {
        assert(!ret);
    }

    return NULL;
}

int main()
{
    pthread_t thr[NUM_THREADS];
    long i;
    int rc;

    /* initialize pthread mutex protecting "shared_x" */
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    /* create threads */
    for (i = 0; i < NUM_THREADS; ++i)
    {
        if ((rc = pthread_create(&thr[i], NULL, thr_func, (void *)i)))
        {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }
    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(thr[i], NULL);
    }

    return 0;
}
