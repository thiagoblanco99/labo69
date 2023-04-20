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

void catch_child(int sig_num, siginfo_t *psiginfo, void *pcontext)
{
    wait(NULL);
    printf("Recibi signal #%d %s de %d, code: %d\n", sig_num,
            strsignal(sig_num), psiginfo->si_pid, psiginfo->si_code);
}

void padre(int pid)
{
    close(p_h[0]);
    close(h_p[1]);
    fd_set rfds;
    int retval;
    char buf[100];//buffer para stdin
    char BUF[100];//buffer para lo que lee del hijo
    int ret;
    bool stdin_alive = true; 
    printf("hello from padre (%d), hijo es %d\n", getpid(), pid);
    while(1)
    {
        FD_ZERO(&rfds);
        if( stdin_alive )
            FD_SET(0, &rfds);
        FD_SET(h_p[0], &rfds);

        retval = select( h_p[0]+1 , &rfds, NULL, NULL, NULL); //hago select de stdin o de la lectura de hijo
        if( retval == -1 ) 
        {
            perror("select()");
            return;
        }
        if(retval)
        {
            if( FD_ISSET(0, &rfds) ) 
            {
                printf("entre al isset de stdin");
                if( ! (retval = read(0, buf, sizeof(buf)))) 
                {
                    puts("\nstdin closed");
                    stdin_alive = false;
                    close(p_h[1]);
                } else {
                    ret = write(p_h[1], buf, retval);
                    if( ret != sizeof(buf) )
                        perror("write en el pipe");   
                }             
            }   
            if(FD_ISSET(h_p[0], &rfds))
            {
                printf("entre al isset que lee del hijo");
                if( ! (retval = read(h_p[0], BUF, sizeof(BUF))) ) 
                {
                    puts("\nbye");
                    break;
                }
                ret = write(1, BUF, retval);
                if( ret != sizeof(BUF) )
                    perror("write en el pipe");
            }
        }

    }

    
    getchar();
    
    
    open(/dev/tty);
    close(h_p[0]);
    close(p_h[1]);
}

void hijo()
{
    
    printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
    printf("estoy aca hijo\n");
    close(p_h[1]);
    close(h_p[0]);
    close(0);
    close(1);
    int d0 = dup2(p_h[0], 0);
        if(d0==-1)
            perror("dup2 de 0");
    int d1 = dup2(h_p[1], 1);
        if(d1==-1)
            perror("dup2 de 1");
    fprintf(stderr,"duplique los fd en los standar inputs y outputs\n");
    close(p_h[0]);
    close(h_p[1]);
    fprintf(stderr,"cierro los fd duplicados\n");
    execl("/usr/bin/perl", "perl", "-pe","$_ = uc $_", NULL); //ejecuta el comando perl -pe '$_ = uc$_' y lo envia a la salida estandar
    printf("(%d) pipe closed, bye\n", getpid());
    close(0);
    close(1);
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
