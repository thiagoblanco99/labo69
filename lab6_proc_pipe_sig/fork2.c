#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>


int main(int argc, char *argv[])
{
	pid_t pid;
    int status;

    printf("[%s] mi pid es %d\n", argv[0], getpid());
    
    pid = fork();
    
    switch( pid ) {
        case 0:
            printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
            sleep(10);
            printf("chau from hijo %d\n", getpid());
            assert(0);
            return 42;
            
        case -1:
            perror("error en el fork:");
            break;
            
        default:
            printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
        
            puts("paktc");
            getchar();
            
            pid = wait(&status);
            assert(pid != -1);
            printf("mi hijo %d, retorno %d (0x%04x)%s\n", pid,
                    WEXITSTATUS(status), status,
                    WIFSIGNALED(status) ? " signaled" : " muerte natural");
                    
            puts("paktc");
            getchar();
            
            break;
    }

	return 0;
}
