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

int main(int argc, char *argv[])
{
    printf("mi pid es %d\n", getpid());
    signal(SIGINT, catch_int);
    signal(SIGQUIT, catch_int);

    while( 1 )
        pause();

    return 0;
}
