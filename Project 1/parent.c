#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#define MAX_PLAYERS 10

pid_t players_pids[MAX_PLAYERS];
size_t players_pids_size = 0;
pid_t parent_id;
void kill_children(int signal , int delay);
void signal_handler(int signal);
int pipes[MAX_PLAYERS][2];
// int fromparentpipe[MAX_PLAYERS][2];
double speed[MAX_PLAYERS];
double distance=40;
pid_t nextplayer;
double drawdistance =3;
// pid_t drawer;
// int drawer1pipe[2];
// int drawer2pipe[2];
void display();
void reshape(int,int);

void init(){
    glClearColor(0.0,0.0,0.0,1.0);
}

int main(int argc, char** argv){
    

    //always having the parent id.
    parent_id = getpid();
    for (int i = 0; i < MAX_PLAYERS; i++){
        pipe(pipes[i]);
        // pipe(fromparentpipe[i]);
    }
    
    for (int i = 0; i < MAX_PLAYERS; i++){
        speed[i] = -1;
    }



    // drawer = fork();
    // if(drawer == 0){
    //     close(drawerpipe[1]);
    //     kill(getpid(), SIGTSTP);
    //     while(1){
    //         read(drawerpipe[0], &)

    //         sleep(1);
    //     }
    //     exit(0);
    //     raise(SIGINT);
    // }
    // close(drawerpipe[0]);
    
    for(int rounds=0; rounds<5;rounds++){


    //while tell we have 10 childs
    while (players_pids_size < MAX_PLAYERS){
        //new child.
        pid_t player_id = fork();
        //error in making child.
        if(player_id < 0){
            // error name and number.
            fprintf(stderr, "Couldn't make child player. error is: %s (error number: %d)", strerror(errno), errno);
                
        } else if(player_id == 0){//child forking was success
            close(pipes[players_pids_size][0]);
            // close(fromparentpipe[players_pids_size][1]);
            /*stopping the process from wokring to save 
                processing power from CPU but will
                continue from this place when 'SIGCONT'
                is sent*/
            kill(getpid(), SIGTSTP);
            // read(fromparentpipe[players_pids_size][0], &nextplayer, sizeof(nextplayer));
            char dis[50];
            char previd[50];
            char pidsize[50];
            char pp[50];
            sprintf(dis, "%f", distance);
            sprintf(previd, "%d", players_pids[players_pids_size-1]);
            sprintf(pidsize, "%ld", players_pids_size);
            sprintf(pp, "%d", pipes[players_pids_size][1]);
            execlp("./child", dis, previd, pidsize, pp, NULL);
            /*the problem is that the child process 
                will continue working and will go through
                the while loop as a parent process for its
                children so we need a break to stop it*/
            raise(SIGINT); 
            break;
        } else{//parent process
            //saving all childrens ids in array
            close(pipes[players_pids_size][1]);
            // close(fromparentpipe[players_pids_size][0]);
            players_pids[players_pids_size++] = player_id;
        }
        
    }
    
    if(parent_id == getpid()){
        
        for (int i = 0; i < players_pids_size; i++){
            waitpid(players_pids[i], NULL, WUNTRACED);    
        }
        // kill(players_pids[4], SIGCONT);
        // write(fromparentpipe[0][1], &players_pids[0], sizeof(players_pids[0]));

        kill(players_pids[9], SIGCONT);
        //kill(players_pids[4], SIGCONT);
        double temp2;
        size_t t = players_pids_size-1;
        for (int i = 9; i >= 0; i--)
        {

            //read(0, &temp, sizeof(temp));
            read(pipes[t][0], &temp2, sizeof(temp2));
            speed[i]=temp2;
            printf("#p speed of process %d => %f\n", i, speed[i]);
            t--;
        }
        pid_t openGL = fork();
        if(openGL<0){
            fprintf(stderr, "Couldn't make draw process. error is: %s (error number: %d)", strerror(errno), errno);
        } else if(openGL == 0){
            glutInit(&argc,argv);
            glutInitDisplayMode(GLUT_RGB);

            glutInitWindowPosition(200,100);
            glutInitWindowSize(500,500);

            char windowname[50];
            sprintf(windowname, "Round %d", rounds+1);
            glutCreateWindow(windowname);

            glutDisplayFunc(display);
            glutReshapeFunc(reshape);
            init();

            glutMainLoop();
        } else{
            waitpid(openGL, NULL, 0); 
        }
        
        // write(fromparentpipe[5][1], &players_pids[5], sizeof(players_pids[5]));
    }

    waitpid(players_pids[0], NULL, 0); 
    for (int i = 0; i < players_pids_size; i++)
    {
        kill(players_pids[i], SIGKILL);
    }
    }
}


void display(){
    // glClear(GL_COLOR_BUFFER_BIT);
    // glLoadIdentity();
    // for(size_t i=0; i<players_pids_size;i++){
        
    //     if(i>4){
    //         glColor3f(1,1,0);
    //     } else{
    //         glColor3f(1,0,0);
    //     }
    //     glPointSize(8);
    //     glBegin(GL_POINTS);
    //     if(i>4){
    //         glVertex2i((drawdistance*(i-5))-6,-5.0);
    //     } else{
    //         glVertex2i((drawdistance*i)-6,5.0);
    //     }
    //     glEnd();
    // }
    //draw
    int cA=0, cB=0, pA=0, pB=5;
    while (1){
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        

        glColor3f(1,0,0);
        glPointSize(8);
        glBegin(GL_POINTS);
        for(size_t i=0; i<5;i++){
            if(i==pA){
                double dis= (drawdistance*(i))+(cA*(drawdistance/distance)*speed[i])-6;
                //cA++;
                if(dis>((drawdistance*(i+1))-6)){
                    dis= (drawdistance*(i+1))-6;
                    pA++;
                    cA=0;
                }
                glVertex2i(dis,5.0);
                
            } else if(i<pA){
                glVertex2i((drawdistance*(i+1))-6,5.0);
            }  else{
                glVertex2i((drawdistance*i)-6,5.0);
            }
        }
        glEnd();

        glColor3f(1,1,0);
        glPointSize(8);
        glBegin(GL_POINTS);
        for(size_t i=5; i<10;i++){
            if(i==pB){
                double dis= ((drawdistance*(i-5))+(cB*drawdistance/distance*speed[i]))-6;
                //cB++;
                if(dis>((drawdistance*(i-4))-6)){
                    dis= (drawdistance*(i-4))-6;
                    pB++;
                    cB=0;
                }
                glVertex2i(dis,-5.0);
                
            } else if(i<pB && i>4){
                glVertex2i((drawdistance*(i-4))-6,-5.0);
            } else{
                glVertex2i((drawdistance*(i-5))-6,-5.0);
            }
        }
        glEnd();
        cA++;
        cB++;
        glFlush();
        if(pA==5 && pB==10){
            break;
        }
        sleep(1);

        // for(size_t i=0; i<players_pids_size;i++){
                
        //         if(i>4){
        //             glColor3f(1,1,0);
        //         } else{
        //             glColor3f(1,0,0);
        //         }
        //         glPointSize(8);
        //         glBegin(GL_POINTS);

        //         if(i==pA){
        //             double dis= (drawdistance*(i))+(cA*(drawdistance/distance)*speed[i])-6;
        //             printf("$$$$$$$%f\n",drawdistance*(i));
        //             //cA++;
        //             printf("########%f\n",dis);
        //             printf("total->%f\n",(drawdistance*(i+1))-6);
        //             if(dis>((drawdistance*(i+1))-6)){
        //                 dis= (drawdistance*(i+1))-6;
        //                 pA++;
        //                 cA=-1;
        //             }
        //             glVertex2i(dis,5.0);
                    
        //         } else if(i==pB){
        //             double dis= ((drawdistance*(i-5))+(cB*drawdistance/distance*speed[i]))-6;
        //             //cB++;
        //             printf("@@@@@@@%f\n",dis);
        //             if(dis>((drawdistance*(i-4))-6)){
        //                 dis= (drawdistance*(i-4))-6;
        //                 pB++;
        //                 cB=-1;
        //             }
        //             glVertex2i(dis,-5.0);
                    
        //         } else if(i<pA){
        //             glVertex2i((drawdistance*(i+1))-6,5.0);
        //         } else if(i<pB && i>4){
        //             glVertex2i((drawdistance*(i-4))-6,-5.0);
        //         } else{
        //             if(i>4){
        //                 glVertex2i((drawdistance*(i-5))-6,-5.0);
        //             } else{
        //                 glVertex2i((drawdistance*i)-6,5.0);
        //             }
        //         }
                
        //     }
        // glEnd();
        
    }
    raise(SIGINT); 
    
    // glFlush();
    // sleep(2);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glLoadIdentity();
    // for(size_t i=0; i<players_pids_size;i++){
        
    //     if(i>4){
    //         glColor3f(1,1,0);
    //     } else{
    //         glColor3f(1,0,0);
    //     }
    //     glPointSize(8);
    //     glBegin(GL_POINTS);
    //     if(i>4){
    //         glVertex2i((drawdistance*(i-5))-6,-2.0);
    //     } else{
    //         glVertex2i((drawdistance*i)-6,2.0);
    //     }
    //     glEnd();
    // }
    // // glVertex2i(-6.0,5.0);
    // // glVertex2i(-3.0,5.0);
    // // glVertex2i(0.0,5.0);
    // // glVertex2i(3.0,5.0);
    // // glVertex2i(6.0,5.0);
    // // glEnd();


    // // glColor3f(1,1,0);
    // // glPointSize(8);
    // // glBegin(GL_POINTS);
    // // glVertex2i(-6.0,-5.0);
    // // glVertex2i(-3.0,-5.0);
    // // glVertex2i(0.0,-5.0);
    // // glVertex2i(3.0,-5.0);
    // // glVertex2i(6.0,-5.0);
    // // glEnd();

    // glFlush();

}

void reshape(int w, int h){
    glViewport(0,0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-10,10,-10,10);
    glMatrixMode(GL_MODELVIEW);
}