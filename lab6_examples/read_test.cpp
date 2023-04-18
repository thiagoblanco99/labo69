#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

using namespace std;

struct Str_t
{
	double t;
	double val;
};

int main()
{
	Str_t mys;

	int fd = open("kk_open", O_RDONLY);
	if (fd < 0)
	{
		perror("open de kk_open");
		exit(1);
	}

	int tot = 0;

	while (1)
	{
		ssize_t nc = read(fd, &mys, sizeof(mys));
		if (nc == 0)
			break;
		if (nc < 0)
		{
			perror("reading from kk_open");
			exit(1);
		}
		if (nc < sizeof(mys))
		{
			cout << "Estructura corrupta\n";
			exit(2);
		}

		cout << tot << ":: T = " << mys.t << "Val = " << mys.val << endl;
		tot++;
	}

	close(fd);

	return 0;
}
