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


int main(int argc, char *argv[], char *envp[])
{
	pid_t pid;
    int status;

    printf("[%s] mi pid es %d\n", argv[0], getpid());
    
    pid = fork();
    
    switch( pid ) {
        case 0:
            printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
            execl("/usr/bin/ls", "ls", "-l", NULL);
            perror("error en exec:");
            assert(0);
            
        case -1:
            perror("error en el fork:");
            break;
            
        default:
            printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
        
            pid = wait(&status);
            assert(pid != -1);
            printf("mi hijo %d, retorno %d (0x%04x)%s\n", pid,
                    WEXITSTATUS(status), status,
                    WIFSIGNALED(status) ? " signaled" : " muerte natural");
                    
            break;
    }

	return 0;
}
