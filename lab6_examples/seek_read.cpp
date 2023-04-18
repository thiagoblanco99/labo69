#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

using namespace std;

struct Str_t
{
	double t;
	double val;
};

int search_seek(double t, Str_t *result, int fd);
Str_t get_record(int fd, int pos);
int get_nrecords(int fd);

int main()
{
	Str_t mys;

	int fd = open("kk_open", O_RDWR, 0640);
	if (fd < 0)
	{
		perror("open de kk_open");
		exit(1);
	}

	int ret = search_seek(2.3, &mys, fd);
	assert(ret == 0);
	cout << "T = " << mys.t << " Val = " << mys.val << "\n";

	close(fd);

	return 0;
}

int search_seek(double t, Str_t *result, int fd)
{
	int ini = 0, fin, mid;
	Str_t s1, s2;

	fin = get_nrecords(fd) - 1;
	s1 = get_record(fd, ini);
	s2 = get_record(fd, fin);

	if (t < s1.t || t > s2.t)
	{
		cout << "Tiempo fuera de rango\n";
		return (-1);
	}

	while (fin - ini > 1)
	{
		mid = (ini + fin) / 2;

		*result = get_record(fd, mid);

		if (result->t == t)
			return 0;

		if (result->t > t)
		{
			fin = mid;
			s2 = get_record(fd, fin);
		}
		else
		{
			ini = mid;
			s1 = get_record(fd, ini);
		}
	}
	result->t = t;
	result->val = s1.val;
	result->val += ((s2.val - s1.val) / (s2.t - s1.t)) * (t - s1.t);
	return 0;
}

Str_t get_record(int fd, int pos)
{
	Str_t result;
	if(lseek(fd, pos * sizeof(Str_t), SEEK_SET)==-1);
	ssize_t nc = read(fd, &result, sizeof(result));
	assert(nc == sizeof(result));
	return result;
}

int get_nrecords(int fd)
{
	struct stat s;
	int ret;

	ret = fstat(fd, &s);
	assert(ret != -1);

	return s.st_size / sizeof(Str_t);
}
