#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>

#define MY_SIGNAL2 (SIGUSR2)
pid_t *shmptr_offices;
void mysig(int sig){
    if(shmdt(shmptr_offices) == -1){
        perror("teller: shmptr_offices failed");
        exit(1);
    }

    exit(0);
}
int main(int argc, char *argv[]) {
    int shmid_offices = strtol(argv[1], 0,0);
    int id = strtol(argv[2], 0,0);
    signal(SIGTERM, mysig);
    // Attach shared memory segment to the process' address space
    shmptr_offices = (pid_t *)shmat(shmid_offices, NULL, 0);
    if (shmptr_offices == (pid_t *)-1) {
        perror("teller1-> shmat failed");
        exit(1);
    }

    while (shmptr_offices[id] != -123){
        if (shmptr_offices[id] != 0  && shmptr_offices[id] != -123 ){
            sleep(5);
            kill(shmptr_offices[id], MY_SIGNAL2);
            shmptr_offices[id]=0;
        }
    }

    if(shmdt(shmptr_offices) == -1){
        perror("teller: shmptr_offices failed");
        exit(1);
    }

    exit(0);
}