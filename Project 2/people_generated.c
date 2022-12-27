#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>

void mysig(int sig){
    exit(0);
}

int main(int argc, char *argv[]) {
    int num_people = strtol(argv[0], 0,0);
    int msgid = strtol(argv[1],0,0);
    int pipechild = strtol(argv[2],0,0);
    int msgidch = strtol(argv[3],0,0);
    
    signal(SIGTERM, mysig);

    for (int i = 0; i < num_people; i++) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            char person_id_str[16];
            sprintf(person_id_str, "%d", msgid);
            char pc[50];
            sprintf(pc, "%d", pipechild);

            char msgidchild[50];
            sprintf(msgidchild, "%d", msgidch);

            char numofp[50];
            sprintf(numofp, "%d", num_people);
            execlp("./person", numofp, person_id_str, pc, msgidchild, NULL);
            perror("execlp failed");
            exit(1);
        } else{
            //sleep(1);
        }
    }
    
    exit(0);
}