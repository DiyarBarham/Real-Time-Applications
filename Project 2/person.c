#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/msg.h>

#define MY_SIGNAL (SIGUSR1)
#define MY_SIGNAL2 (SIGUSR2)

int should_exit_loop = 0;  // Global flag to indicate when the process should exit the loop
char request_types[4][256] = {
        "birth certificates",
        "travel documents",
        "family reunion",
        "ID-related"
};
void mysig(int sig){
    exit(0);
}
// Structure for message queue
struct msgbuf {
    pid_t child_id;
    int value;
};

// Structure for message queue
struct msgbuf2 {
    pid_t child_id;
    int gender;
};

void my_signal_handler(int sig);
void my_signal_handler2(int sig);
int main(int argc, char *argv[]) {
    int num_people = strtol(argv[0], 0,0);
    int msgidparent = strtol(argv[1],0,0);
    int pipechild = strtol(argv[2],0,0);
    int msgid = strtol(argv[3],0,0);

    signal(SIGTERM, mysig);
    
    // Register a signal handler for the custom signal
    signal(MY_SIGNAL, my_signal_handler);
    signal(MY_SIGNAL2, my_signal_handler2);
    srand(time(0) + getpid());
    int gender = rand() % 2;
    int patient_level = rand() % 100 +10;
    int current_patient = 0;

    int appearance_time = rand() % 4 + 5;
    char appearance_time_str[8];
    sprintf(appearance_time_str, "%02d:00", appearance_time);

    int request_type_index = rand() % 4;

    char request_type[256];
    strcpy(request_type, request_types[request_type_index]);

    struct msgbuf2 message2;
    message2.child_id = getpid();
    message2.gender = gender;
    msgsnd(msgidparent, &message2, 1, 0);

    printf("Person %d: gender=%d patient_level=%d appearance_time=%s request_type=%s\n", getpid(), gender, patient_level, appearance_time_str, request_type);
    while (should_exit_loop != 2 && current_patient <=patient_level){
        if (should_exit_loop ==1){
            write(pipechild, &request_type, sizeof(request_type));
            should_exit_loop=0;
        }
        sleep(1);
        current_patient++;
    }
    if (current_patient > patient_level){
        struct msgbuf message;
        message.child_id = getpid();
        message.value = 2;
        msgsnd(msgid, &message, 1, 0);
        printf("Leaving=> Person %d: patient_level=%d Current Patient=%d Leave Type=Anger\n", getpid(), patient_level, current_patient);
        raise(SIGINT);
        exit(0);
    }

    should_exit_loop=0;

    while (should_exit_loop != 2){
        
        sleep(1);
        current_patient++;
    }
    if (current_patient >= patient_level/2){
        struct msgbuf message;
        message.child_id = getpid();
        message.value = 1;
        msgsnd(msgid, &message, 1, 0);
        printf("Leaving=> Person %d: patient_level=%d Current Patient=%d Leave Type=Unhappy\n", getpid(), patient_level, current_patient);
    } else{
        struct msgbuf message;
        message.child_id = getpid();
        message.value = 0;
        msgsnd(msgid, &message, 1, 0);
        printf("Leaving=> Person %d: patient_level=%d Current Patient=%d Leave Type=Satisfied\n", getpid(), patient_level, current_patient);
    }
    exit(0);
}
void my_signal_handler(int sig)
{
    should_exit_loop = 1;
}
void my_signal_handler2(int sig)
{
    should_exit_loop = 2;
}