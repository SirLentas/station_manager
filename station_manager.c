#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "shared_memory.h"
#include <signal.h>

#define SEGMENTPERM 0666

int loop=1;

void sigquit()
{ printf("Station Manager says: My Station ended my service!!!\n");
  loop=0;
}

int main(int argc, char **argv){
    struct sh_memory *shared;
    int shared_id;
    sscanf(argv[2], "%d", &shared_id);
	  printf("Allocated %d\n", shared_id);

    shared =shmat(shared_id,NULL, 0);
    int in=0;
    int out=0;
    if (shared == (void *) -1) { perror("Attachment."); exit(2);}

    if (sem_init(&(shared->enter),1,0) != 0) {
      perror("Couldn't initialize.");
      exit(3);
    }

    if (sem_init(&(shared->exit),1,0) != 0) {
      perror("Couldn't initialize.");
      exit(3);
    }

    if (sem_init(&(shared->mutex),1,1) != 0) {
      perror("Couldn't initialize.");
      exit(3);
    }
    
    if (sem_init(&(shared->mutex_st),1,1) != 0) {
      perror("Couldn't initialize.");
      exit(3);
    }

    int parked;
    while(loop){     
        signal(SIGUSR2, sigquit);  
        
        sem_wait(&(shared->mutex_st));

        if(shared->entering==0){            //efoswn den yparxei kinoymeno lewforeio mesa sto stathmo dinei egkrish sto epomeno gia eisodo
          sem_post(&(shared->enter));
          shared->entering=1;
        }

        if(shared->exiting==0){              //efoswn den yparxei kinoymeno lewforeio mesa sto stathmo dinei egkrish sto epomeno gia exodo
          sem_post(&(shared->exit));
          shared->exiting=1;
        }
        

        sem_post(&(shared->mutex_st));
    }  
    
    shmdt(shared);
}
