/* semtest3.c: POSIX Semaphore test example using shared memory */

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "shared_memory.h"
#include <signal.h>
#include <time.h>

#define SEGMENTPERM 0666

int main(int argc, char **argv){
    char* input;
    if(argc!=3){
        printf("Invalid call\n");
        return 0;
    }else if(strcmp(argv[1],"-l")==0){
        input=argv[2];
    }else{
        printf("Invalid flag\n");
        return 0;
    }


    FILE *infile = fopen(input, "r");
    if(infile==NULL){
      printf("Give a valid input\n");
      return 0;
    }
    
    
    int number_buses;
    int bay_number;
    int bay_cap_ASK;
    int bay_cap_VOR;
    int bay_cap_PEL;
    int capacity;
    int parkperiod;
    int mantime;
    int time1;
    int stattime;
    int incpassengers;

    char*token;
    
    char line[120];
    
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    number_buses=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    bay_number=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    bay_cap_ASK=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    bay_cap_VOR=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    bay_cap_PEL=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    capacity=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    parkperiod=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    mantime=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    time1=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    stattime=atoi(token);
    fgets(line, sizeof(line), infile);
    token=strtok(line," \n");
    incpassengers=atoi(token);
    
    
    printf("%d\n",number_buses);
    printf("%d\n",bay_number);
    printf("%d\n",bay_cap_ASK);
    printf("%d\n",bay_cap_VOR);
    printf("%d\n",bay_cap_PEL);
    printf("%d\n",capacity);
    printf("%d\n",parkperiod);
    printf("%d\n",mantime);
    printf("%d\n",time1);
    printf("%d\n",stattime);
    printf("%d\n",incpassengers);

    fclose(infile);
    
    

    struct sh_memory *shared;
    int num_of_buses=number_buses;
    
    char* type=NULL;
    int pid[num_of_buses];
    int shared_id;
    int err;
    int my_st;
    int comptroller;
    shared_id = shmget(IPC_PRIVATE, sizeof(*shared), SEGMENTPERM);
	if (shared_id == -1) perror("Creation");
	else printf("Allocated %d\n", shared_id);

    shared =shmat(shared_id,NULL, 0);
    if (shared == (void *) -1) { perror("Attachment."); exit(2);}

	//Den yparxoyn mutex afoy mexri edw o mystation
    //einai h monadikh diergasia poy trexei

    shared->passengers_out=0;
    shared->passengers_in=0;
    shared->total_buses_come=0;
    shared->total_buses_left=0;
    shared->ASK_buses_come=0;
    shared->VOR_buses_come=0;
    shared->PEL_buses_come=0;
    shared->waiting_time=0.0;
    shared->waiting_time_ASK=0.0;
    shared->waiting_time_PEL=0.0;
    shared->waiting_time_VOR=0.0;

    
    shared->ASK_bay_cap=bay_cap_ASK*bay_number;
    shared->VOR_bay_cap=bay_cap_VOR*bay_number;
    shared->PEL_bay_cap=bay_cap_PEL*bay_number;
    shared->buses_in_ASK=0;
    shared->buses_in_VOR=0;
    shared->buses_in_PEL=0;

    printf("\n/////...COMTROLLER UPDATE(Station Condition)...////////\n");
    printf("Passengers arrived in the station: %d \n",shared->passengers_out);
    printf("Buses parked in the station: %d\n",(shared->total_buses_come)-(shared->total_buses_left));
    printf("Free spaces in ASK: %d\n",(shared->ASK_bay_cap)-(shared->buses_in_ASK));
    printf("Free spaces in VOR: %d\n",(shared->VOR_bay_cap)-(shared->buses_in_VOR));
    printf("Free spaces in PEL: %d\n",(shared->PEL_bay_cap)-(shared->buses_in_PEL));
    printf("/////////////////////////////////////////////////////////\n\n");

    if (sem_init(&(shared->mutex_file),1,1) != 0) {
      perror("Couldn't initialize.");
      exit(3);
    }

    sem_wait(&(shared->mutex_file));
        FILE * fpb=fopen("public_ledger","w");
        fclose(fpb);
    sem_post(&(shared->mutex_file));

    printf("Buses in: %d\n",(shared->total_buses_come)-(shared->total_buses_left));

    if(my_st=fork()==0){
        char *arg[4];
        arg[0] = "./station-manager";
        char str[12];
        sprintf(str, "%d", shared_id);
        arg[1] = "-s";
        arg[2] = str;
        arg[3] = NULL;
        execvp(arg[0],arg);
        
    }else{
        if(comptroller=fork()==0){
            char *arg1[8];
            arg1[0] = "./comptroller";
            char str[12];
            sprintf(str, "%d", shared_id);
            char str1[12];
            sprintf(str1, "%d", time1);
            char str2[12];
            sprintf(str2, "%d", stattime);
            arg1[1] = "-d";
            arg1[2] = str1;
            arg1[3] = "-t";
            arg1[4] = str2;
            arg1[5] = "-s";
            arg1[6] = str;
            arg1[7] = NULL;
            execvp(arg1[0],arg1);
        }else{
            sleep(1);
            int i;
            srand(time(0));
            for(i = 0; i < num_of_buses; i++){
                int f=rand();
                pid[i] = fork();
                if(pid[i] < 0){
                    printf("Error!\n");
                    exit(1);
                }
                if(pid[i] == 0){
                    if(f%3==0){
                        type="ASK";
                    }else if(f%3==1){
                        type="VOR";
                    }else if(f%3==2){
                        type="PEL";
                    }
                    char str1[12];
                    sprintf(str1, "%d", f%incpassengers);
                    char str2[12];
                    sprintf(str2, "%d", capacity);
                    char str3[12];
                    sprintf(str3, "%d", parkperiod);
                    char str4[12];
                    sprintf(str4, "%d", mantime);
                    /* Child processes */
                    /* Structs for semaphor */
                    char *arg[14];
                    arg[0] = "./bus";
                    char str[12];
                    sprintf(str, "%d", shared_id);
                    
                    arg[1] = "-t";
                    arg[2] = type;
                    arg[3] = "-n";
                    arg[4] = str1;
                    arg[5] = "-c";
                    arg[6] = str2;
                    arg[7] = "-p";
                    arg[8] = str3;
                    arg[9] = "-m";
                    arg[10] = str4;
                    arg[11] = "-s";
                    arg[12] = str;
                    arg[13] = NULL;
                    execvp("./bus",arg);

                    exit(0);
                }
            }

                /* Wait for childs. */
            for(i = 0; i < num_of_buses; i++){
                waitpid(pid[i], NULL, 0);
            }


            printf("ALL starting buses served\n"); //Eidopoihsh gia exiphrethsh olwn twn leoforeiwn
            
            getchar();                      //perimenei na pathsei o xrhsthw enter gia na termatisei
                                            //Dinetai etsis to xrhsth h dinatothta na dhmiourghsei kai alla bus mesw enow allou termatikoy                    
            kill(my_st,SIGUSR2);            //shma termatismoy toy manager
            kill(comptroller,SIGUSR2);      //shma termatismoy toy comtroller

            //epishs se ayto to shmeio exoyn termatisei
            //oles oi alles diergasies

            sem_destroy(&(shared->enter));
            sem_destroy(&(shared->exit));
            sem_destroy(&(shared->mutex_bay));
            sem_destroy(&(shared->mutex));

            err = shmctl(shared_id, IPC_RMID, 0);
            if (err == -1) perror("Removal.");
            else printf("Removed. %d\n",err);
        }
    }
}
