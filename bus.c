#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "shared_memory.h"
#include <sys/time.h>
#include <time.h>
#include <string.h>

#define SEGMENTPERM 0666

int main(int argc, char **argv){
    char* type=NULL;
    char* inc_passengers=NULL;
    char* capacity=NULL;
    char* parkperiod=NULL;
    char* mantime=NULL;
    char* shmid=NULL;

    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],"-t")==0){
            type=argv[i+1];
        }
        if(strcmp(argv[i],"-n")==0){
            inc_passengers=argv[i+1];
        }
        if(strcmp(argv[i],"-c")==0){
            capacity=argv[i+1];
        }
        if(strcmp(argv[i],"-p")==0){
            parkperiod=argv[i+1];
        }
        if(strcmp(argv[i],"-m")==0){
            mantime=argv[i+1];
        }
        if(strcmp(argv[i],"-s")==0){
            shmid=argv[i+1];
        }
    }
    struct timeval t1, t2, t3, t4;
    double elapsedTime;
    struct sh_memory *shared;
    int shared_id;
    int parked;
    int passengers = atoi(inc_passengers);  //oi epivates poy tha apovivastoyne einai osoi leei to flag -n
    char * position;
    FILE * fpb;
    sscanf(shmid, "%d", &shared_id);
	printf("Bus with id: %d allocated %d\n",getpid(), shared_id);

    shared =shmat(shared_id,NULL, 0);
    if (shared == (void *) -1) { perror("Attachment."); exit(2);}
    
    if (sem_init(&(shared->mutex_bay),1,1) != 0) {
        perror("Couldn't initialize.");
        exit(3);
    }

    
    //PARKING START
    
    sem_wait(&(shared->enter));                     //anamonh gia eidopoihsh apo ton sattion manager gia sfalh eisodo
    sem_wait(&(shared->mutex_file));                //Perimenei th seira toy gia na apokteisei prosbash sto arxeio
        gettimeofday(&t4, NULL);                    //wra eisodou ston stathmo
        fpb=fopen("public_ledger","a");
        fprintf(fpb,"Bus with id %d from [%s] get the green light to enter the station at %ld.%06ld\n",getpid(),type,t4.tv_sec,t4.tv_usec);
        fclose(fpb);
    sem_post(&(shared->mutex_file));
        if(strcmp(type,"ASK")==0){                  //To leoforeio afoy labei to prasino fvw apo ton manager gia eisodo
           sem_wait(&(shared->mutex_bay));          //parkarei sto katallilo bay me bash thn prwteraiothta toy
                parked=0;
                do{
                    if(shared->buses_in_ASK<shared->ASK_bay_cap){
                        (shared->buses_in_ASK)++;
                        position="ASK";
                        parked=1;
                    }else if(shared->buses_in_PEL<shared->PEL_bay_cap){
                        (shared->buses_in_PEL)++;
                        position="PEL";
                        parked=1;
                    }
                }while(parked==0);                  //To loop ekteleitai mexri na adeiasei kapoia thesei se ena apo ta 2 bay
            sem_post(&(shared->mutex_bay)); 
        }else if(strcmp(type,"VOR")==0){
            sem_wait(&(shared->mutex_bay));
                parked=0;
                do{
                    if(shared->buses_in_VOR<shared->VOR_bay_cap){
                        (shared->buses_in_VOR)++;
                        position="VOR";
                        parked=1;
                    }else if(shared->buses_in_PEL<shared->PEL_bay_cap){
                        (shared->buses_in_PEL)++;
                        position="PEL";
                        parked=1;
                    }
                }while(parked==0);
            sem_post(&(shared->mutex_bay)); 
        }else{
            sem_wait(&(shared->mutex_bay));
                parked=0;
                do{
                    if(shared->buses_in_PEL<shared->PEL_bay_cap){
                        (shared->buses_in_PEL)++;
                        position="PEL";
                        parked=1;
                    }
                }while(parked==0);
            sem_post(&(shared->mutex_bay)); 
        }
        sleep(atoi(mantime));                       //mantime
        sem_wait(&(shared->mutex_st));              //Enhmerwsh shared memory oti ta eiserxomena lewforeia einai pali 0
            shared->entering=0;
        sem_post(&(shared->mutex_st));
    
    gettimeofday(&t1, NULL);


    sem_wait(&(shared->mutex_file));
        gettimeofday(&t3, NULL);
        fpb=fopen("public_ledger","a");
        fprintf(fpb,"Bus with id %d from [%s] has parked in [%s] bay at %ld.%06ld\n",getpid(),type,position,t3.tv_sec,t3.tv_usec);
        fclose(fpb);
    sem_post(&(shared->mutex_file));
    
    //PARKING END

    //TIME IN STATION START
    srand(time(0));
    int f=rand();
    int passengers_r= f%(atoi(capacity));               //oi epivates poy tha epivivastoyn einai enaw tyxaios arithmos
                                                        //apo to 0 ews to Capacity toy lewforeiou
    sem_wait(&(shared->mutex));
         if(strcmp(type,"ASK")==0){
            (shared->ASK_buses_come)++;
         }else if(strcmp(type,"VOR")==0){
            (shared->VOR_buses_come)++;
         }else{
            (shared->PEL_buses_come)++;
         }
         
        shared->passengers_out += passengers;   //apovivash epivatwn
	shared->curr_passengers += passengers;	//epivates pou vriskontai sto stathmo
        shared->passengers_in += passengers_r;  //epivivash epivatwn
        (shared->total_buses_come)++;

        sem_wait(&(shared->mutex_file));
            gettimeofday(&t3, NULL);
            fpb=fopen("public_ledger","a");
            fprintf(fpb,"Bus with id %d : %d passengers get off\n",getpid(),passengers);
            fprintf(fpb,"Bus with id %d : %d passengers get on\n",getpid(),passengers_r);
            fclose(fpb);
        sem_post(&(shared->mutex_file));
    sem_post(&(shared->mutex));
    sleep(atoi(parkperiod));

    //TIME IN STATION END

    //LEAVING START
    
    sem_wait(&(shared->exit));
    
    gettimeofday(&t2, NULL);
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    sem_wait(&(shared->mutex_file));
        fpb=fopen("public_ledger","a");
        fprintf(fpb,"Bus with id %d from [%s] get the green light to exit from bay [%s] at %ld.%06ld\n",getpid(),type,position,t2.tv_sec,t2.tv_usec);
        fclose(fpb);
    sem_post(&(shared->mutex_file));
    //sem_post(&(shared->exit));
    sleep(atoi(mantime)); //mantime
    sem_wait(&(shared->mutex_file));
        gettimeofday(&t2, NULL);
        fpb=fopen("public_ledger","a");
        fprintf(fpb,"Bus with id %d from [%s] left the station at %ld.%06ld\n",getpid(),type,t2.tv_sec,t2.tv_usec);
        fclose(fpb);
    sem_post(&(shared->mutex_file));
    //sem_post(&(shared->exited));
   

    sem_wait(&(shared->mutex));             
        (shared->total_buses_left)++;
	shared->curr_passengers -= passengers;
        if(strcmp(position,"ASK")==0){                      //enhmerwsh shared memory gia statistika poy exoyn na kanoyn me
            (shared->buses_in_ASK)--;                       //th thesi toy lewforeioy sto stathmo
        }else if(strcmp(position,"VOR")==0){
            (shared->buses_in_VOR)--;
        }else if(strcmp(position,"PEL")==0){
            (shared->buses_in_PEL)--;
        }
        if(strcmp(type,"ASK")==0){
            shared->waiting_time_ASK += elapsedTime;        //enhmerwsh shared memory gia statistika poy exoyn na kanoyn me
        }else if(strcmp(type,"VOR")==0){                    //ton typo tou lewforeioy 
            shared->waiting_time_VOR += elapsedTime;
        }else if(strcmp(type,"PEL")==0){
            shared->waiting_time_PEL += elapsedTime;
        }
        shared->waiting_time += elapsedTime;
    sem_post(&(shared->mutex));

    sleep(atoi(mantime));

    sem_wait(&(shared->mutex_st));
        shared->exiting=0;                  //Enhmerwsh shared memory oti ta exerxomena lewforeia einai pali 0
    sem_post(&(shared->mutex_st));

    //LEAVING END

    shmdt(shared);
}