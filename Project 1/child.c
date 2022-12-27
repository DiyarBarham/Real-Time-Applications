#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
int main(int argc ,char **argv){
    srand(time(0) + getpid());
    //printf("testing2\n");
    double distance = strtol(argv[0],0,0);
    pid_t process = strtol(argv[1],0,0);
    size_t size = strtol(argv[2],0,0);
    int pp = strtol(argv[3],0,0);
    
    double speed = 3+((double) rand()*7 / (double) (RAND_MAX-3));
    
    //double speed = rand();
    //write(1, &size, sizeof(size));
    write(pp, &speed, sizeof(speed));
    printf("process number %ld speed=> %f\n", size, speed);
    //printf("asd\n");
    if (size !=0)
    {
        kill(process, SIGCONT);
    }
    raise(SIGINT); 
    return 0;
}