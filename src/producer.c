#include "common.h"

// typedef struct state{
//     int shmid;
//     seg shmseg;
//     int shmflg;
// } state;
// state ap[MAXnap+1];


double randnum(){
    return ( (double)(rand()) + 1. )/( (double)(RAND_MAX) + 1. );
}


double normDist(double Mu, double sigma){
    double v1= randnum();
    double v2= randnum();
    return cos(2*3.14*v2)*sqrt(-2.*log(v1))*sigma+Mu;
}


void produce(seg* comm, char* commName, double Mu, double dev, int entries){
    strcpy(comm->commName,commName);
    comm->commPrice = normDist(Mu,dev);
    printComm(comm);
    shmp->end = (shmp->end+sizeof(seg));
}


void control(int old){
    int shmflg = SHM_MODE|IPC_CREAT, shmid;
    size_t size = sizeof(queue);

    key_t key = ftok("control",1);
    shmid = shmget(key,size,shmflg);
    queue* q = (queue*) shmat(shmid,(void*)0,shmflg);
    if (q == (void *) -1) {
        perror("Shared memory attach");
        exit(1);
    }
    if (old == 0){
        shmp->end = q->end;
        shmp->start = q->start;
    }else if (old == 1){
        if (q->end == MAX_MEM && q->start != 1){
            q->end = 1;
        }else if(q->start == 0){
            q->end = q->start = 1;
        }else{
            q->end = shmp->end;
        }
    }
    shmdt(q);
    // printQueue(shmp, "shmp");
}


int main(int argc, char *argv[])
{
    if (argc != 6){
        printf( "wrong syntax.\nusage: %s commodityName commodityPriceMean commodityPriceDeviation sleepInterval entriesNumber", argv[0] );
    }else{
        double sigma = atof(argv[3]), Mu = atof(argv[2]), sleepTime = atof(argv[4])/1000;
        int semid, shmflg = SHM_MODE|IPC_CREAT, shmid, entries = atoi(argv[5]), id = 1;
        char *name = argv[1];
        union semun su;     //sem union, used to initialize the semaphore
        size_t size = sizeof(seg);
        shmp = (queue*)malloc(sizeof(queue));
    
        srand(time(0));
        control(0);
        semid = init(semid,su,entries);

        while(1){
          printf("\nentry number %d\n", id);
        //   key_t key = ftok(name,shmp->end);
          shmid = shmget(IPC_PRIVATE,size,shmflg);
          if(shmid < 0){
              perror("getting shared memory failed");
              exit(1);
          }
  
          seg* mem = (seg*) shmat(shmid,(void*)0,shmflg);
          if (mem == (void *) -1) {
            perror("attaching shared memory failed");
            return 1;
          }
  
          waitSem(semid,0);//Get a space for storing products
          waitSem(semid,2);//Occupy product buffer
          produce(mem,argv[1],Mu, sigma,entries);
          sigSem(semid,2);//Release product buffer
          sleep(sleepTime);//Produce one every two seconds
          sigSem(semid,1);//Inform consumers that there is a product
          shmdt(mem);
          id++;
          control(1);
        }
    }
    return 0;
}
