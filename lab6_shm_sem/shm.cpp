#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

static const size_t MM_SZ8 = 4096;
static const size_t COUNT = 1000000;

static void hijo(int *ptr);
static void padre(int *ptr);

int main()
{
    int *ptr = (int *) mmap(NULL, MM_SZ8, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    assert(ptr != MAP_FAILED);
    *ptr = 0;
    
    pid_t pid = fork();

    switch( pid ) {
        case -1:
            perror("Error en el fork");
            return 1;
        case 0:
            hijo(ptr);
            break;
        default:
            padre(ptr);
            break;
    }

    munmap(ptr, MM_SZ8);

    return 0;
}

void hijo(int *ptr)
{
    size_t cnt = COUNT;
    while( cnt-- ) {
        --*ptr;
        if( cnt % 10000 == 0 )
            cout << "Hijo val: " << *ptr << endl;
    }
    cout << "Hijo final val: " << *ptr << endl;
}

void padre(int *ptr)
{
    size_t cnt = COUNT;
    while( cnt-- ) {
        ++*ptr;
        if( cnt % 10000 == 0 )
            cout << "Padre val: " << *ptr << endl;
    }
    cout << "Padre final val: " << *ptr << endl;
}