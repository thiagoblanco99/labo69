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

int p_h[2];
int h_p[2];


const char *msg = "abcdefghijklmnopqrstuvwxyz\n";

void catch_child(int sig_num, siginfo_t *psiginfo, void *pcontext)
{
    wait(NULL);
    printf("Recibi signal #%d %s de %d, code: %d\n", sig_num,
            strsignal(sig_num), psiginfo->si_pid, psiginfo->si_code);
}
void padre(int pid)
{
    int ret;
    int sz8 = strlen(msg);
    char c;
    
    printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
    
    close(p_h[0]);
    close(h_p[1]);
    getchar();
    
    ret = write(p_h[1], msg, sz8);
    if( ret != sz8 )
        perror("write en el pipe");
    
    close(p_h[1]);

    while( read(h_p[0], &c, 1) )
        {
            putchar(c);
        }   
    close(h_p[0]);

    while( 1 )
        pause();
}

void hijo()
{
    char c;
    char salida[100]="";
    int len = strlen(salida);

    printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());

    close(p_h[1]);
    close(h_p[0]);

    salida[len+1] = '\0';
    while( read(p_h[0], &c, 1) )
        {
            putchar(c);
            salida[len+1] = '\0';
            len = strlen(salida);
            salida[len] = c-32;
            
        }
    salida[len+1] = '\n';
    close(p_h[0]);

    const char *mensaje = salida;
    int ret;
    int sz8 = strlen(mensaje);
    
    ret = write(h_p[1], mensaje, sz8);
    if( ret != sz8 )
        perror("write en el pipe");
    
    printf("\n%s\n",salida);
    printf("(%d) pipe closed, bye\n", getpid());
    
    close(h_p[1]);
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
    
    if( pipe(p_h) == -1 ) {
        perror("pipe");
        return 1;
    }
    if( pipe(h_p) == -1 ) {
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
