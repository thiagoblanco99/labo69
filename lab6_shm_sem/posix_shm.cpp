#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

static const size_t MM_SZ8 = 4096;

int main()
{
    int ret;
    int shmfd;
    int cnt = 5;
    
    shmfd =  shm_open("/pru1", O_RDWR | O_CREAT, 0640);
    assert(shmfd > 0);

    ret = ftruncate(shmfd, MM_SZ8);
    assert(ret != -1);

    int *ptr = (int *) mmap(NULL, MM_SZ8, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    assert(ptr != MAP_FAILED);
    *ptr = 0;
    
    close(shmfd);

    while(cnt--) {
        ++*ptr;
        cout << "Val: " << *ptr << endl;
        cin.get();
    }

    munmap(ptr, MM_SZ8);

    shm_unlink("/pru1"); 
    
    return 0;
}

