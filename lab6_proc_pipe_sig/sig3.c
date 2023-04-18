#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>

void catch_int_info(int sig_num, siginfo_t *psiginfo, void *pcontext)
{
    printf("(%d) Recibi signal #%d %s de %d, code: %d\n", getpid(), sig_num,
            strsignal(sig_num), psiginfo->si_pid, psiginfo->si_code);
}

void padre(int pid)
{
    printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
    getchar();

    printf("kill a %d\n", pid);
    kill(pid, SIGUSR1);
    pause();
}

void hijo()
{
    printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
    pause();
}


int main(int argc, char *argv[])
{
	pid_t pid;
    struct sigaction act;

    printf("[%s] mi pid es %d\n", argv[0], getpid());
    
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = catch_int_info;
    sigfillset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = catch_int_info;
    sigfillset(&act.sa_mask);
    sigaction(SIGUSR1, &act, NULL);

    pid = fork();
    
    switch( pid ) {
        case 0:
            hijo();
            break;
            
        case -1:
            perror("error en el fork:");
            break;
            
        default:
            padre(pid);
            break;
    }

	return 0;
}
