#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct sh_memory{
    sem_t enter;
    sem_t exit;
    // sem_t entered;
    // sem_t exited;

    sem_t mutex;
    int passengers_out;
    int passengers_in;
    int curr_passengers;
    int total_buses_come;
    int ASK_buses_come;
    int VOR_buses_come;
    int PEL_buses_come;
    int total_buses_left;
    double waiting_time;
    double waiting_time_ASK;
    double waiting_time_VOR;
    double waiting_time_PEL;

    sem_t mutex_st;
    int entering;
    int exiting;

    sem_t mutex_bay;
    int ASK_bay_cap;
    int buses_in_ASK;
    int VOR_bay_cap;
    int buses_in_VOR;
    int PEL_bay_cap;
    int buses_in_PEL;

    sem_t mutex_file;

};