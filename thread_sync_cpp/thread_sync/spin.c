#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>

#define LOOPS 1000

pthread_spinlock_t spinlock;
pthread_mutex_t mutex;

int count;

pid_t gettid() { return syscall( __NR_gettid ); }

void *mtx(void *ptr)
{
    printf("Consumer mtx TID %lu\n", (unsigned long)gettid());

    while (1)
    {
        pthread_mutex_lock(&mutex);

        if( count == 0 ) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        --count;

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *spin(void *ptr)
{
    printf("Consumer spin TID %lu\n", (unsigned long)gettid());

    while (1)
    {
        pthread_spin_lock(&spinlock);

        if( count == 0 ) {
            pthread_spin_unlock(&spinlock);
            break;
        }

        --count;

        pthread_spin_unlock(&spinlock);
    }

    return NULL;
}

void test(void *(*f)(void *), const char *str)
{
    pthread_t thr1, thr2;
    struct timeval tv1, tv2;

    count = LOOPS;

    // Measuring time before starting the threads...
    gettimeofday(&tv1, NULL);

    pthread_create(&thr1, NULL, f, NULL);
    pthread_create(&thr2, NULL, f, NULL);

    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL);

    // Measuring time after threads finished...
    gettimeofday(&tv2, NULL);

    if (tv1.tv_usec > tv2.tv_usec)
    {
        tv2.tv_sec--;
        tv2.tv_usec += 1000000;
    }

    printf("Result %s - %ld.%ld\n", str, tv2.tv_sec - tv1.tv_sec,
        tv2.tv_usec - tv1.tv_usec);
}

int main()
{
    pthread_spin_init(&spinlock, 0);
    pthread_mutex_init(&mutex, NULL);

    test(mtx, "mutex");

    test(spin, "spinlock");

    pthread_spin_destroy(&spinlock);
    pthread_mutex_destroy(&mutex);

    return 0;
}

