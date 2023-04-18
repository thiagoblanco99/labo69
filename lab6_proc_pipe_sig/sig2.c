#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

void catch_int(int sig_num)
{
    printf("Recibi signal #%d %s\n", sig_num, strsignal(sig_num));
}

void catch_int_info(int sig_num, siginfo_t *psiginfo, void *pcontext)
{
    printf("Recibi signal #%d %s de %d, code: %d\n", sig_num,
            strsignal(sig_num), psiginfo->si_pid, psiginfo->si_code);
}

int main(int argc, char *argv[])
{
    struct sigaction act, old;
    
    printf("mi pid es %d\n", getpid());
    
    act.sa_flags     = SA_SIGINFO;
    act.sa_sigaction = catch_int_info;
    sigfillset(&act.sa_mask);
    
    sigaction(SIGINT, &act, &old);

    act.sa_flags   = 0;
    act.sa_handler = catch_int;
    sigfillset(&act.sa_mask);
    
    sigaction(SIGQUIT, &act, &old);

    act.sa_flags   = 0;
    act.sa_handler = catch_int;
    sigfillset(&act.sa_mask);
    
    sigaction(SIGKILL, &act, &old);

    while( 1 )
        pause();

    return 0;
}
