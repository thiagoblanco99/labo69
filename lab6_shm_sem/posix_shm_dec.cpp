#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdlib.h>
#include "shresource.h"

#define MM_SZ8 (1<<12)

int main()
{
    int shmfd;
    ShResource_t *pRes;
    
    shmfd =  shm_open("/pru2", O_RDWR, 0640);
    assert(shmfd > 0);

    pRes = (ShResource_t *)mmap(NULL, MM_SZ8, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(pRes != MAP_FAILED);
    close(shmfd);

    printf("Press any key to start decrementer\n");
    getchar();

    while(1) {
        resourceLock(pRes);
        pRes->value--;
        resourceUnlock(pRes);
    }
    
    return 0;
}

