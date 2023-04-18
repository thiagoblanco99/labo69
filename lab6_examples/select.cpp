#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFLEN 8

int main(void)
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    char buf[BUFLEN+1];

    while( 1 ) {
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);

        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(1, &rfds, NULL, NULL, &tv);

        if( retval == -1 ) {
            perror("select()");
            return 1;
        }

        if( retval ) {
            if( FD_ISSET(0, &rfds) ) {
                if( ! (retval = read(0, buf, BUFLEN)) ) {
                    puts("\nbye");
                    break;
                }
                buf[retval] = '\0';
                printf("%s", buf);
            }
        } else {
            puts("No data within five seconds.");
        }
    }

    return 0;
}
