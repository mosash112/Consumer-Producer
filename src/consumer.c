#include "common.h"

void consume(seg* comm){
    printComm(comm);
    shmp->start = (shmp->start+sizeof(seg));
}


key_t control(int old){
    int shmflg = SHM_MODE|IPC_CREAT, shmid;
    size_t size = sizeof(queue);
    key_t key;

    key = ftok("control",1);
    shmid = shmget(key,size,shmflg);
    queue* q = (queue*) shmat(shmid,(void*)0,shmflg);
    if (q == (void *) -1) {
        perror("Shared memory attach");
        exit(1);
    }
    if (old == 0){
        // q->start = q->end;
        shmp->end = q->end;
        shmp->start = q->start;
        key = IPC_PRIVATE;
    }else if (old == 1){
        if (q->start == q->end){
            printf("Queue is empty...");
            sleep(500);
        }else if(q->start == MAX_MEM){
            q->start = 1;            
        }else{
            q->start = shmp->start;
        }
        key = ftok("",shmp->start);
    }
    shmdt(q);
    // printQueue(shmp, "shmp");
    return key;
}


int main(int argc, char* argv[])
{
    if (argc != 2){
        printf( "wrong syntax.\nusage: %s entriesNumber", argv[0] );
    }else{
        int nap, semid, shmflg = SHM_MODE|IPC_CREAT, shmid, entries = atoi(argv[1]);
        size_t size = sizeof(seg);
        union semun su;     //sem union, used to initialize the semaphore
        shmp = (queue*)malloc(sizeof(queue));
        key_t key;
        // shmp->start = shmp->end = 0;

        key = control(0);
        semid = init(semid,su, entries);

        // while (1)
        // {
            waitSem(semid,1);//Must have products to consume
            waitSem(semid,2);//Lock buffer

            int start = shmp->start, end = shmp->end;
            while (start!=end){    
                shmid = shmget(key,size,shmflg);
                seg* mem= (seg*) shmat(shmid,(void*)0,shmflg);
                consume(mem);
                shmdt(mem);
                // shmctl(shmid,IPC_RMID,NULL);
                start = start+sizeof(seg);
            }
            // shmp->start = start;

            sigSem(semid,2);//Release buffer
            sigSem(semid,0);//Inform producers that there is room
            // sleep(2);//Consumption frequency
            key = control(1);
        // }
    }
    return 0;
}