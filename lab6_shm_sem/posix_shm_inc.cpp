#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shresource.h"

#define MM_SZ8 (1<<12)

int main()
{
    int ret;
    int shmfd;
    ShResource_t *pRes;
   
    shmfd =  shm_open("/pru2", O_RDWR | O_CREAT, 0640);
    assert(shmfd > 0);

    ret = ftruncate(shmfd, MM_SZ8);
    assert(ret != -1);

    pRes = (ShResource_t *)mmap(NULL, MM_SZ8, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(pRes != MAP_FAILED);
    close(shmfd);

    printf("Initializing resource @ 0x%p\n", pRes);

    resourceInit(pRes);

    printf("Press any key to start incrementer\n");
    getchar();

    while(1) {
        resourceLock(pRes);
        pRes->value++;
        resourceUnlock(pRes);
    }
    
    
    return 0;
}

