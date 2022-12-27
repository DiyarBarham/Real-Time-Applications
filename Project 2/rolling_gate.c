#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Structure for message queue
struct msgbuf {
    pid_t child_id;
    int end;
};

pid_t *shmptr;
size_t *shmptr_rolling;

void mysig(int sig){
    if(shmdt(shmptr) == -1){
        perror("rolling_gate: shmptr failed");
        exit(1);
    }
    if(shmdt(shmptr_rolling) == -1){
        perror("rolling_gate: shmptr_rolling failed");
        exit(1);
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    // Get the shared memory segment ID from the command line argument
    int msgid = strtol(argv[1], 0,0);
    int shmid = strtol(argv[2], 0,0);
    size_t size = strtol(argv[3], 0,0);
    int gate_size = strtol(argv[4], 0,0);
    int shmid_total = strtol(argv[5], 0,0);
    
    signal(SIGTERM, mysig);
    // Attach shared memory segment to the process' address space
    shmptr = (pid_t *)shmat(shmid, NULL, 0);
    if (shmptr == (pid_t *)-1) {
        perror("rolling1-> shmat failed");
        exit(1);
    }

    shmptr_rolling = (size_t *)shmat(shmid_total, NULL, 0);
    if (shmptr_rolling == (size_t *)-1) {
        perror("rolling1-> shmat failed");
        exit(1);
    }
    *shmptr_rolling = 0;
    int i=0;
    while (i< size-1) {
        struct msqid_ds buf;
        msgctl(msgid, IPC_STAT, &buf);
        *shmptr_rolling = buf.msg_qnum;
        if(buf.msg_qnum < gate_size){
            if (kill(shmptr[i], 0) ==0){
                struct msgbuf message;
                message.child_id = shmptr[i];
                message.end=0;
                msgsnd(msgid, &message, 1, 0);
            }

            i = i +1;
        }

        sleep(1);
    }
    while (1){
        struct msqid_ds buf;
        msgctl(msgid, IPC_STAT, &buf);
        if(buf.msg_qnum < gate_size){
            struct msgbuf message;
            if (kill(shmptr[i], 0) ==0){
                message.child_id = shmptr[size-1];
            } else {
                message.child_id = -123;
            }
            message.end=1;
            msgsnd(msgid, &message, 1, 0);
            sleep(1);
            break;
        }
    }

    if(shmdt(shmptr) == -1){
        perror("rolling_gate: shmptr failed");
        exit(1);
    }
    if(shmdt(shmptr_rolling) == -1){
        perror("rolling_gate: shmptr_rolling failed");
        exit(1);
    }

    exit(0);
}
