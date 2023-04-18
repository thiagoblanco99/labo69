#define _GNU_SOURCE

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
#include <signal.h>
#include <string.h>

int pfd[2];

const char *msg = "abcdefghijklmnopqrstuvwxyz\n";

void catch_child(int sig_num, siginfo_t *psiginfo, void *pcontext)
{
    wait(NULL);
    printf("Recibi signal #%d %s de %d, code: %d\n", sig_num,
            strsignal(sig_num), psiginfo->si_pid, psiginfo->si_code);
}

void catch_int_info(int sig_num, siginfo_t *psiginfo, void *pcontext)
{
    printf("(%d) Recibi signal #%d %s de %d, code: %d\n", getpid(), sig_num,
            strsignal(sig_num), psiginfo->si_pid, psiginfo->si_code);
}

void padre(int pid)
{
    int ret;
    int sz8 = strlen(msg);
    
    printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
    
    close(pfd[0]);
    
    getchar();
    
    ret = write(pfd[1], msg, sz8);
    if( ret != sz8 )
        perror("write en el pipe");
    
    close(pfd[1]);

    while( 1 )
        pause();
}

void hijo()
{
    printf("bye from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
}


int main(int argc, char *argv[])
{
	pid_t pid;
    struct sigaction act;

    printf("[%s] mi pid es %d\n", argv[0], getpid());
    
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = catch_child;
    sigfillset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);
    
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = catch_int_info;
    sigfillset(&act.sa_mask);
    sigaction(SIGPIPE, &act, NULL);

//    signal(SIGPIPE, SIG_IGN);
    
    if( pipe(pfd) == -1 ) {
        perror("pipe");
        return 1;
    }
    
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
