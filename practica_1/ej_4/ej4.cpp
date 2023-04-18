#include <iostream>
#include <stdio.h>
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



int main()
{
    struct stat s;
	char msg;
    msg = 'a';

	int fd = open("agujereado", O_RDWR | O_CREAT | O_EXCL, 0640);
	if (fd < 0)
	{
		perror("open de kk_open");
		exit(1);
	}

	if (write(fd, &msg, sizeof(msg)) != sizeof(msg))
		{perror("on write");}
	close(fd);

    fd = open("agujereado", O_RDWR, 0640);
    if (fd < 0)
	{
		perror("open de kk_open");
		exit(1);
	}
    if(lseek(fd,1000*sizeof(msg), SEEK_SET)==-1)
	{
		perror("error haciendo el seek");
		exit(1);
	}
    if (write(fd, &msg, sizeof(msg)) != sizeof(msg))
	    {perror("on write");}
	close(fd);

    if (stat("agujereado", &s) == -1) {
        perror("Error al obtener la información del archivo");
        exit(1);
    }
    printf("ID del dispositivo: %ld\n", s.st_dev);
    printf("Número de inodo: %ld\n", s.st_ino);
    printf("Tipo de archivo: %o\n", s.st_mode);
    printf("Número de enlaces: %ld\n", s.st_nlink);
    printf("ID del propietario: %d\n", s.st_uid);
    printf("ID del grupo: %d\n", s.st_gid);
    printf("ID del dispositivo (si es especial): %ld\n", s.st_rdev);
    printf("Tamaño del archivo: %ld bytes\n", s.st_size);
    printf("Tamaño de bloque óptimo: %ld bytes\n", s.st_blksize);
    printf("Número de bloques asignados: %ld\n", s.st_blocks);
    printf("Tiempo de último acceso: %ld\n", s.st_atime);
    printf("Tiempo de última modificación: %ld\n", s.st_mtime);
    printf("Tiempo de cambio de estado: %ld\n", s.st_ctime);

	return 0;
}
