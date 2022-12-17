#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#define MAXprod 20
#define SEM_MODE 0666
#define SHM_MODE 0666
#define MAX_MEM (sizeof(seg)*MAXprod)

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/*   union   semun   is   defined   by   including   <sys/sem.h>   */ 
#else 
/*   according   to   X/OPEN   we   have   to   define   it   ourselves   */ 
union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
#endif

typedef struct queue{
    int start;
    int end;
}queue;
queue *shmp;

typedef struct segment{
    char commName[10];
    double commPrice;
} seg;


void printComm(seg* comm){
    printf("-------------------------\n");
    printf("commodity name: %s\ncommodity price: %4.2f\n",comm->commName, comm->commPrice);
}


void printQueue(queue* q, char* name){
    printf("\n%s data:\nstart: %ld\tend: %ld\n", name, q->start/sizeof(seg), q->end/sizeof(seg));
}


void waitSem(int semSetId,int semNum)
{
    struct sembuf sb;
    sb.sem_num = semNum;
    sb.sem_op = -1;//Indicates that the semaphore is to be reduced by one
    sb.sem_flg = SEM_UNDO;//
    //The second parameter is of type sembuf [], representing an array
    //The third parameter represents the size of the array represented by the second parameter
    if(semop(semSetId,&sb,1) < 0){
        perror("waitSem failed");
        exit(1);
    }
}


void sigSem(int semSetId,int semNum)
{
    struct sembuf sb;
    sb.sem_num = semNum;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    //The second parameter is of type sembuf [], representing an array
    //The third parameter represents the size of the array represented by the second parameter
    if(semop(semSetId,&sb,1) < 0){
        perror("sigSem failed");
        exit(1);
    }
}


int init(int semid, union semun su, int entries){
    //Semaphore creation
    //The producer sem: the amount of synchronous semaphore, indicating the order, there must be space to produce
    //The consumer sem: the amount of synchronization signal, indicating the order, and there must be products to consume
    //The global sem (between producer & consumer): mutually exclusive semaphore, the each producer and consumer cannot enter the shared memory at the same time
    if((semid = semget(IPC_PRIVATE,3,SEM_MODE)) < 0)
    {
        perror("create semaphore failed");
        exit(1);
    }
    su.val = entries;//How many products can the warehouse receive
    if(semctl(semid,0,SETVAL, su) < 0){
        perror("producer semctl failed");
        exit(1);
    }
    //Semaphore initialization, where su means union semun 
    su.val = 1;//No products currently
    if(semctl(semid,1,SETVAL,su) < 0){
        perror("consumer semctl failed");
        exit(1);
    }
    su.val = 1;//Can enter the buffer when it is 1
    if(semctl(semid,2,SETVAL,su) < 0){
        perror("global semaphore semctl failed");
        exit(1);
    }
    return semid;
}
