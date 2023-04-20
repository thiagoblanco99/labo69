#ifndef __SHRESOURCE_H
#define __SHRESOURCE_H 1

#include <semaphore.h>
#include <assert.h>

struct ShResource_t {
    sem_t sem;
    int   value;
};

static inline void resourceInit(ShResource_t *pRes)
{
    int code = sem_init(&pRes->sem, 1, 1);
    assert(!code);
    pRes->value = 0;
}

// returns true if succcess
static inline int resourceLock(ShResource_t *pRes)
{
    return sem_wait(&pRes->sem) == 0;
}

// returns true if succcess
static inline int resourceUnlock(ShResource_t *pRes)
{
    return sem_post(&pRes->sem) == 0;
}


#endif  //  __SHRESOURCE_H
