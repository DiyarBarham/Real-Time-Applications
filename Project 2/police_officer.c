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

void mysig(int sig){
    exit(0);
}

int main(int argc, char *argv[]) {
    // Get the shared memory segment ID from the command line argument
    int msgidmale = strtol(argv[1], 0,0);
    int msgidfemale = strtol(argv[2], 0,0);
    int msgidmetal = strtol(argv[3], 0,0);
    int hallsize = strtol(argv[4], 0,0);

    signal(SIGTERM, mysig);

    srand(time(0) + getpid());
    int male=0, female=0;
    while (male==0 || female == 0){
        struct msqid_ds buf;
        msgctl(msgidmetal, IPC_STAT, &buf);
        if(buf.msg_qnum < hallsize){
            int r = rand() % 2;
            struct msgbuf message;
            if(r==0 && male==0){
                msgrcv(msgidmale, &message, 1, 0, 0);
                if (message.end == 1){
                    male=1;
                }
            } else if(r==1 && female == 0){
                msgrcv(msgidfemale, &message, 1, 0, 0);
                if (message.end == 1){
                    female=1;
                }
            } else if (male ==1 && r==0){
                msgrcv(msgidfemale, &message, 1, 0, 0);
                if (message.end == 1){
                    female=1;
                }
            } else if (female ==1 && r==1){
                msgrcv(msgidmale, &message, 1, 0, 0);
                if (message.end == 1){
                    male=1;
                }
            }
            if (message.child_id ==-123){
                if(male==1 && female == 1){
                    message.end = 1;
                    msgsnd(msgidmetal, &message, 1, 0);
                }
            } else{
                if (kill(message.child_id, 0) ==0){
                    if(male==1 && female == 1){
                        message.end = 1;
                        msgsnd(msgidmetal, &message, 1, 0);
                    } else{
                        message.end = 0;
                        msgsnd(msgidmetal, &message, 1, 0);
                    }
                } else{
                    if(male==1 && female == 1){
                        message.end = 1;
                        message.child_id = -123;
                        msgsnd(msgidmetal, &message, 1, 0);
                    }
                }

            }

            srand(time(0) + message.child_id);
        }
        sleep(1);

    }

    exit(0);
}