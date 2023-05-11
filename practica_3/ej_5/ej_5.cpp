#define _GNU_SOURCE

#include <stdio.h>
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
#include <sys/time.h>
#include <cstdlib>
#include <semaphore.h>
#include <sys/mman.h>
#include <assert.h>



int p_h[2];
int h_p[2];
timeval t_inicio, t_fin;

void padre(int num_bloq, int size_bloq){//tiene que retornar el tiempo
    
    close(p_h[0]);
    close(h_p[1]);
    int ret=0;
    char buff[size_bloq] = {'a'};
    gettimeofday(&t_inicio, NULL);

    for(int i=0;i<num_bloq;i++){
    
        ret = write(p_h[1],buff, size_bloq);
        if( ret != size_bloq ){
            perror("write en el pipe");
        }
        /*else{
            printf("escribi %d bloques\n",i+1);
        }*/
    }
    
    close(p_h[1]);
    
    while(1){    //printf("%d \n",ret);
        ret = read(h_p[0],&t_fin, sizeof(timeval));
        if( ret == -1 ){
            perror("read en el pipe");
        }
        if( ret == 0 ){
            break;
        }
        if(ret==1){
            break;
        }
    }
    close(h_p[0]);

    timeval resultado;
    resultado.tv_sec = t_fin.tv_sec - t_inicio.tv_sec;
    resultado.tv_usec = t_fin.tv_usec - t_inicio.tv_usec;
    float tiempo_total = resultado.tv_sec + resultado.tv_usec / 1000000.0;
    float W = num_bloq*size_bloq / tiempo_total;
    printf("Se mandaron %d bloques de %d bytes en %ld segundos y %ld microsegundos\n El ancho de banda es %f byte/s\n",num_bloq,size_bloq,resultado.tv_sec, resultado.tv_usec,W);
}

void hijo (int size_bloq){
    close(p_h[1]);
    close(h_p[0]);
    int ret=0;
    int cont=0;
    char buff[size_bloq];
    while( read(p_h[0], buff, size_bloq) ){
        ++cont;
        printf("lei %d bloques de %s\n",cont,buff);
    }
    
    gettimeofday(&t_fin, NULL);
    close(p_h[0]);
    
    ret = write(h_p[1],&t_fin, sizeof(timeval));
    if( ret == -1 ){
        perror("write en el pipe");
    }
    close(h_p[1]);


}


int main(int argc, char *argv[])
{
	if(argc!=3){
        printf("error: no hay dos argumentos");
        return 1;
    }
    int num_bloq = atoi(argv[1]);
    int size_bloq = atoi(argv[2]);
    pid_t pid;
    
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
            hijo(size_bloq);
            break;
            
        case -1:
            perror("error en el fork:");
            break;
            
        default:
            padre(num_bloq, size_bloq);
            break;
    }

	return 0;
}