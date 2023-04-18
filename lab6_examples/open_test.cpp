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
	Str_t mys;

	int fd = open("kk_open", O_RDWR | O_CREAT | O_EXCL, 0640);
	if (fd < 0)
	{
		perror("open de kk_open");
		exit(1);
	}

	for (int i = 0; i < 10000; i++)
	{
		mys.t += 0.1 * (rand() % 10);
		mys.val = sin(mys.t);
		if (write(fd, &mys, sizeof(mys)) != sizeof(mys))
			perror("on write");
	}

	close(fd);

	return 0;
}
