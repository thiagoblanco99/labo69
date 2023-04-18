#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char *argv[])
{
	pid_t pid;

    printf("[%s] mi pid es %d\n", argv[0], getpid());
    
    pid = fork();
    
    switch( pid ) {
        case 0:
            printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
            break;
            
        case -1:
            perror("error en el fork:");
            break;
            
        default:
            printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
            getchar();
            break;
    }

	return 0;
}
