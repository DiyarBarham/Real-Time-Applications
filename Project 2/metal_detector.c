#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

// Structure for message queue
struct msgbuf {
    pid_t child_id;
    int end;
};

pid_t *shmptr_hall;
size_t *shmptr_hall_size;
int *shmptr_mutix;

void mysig(int sig){
    if(shmdt(shmptr_hall) == -1){
        perror("metal_detector: shmptr_hall failed");
        exit(1);
    }
    if(shmdt(shmptr_hall_size) == -1){
        perror("metal_detector: shmptr_hall_size failed");
        exit(1);
    }
    if(shmdt(shmptr_mutix) == -1){
        perror("metal_detector: shmptr_mutix failed");
        exit(1);
    }

    exit(0);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
int main(int argc, char *argv[]) {
    int msgidmetal = strtol(argv[1], 0,0);
    int shmid_hall = strtol(argv[2], 0,0);
    int shmid_hall_size = strtol(argv[3], 0,0);
    int shmid_mutix = strtol(argv[4], 0,0);
    int hall_size = strtol(argv[5], 0,0);

    signal(SIGTERM, mysig);
    
    // Attach shared memory segment to the process' address space
    shmptr_hall = (pid_t *)shmat(shmid_hall, NULL, 0);
    if (shmptr_hall == (pid_t *)-1) {
        perror("metal2-> shmat failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    shmptr_hall_size = (size_t *)shmat(shmid_hall_size, NULL, 0);
    if (shmptr_hall_size == (size_t *)-1) {
        perror("metal3-> shmat failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    shmptr_mutix = (int *)shmat(shmid_mutix, NULL, 0);
    if (*shmptr_mutix == -1) {
        perror("metal4-> shmat failed");
        exit(1);
    }

    pid_t child_pids[hall_size];
    int queue_size = 0;


    int h=0;
    while(h ==0){
        if(*shmptr_hall_size<hall_size){
            struct msgbuf message;
            msgrcv(msgidmetal, &message, 1, 0, 0);
            if (message.end == 1){
                h=1;
            }
            //sleep(2);
            if (*shmptr_mutix == 0){
                for (int i = 0; i < queue_size; ++i) {
                    if (child_pids[i] != -123){
                        if (kill(child_pids[i], 0) ==0){
                            shmptr_hall[*shmptr_hall_size] = child_pids[i];
                            *shmptr_hall_size = *shmptr_hall_size +1;
                        }
                    }

                    child_pids[i] = -1;
                }
                queue_size =0;
                if (message.child_id != -123){
                    if (kill(message.child_id, 0) ==0){
                        shmptr_hall[*shmptr_hall_size] = message.child_id;
                        *shmptr_hall_size = *shmptr_hall_size + 1;
                    }
                }
                *shmptr_mutix= 1;
            } else{
                if (message.child_id != -123){
                    if (kill(message.child_id, 0) ==0){
                        child_pids[queue_size] = message.child_id;
                        queue_size++;
                    }
                }


            }

        }
        sleep(1);
    }
    while (shmptr_mutix != 0){

    }
    for (int i = 0; i < queue_size; ++i) {
        if (child_pids[i] != -123){
            if (kill(child_pids[i], 0) ==0){
                shmptr_hall[*shmptr_hall_size] = child_pids[i];
                *shmptr_hall_size = *shmptr_hall_size + 1;
            }
        }
        child_pids[i] = -1;
    }
    queue_size =0;
    shmptr_hall[*shmptr_hall_size] = -123;
    *shmptr_hall_size = *shmptr_hall_size + 1;
    *shmptr_mutix=1;

    if(shmdt(shmptr_hall) == -1){
        perror("metal_detector: shmptr_hall failed");
        exit(1);
    }
    if(shmdt(shmptr_hall_size) == -1){
        perror("metal_detector: shmptr_hall_size failed");
        exit(1);
    }
    if(shmdt(shmptr_mutix) == -1){
        perror("metal_detector: shmptr_mutix failed");
        exit(1);
    }

    exit(0);
}
