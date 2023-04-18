#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

struct Str_t
{
	double t;
	double val;
};

int main()
{
	int fd = open("kk_open", O_WRONLY |O_CREAT| O_EXCL , 0640);
	if (fd < 0)
	{
		perror("open de kk_open");
		exit(1);
	}
	close(fd);

	return 0;
}
