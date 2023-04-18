#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

using namespace std;

struct Str_t {
	double t;
	double val;
};

int search(Str_t *result, Str_t *pstart, int nrecs);

int main()
{
	int fd = open("kk_open", O_RDWR);
    assert(fd > 0);

	struct stat st;
	int ret = fstat(fd, &st);
    assert(ret != -1);

    Str_t *ptr = (Str_t *) mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != MAP_FAILED);

    close(fd);

    Str_t mys = {2.3};
	ret = search(&mys, ptr, st.st_size/sizeof(Str_t));
	assert(ret == 0);
    
	cout << "T = " << mys.t << " Val = " << mys.val << endl;

    munmap(ptr, st.st_size);

	return 0;
}

int search(Str_t *result, Str_t *p, int nrecs)
{
    int fin = nrecs - 1,
        ini = 0,
        mid;

    if( result->t < p[ini].t || result->t > p[fin].t ) {
        printf ("Tiempo fuera de rango\n");
        return -1;
    }

    while( fin - ini > 1 ) {
        mid = (ini+fin) >> 1;

        if( p[mid].t == result->t ) {
            result->val = p[mid].val;
            return 0;
        }

        if( p[mid].t > result->t )
            fin = mid;
        else
            ini = mid;
    }
    result->val  = p[mid].val;
    result->val += ((p[mid+1].val-p[mid].val)/(p[mid+1].t-p[mid].t)) *(result->t-p[mid].t);
    return 0;
}

