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
        if( stdin_alive ) //tengo que hacer esto para que no se quede esperando en el select si stdin esta cerrado
            FD_SET(0, &rfds);
        FD_SET(h_p[0], &rfds); // este siempre esta activo

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
                printf("entre al isset de stdin\n");
                if( ! (retval = read(0, buf, sizeof(buf)))) //retval me dice la cantidad de caracteres que leyo
                { //esta condicion se cumple cuando stdin esta cerrado
                    puts("\nstdin closed\n");
                    stdin_alive = false; // si entro a este if es porque ya termino de leer de stdin, uso esta stdin_alive para no meter el stdin en el set de file descriptors
                    close(p_h[1]);//mato el fd que habla de padre a hijo asi el Perl manda la info que tiene en el buffer
                } else {//esta condicion se cumple cuando stdin esta abierto
                    ret = write(p_h[1], buf, retval);//escribo en el pipe lo que leyo de stdin y uso retval para saber cuantos caracteres escribir
                    if( ret != sizeof(buf) )
                        perror("write en el pipe\n");   
                }             
            } 
            if(FD_ISSET(h_p[0], &rfds))
            {
                printf("entre al isset que lee del hijo\n");
                if( ! (retval = read(h_p[0], BUF, sizeof(BUF))) )//si entro aca es porque estoy leyendo cosas del hijo 
                {
                    puts("\nbye\n");
                    break;
                }
                ret = write(1, BUF, retval); //si leyo del pipe que viene del hijo, muestro en pantalla 
                if( ret != sizeof(BUF) )
                    perror("write en el pipe\n");
            }
        }

    }

    
    getchar();
    
    close(h_p[0]);
    close(p_h[1]);
}

void hijo()
{
    
    printf("hello from hijo, mi pid es %d, mi padre es %d\n", getpid(), getppid());
    printf("estoy aca hijo\n");
    close(p_h[1]);
    close(h_p[0]);
    close(0);//cierro stdinpunt
    close(1);//cierro stdoutput
    int d0 = dup2(p_h[0], 0); // copio el fd de lectura del pipe al fd 0
        if(d0==-1)
            perror("dup2 de 0");
    int d1 = dup2(h_p[1], 1); // copio el fd de escritura del pipe al fd 1
        if(d1==-1)
            perror("dup2 de 1");
    fprintf(stderr,"duplique los fd en los standar inputs y outputs\n");//uso stderr para que no se bloquee la salida, porque el stout ya no existe aca; TRUCAZO para debuggear
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
