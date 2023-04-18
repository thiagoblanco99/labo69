#include <iostream>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define MM_SZ8 (1<<20)

int main(int argc, char *argv[])
{
    cout << "[" << argv[0] << "] mi pid es " << getpid() << endl;
    
    int fd = open("mm", O_RDWR | O_CREAT | O_TRUNC, 0640);
    assert(fd > 0);

    int ret = ftruncate(fd, MM_SZ8);
    assert(ret != -1);

    int *ptr = (int *)mmap(NULL, MM_SZ8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != MAP_FAILED);

    cout << "mmap done, ptr: " << ptr << endl;;
    getchar();
    
    close(fd);

    cout << "close done\n";
    getchar();
    
    for( int i = 0; i < MM_SZ8/sizeof(int); i++ )
        ptr[i] = i;
    
    cout << "memfill done\n";
    getchar();

    munmap(ptr, MM_SZ8);
            
    cout << "munmap done\n";
    getchar();

    return 0;
}

