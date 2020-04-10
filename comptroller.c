#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "shared_memory.h"
#include <sys/time.h>
#include <string.h>
#include <signal.h>

#define SEGMENTPERM 0666

int loop=1;

void sigquit()
{ printf("Comptroller says: My Station ended my service!!!\n");
  loop=0;
}

int main(int argc, char **argv){
    char * time;
    char * stattimes;
    char * shmid;

    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],"-d")==0){
            time=argv[i+1];
        }
        if(strcmp(argv[i],"-t")==0){
            stattimes=argv[i+1];
        }
        if(strcmp(argv[i],"-s")==0){
            shmid=argv[i+1];
        }
    }

    struct sh_memory *shared;
    int shared_id;
    
    sscanf(shmid, "%d", &shared_id);
	printf("Comptroller allocated %d\n", shared_id);

    shared =shmat(shared_id,NULL, 0);
    if (shared == (void *) -1) { perror("Attachment."); exit(2);}

    if (fork()==0){  //dhmiourgoumai mia 2h diergasia pou trexei arallhla mexri na termatistoyn aki oi 2 apo to mystation
        while(loop){ 
            sem_wait(&(shared->mutex));
            printf("\n/////...COMTROLLER UPDATE(Station Stats)...////////////\n");
            printf("Passengers arrived in the station: %d \n",shared->passengers_out);
            printf("Passengers left from the station: %d \n",shared->passengers_in);
            printf("Buses arrived in the station: %d \n",shared->total_buses_come);
            printf("Buses fully serviced by the station: %d \n",shared->total_buses_left);
            printf("Average waiting time: %g ms \n", (double)(shared->waiting_time)/(shared->total_buses_come));
            printf("Average waiting time for ASK: %g ms \n", (double)(shared->waiting_time_ASK)/(shared->ASK_buses_come));
            printf("Average waiting time for VOR: %g ms \n", (double)(shared->waiting_time_VOR)/(shared->VOR_buses_come));
            printf("Average waiting time for PEL: %g ms \n", (double)(shared->waiting_time_PEL)/(shared->PEL_buses_come));
            printf("/////////////////////////////////////////////////////////\n\n");
            sem_post(&(shared->mutex));
            signal(SIGUSR2, sigquit); //o termatismos ginetai mesw enos shmatos poy stelnei o mystation
            sleep(atoi(stattimes));
        }
        shmdt(shared);
    }else{
        while(loop){ 
            sem_wait(&(shared->mutex));
            printf("\n/////...COMTROLLER UPDATE(Station Condition)...////////\n");
            printf("Passengers in the station: %d \n",shared->curr_passengers);
            printf("Buses parked in the station: %d\n",(shared->total_buses_come)-(shared->total_buses_left));
            printf("Free spaces in ASK: %d\n",(shared->ASK_bay_cap)-(shared->buses_in_ASK));
            printf("Free spaces in VOR: %d\n",(shared->VOR_bay_cap)-(shared->buses_in_VOR));
            printf("Free spaces in PEL: %d\n",(shared->PEL_bay_cap)-(shared->buses_in_PEL));
            printf("/////////////////////////////////////////////////////////\n\n");
            sem_post(&(shared->mutex));
            signal(SIGUSR2, sigquit); 
            sleep(atoi(time));
        }
        shmdt(shared);
    }
}