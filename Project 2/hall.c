#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>


pid_t *shmptr_hall;
size_t *shmptr_hall_size;
pid_t *shmptr_offices;
int *shmptr_mutix;

void mysig(int sig){
    if(shmdt(shmptr_hall) == -1){
        perror("hall: shmptr_hall failed");
        exit(1);
    }
    if(shmdt(shmptr_hall_size) == -1){
        perror("hall: shmptr_hall_size failed");
        exit(1);
    }
    if(shmdt(shmptr_mutix) == -1){
        perror("hall: shmptr_mutix failed");
        exit(1);
    }

    if(shmdt(shmptr_offices) == -1){
        perror("hall: shmptr_offices failed");
        exit(1);
    }

    exit(0);
}

#define MY_SIGNAL (SIGUSR1)
#define MY_SIGNAL2 (SIGUSR2)

void change_order(int start);
int main(int argc, char *argv[]) {
    int shmid_hall = strtol(argv[1], 0,0);
    int shmid_hall_size = strtol(argv[2], 0,0);
    int shmid_offices = strtol(argv[3], 0,0);
    size_t num_each_type = strtol(argv[4], 0,0);
    int pp = strtol(argv[5], 0,0);
    int shmid_mutix = strtol(argv[6], 0,0);
    pid_t metal_pid = strtol(argv[7], 0,0);

    signal(SIGTERM, mysig);

    // Attach shared memory segment to the process' address space
    shmptr_hall = (pid_t *)shmat(shmid_hall, NULL, 0);
    if (shmptr_hall == (pid_t *)-1) {
        perror("hall1-> shmat failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    shmptr_hall_size = (size_t *)shmat(shmid_hall_size, NULL, 0);
    if (shmptr_hall_size == (size_t *)-1) {
        perror("hall2-> shmat failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    shmptr_offices = (pid_t *)shmat(shmid_offices, NULL, 0);
    if (shmptr_offices == (pid_t *)-1) {
        perror("hall3-> shmat failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    shmptr_mutix = (int *)shmat(shmid_mutix, NULL, 0);
    if (shmptr_mutix == (pid_t *)-1) {
        perror("hall4-> shmat failed");
        exit(1);
    }

    int h=0;
    while(shmptr_hall[0] != -123){
        if(*shmptr_mutix == 1){
            for (int i = 0; i < *shmptr_hall_size; ++i) {
                if (kill(shmptr_hall[i], 0) !=0){
                    change_order(i);
                    i--;
                    continue;
                }
                char temp[256];
                kill(shmptr_hall[i], MY_SIGNAL);
                read(pp, &temp, sizeof(temp));

                if(strcmp(temp, "birth certificates" ) == 0){
                    for (int j = 0; j < num_each_type; ++j) {
                        if (shmptr_offices[j] == 0){
                            shmptr_offices[j] = shmptr_hall[i];
                            kill(shmptr_hall[i], MY_SIGNAL2);
                            change_order(i);
                            i--;
                            break;
                        }
                    }
                } else if(strcmp(temp, "travel documents" ) == 0){
                    for (int j = num_each_type; j < num_each_type*2; ++j) {
                        if (shmptr_offices[j] == 0){
                            shmptr_offices[j] = shmptr_hall[i];
                            kill(shmptr_hall[i], MY_SIGNAL2);
                            change_order(i);
                            i--;
                            break;
                        }
                    }
                } else if(strcmp(temp, "family reunion" ) == 0){
                    for (int j = num_each_type*2; j < num_each_type*3; ++j) {
                        if (shmptr_offices[j] == 0){
                            shmptr_offices[j] = shmptr_hall[i];
                            kill(shmptr_hall[i], MY_SIGNAL2);
                            change_order(i);
                            i--;
                            break;
                        }
                    }
                } else if(strcmp(temp, "ID-related" ) == 0){
                    for (int j = num_each_type*3; j < num_each_type*4; ++j) {
                        if (shmptr_offices[j] == 0){
                            shmptr_offices[j] = shmptr_hall[i];
                            kill(shmptr_hall[i], MY_SIGNAL2);
                            change_order(i);
                            i--;
                            break;
                        }
                    }
                }

            }
            int j=0;
            for (int i = 0; i < *shmptr_hall_size; ++i) {
                if (kill(shmptr_hall[i], 0) !=0){
                    change_order(i);
                    i--;
                    continue;
                }
                while (j < num_each_type*4) {
                    
                    if (shmptr_offices[j] == 0){
                        shmptr_offices[j] = shmptr_hall[i];
                        kill(shmptr_hall[i], MY_SIGNAL2);
                        change_order(i);
                        i--;
                        j++;
                        break;
                    }
                    j++;
                }
            }
            if (kill(metal_pid, 0) ==0){
                *shmptr_mutix=0;
            }
        }
        sleep(1);


    }
    for (int i = 0; i < num_each_type*4; ++i) {
        while (shmptr_offices[i] != 0){
            sleep(1);
        }
        shmptr_offices[i] = -123;
    }

    if(shmdt(shmptr_hall) == -1){
        perror("hall: shmptr_hall failed");
        exit(1);
    }
    if(shmdt(shmptr_hall_size) == -1){
        perror("hall: shmptr_hall_size failed");
        exit(1);
    }
    if(shmdt(shmptr_mutix) == -1){
        perror("hall: shmptr_mutix failed");
        exit(1);
    }

    if(shmdt(shmptr_offices) == -1){
        perror("hall: shmptr_offices failed");
        exit(1);
    }

    exit(0);
}

void change_order(int start){
    for (int j = start; j < *shmptr_hall_size-1; ++j) {
        shmptr_hall[j] = shmptr_hall[j+1];
    }
    shmptr_hall[*shmptr_hall_size -1] = -1;
    *shmptr_hall_size = *shmptr_hall_size - 1;
}