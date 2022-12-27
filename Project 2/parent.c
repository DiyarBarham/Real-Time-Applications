#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <fcntl.h>

// #define NUM_PEOPLE 500
// #define GATE_SIZE 50
// #define HALL_SIZE 50
// #define NUM_OFFICES_EACH_TYPE 4
// #define ANGER_NUMBER 20
// #define SATISFIED_NUMBER 20
// #define UNHAPPY_NUMBER 20
int NUM_PEOPLE=200, GATE_SIZE=50, HALL_SIZE=50, NUM_OFFICES_EACH_TYPE=4, ANGER_NUMBER=20, SATISFIED_NUMBER=20, UNHAPPY_NUMBER=20;
int current_anger, current_satisfied, current_unhappy;
int shmid_total_males;
int shmid_total_females;
int msgidmetal;
int shmid_hall_size;
int shmid_offices;
void display();
void repeat(int);
void reshape(int,int);
void render(double x, double y, void *font, char *string);
void disTextRollingMale();
void disTextRollingFemale();
void basicInfo();
void metalGateText();
void disTextHallSize();
void init(){
    glClearColor(0.0,0.0,0.0,1.0);
}

// Structure for message queue
struct msgbuf2 {
    pid_t child_id;
    int gender;
};

struct msgbuf {
    pid_t child_id;
    int value;
};
int getnum(char ch[256]){
    char num[256];
    printf("%s", ch);
    int j=0;
    for (int i = 0; ch[i]; i++){
        if(ch[i] >= '0' && ch[i]<= '9'){
            num[j] = ch[i];
            j++;
        }
    }
    num[j] = '\0';
    int x;
    sscanf(num, "%d", &x);
    return x;
}
void openfile(){
    FILE* ptr;
    char ch[256];
    
    ptr = fopen("settings.txt", "r");

    if(ptr == NULL){
        printf("File can't be opened");
    } else{
        printf("Content of the file are:\n");
        fgets(ch, sizeof(ch), ptr);
        NUM_PEOPLE = getnum(ch);
        fgets(ch, sizeof(ch), ptr);
        GATE_SIZE = getnum(ch);
        fgets(ch, sizeof(ch), ptr);
        HALL_SIZE = getnum(ch);
        fgets(ch, sizeof(ch), ptr);
        NUM_OFFICES_EACH_TYPE = getnum(ch);
        fgets(ch, sizeof(ch), ptr);
        ANGER_NUMBER = getnum(ch);
        fgets(ch, sizeof(ch), ptr);
        SATISFIED_NUMBER = getnum(ch);
        fgets(ch, sizeof(ch), ptr);
        UNHAPPY_NUMBER = getnum(ch);
        printf("\n");
    }

    fclose(ptr);
}
int main(int argc, char** argv) {
    openfile();
    int pipechild[2];

    if (pipe(pipechild) == -1) {
        perror("pipe child failed");
        exit(1);
    }

    key_t keychild, keygenerate;
    int msgidchild, msgidgenerate;
    keychild = ftok(".", 'D');
    keygenerate = ftok(".", 'E');

    // Create the message queues
    msgidchild = msgget(keychild, IPC_CREAT | 0666);
    msgidgenerate = msgget(keygenerate, IPC_CREAT | 0666);
    if (msgidchild < 0 || msgidgenerate < 0) {
        perror("msgget");
        exit(1);
    }
    pid_t child_pid = fork();
    if (child_pid == 0) {
        close(pipechild[0]);
        char pp[50];
        char pc[50];
        sprintf(pc, "%d", pipechild[1]);
        sprintf(pp, "%d", msgidgenerate);

        char msgidch[50];
        sprintf(msgidch, "%d", msgidchild);

        char numofp[50];
        sprintf(numofp, "%d", NUM_PEOPLE);
        execlp("./people_generated", numofp, pp, pc, msgidch, NULL);
        perror("execlp failed");
        exit(1);
    }


    close(pipechild[1]);

    pid_t child_pids[NUM_PEOPLE];
    int genders[NUM_PEOPLE];
    size_t num_children = 0;
    for (int i = 0; i < NUM_PEOPLE; ++i) {
        struct msgbuf2 message2;
        msgrcv(msgidgenerate, &message2, 1, 0, 0);
        child_pids[num_children] = message2.child_id;
        genders[num_children] = message2.gender;
        num_children++;
    }


    for (size_t i = 0; i < num_children; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        // Process the child's exit status if needed
    }

    // Create shared memory segment for males
    int shmid_males = shmget(IPC_PRIVATE, NUM_PEOPLE * sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid_males < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    pid_t *shmptr_males = (pid_t *)shmat(shmid_males, NULL, 0);
    if (shmptr_males == (pid_t *)-1) {
        perror("1 shmat failed");
        exit(1);
    }

    // Create shared memory segment for females
    int shmid_females = shmget(IPC_PRIVATE, NUM_PEOPLE * sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid_females < 0) {
        perror("shmget failed");
        exit(1);
    }

    shmid_total_males = shmget(IPC_PRIVATE, sizeof(size_t), IPC_CREAT | 0666);
    if (shmid_total_males < 0) {
        perror("shmget failed");
        exit(1);
    }
    
    shmid_total_females = shmget(IPC_PRIVATE, sizeof(size_t), IPC_CREAT | 0666);
    if (shmid_total_females < 0) {
        perror("shmget failed");
        exit(1);
    }

    current_anger = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (current_anger < 0) {
        perror("parent: current_anger shmget failed");
        exit(1);
    }

    current_satisfied = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (current_satisfied < 0) {
        perror("parent: current_satisfied shmget failed");
        exit(1);
    }

    current_unhappy = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (current_unhappy < 0) {
        perror("parent: current_unhappy shmget failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    pid_t *shmptr_females = (pid_t *)shmat(shmid_females, NULL, 0);
    if (shmptr_females == (pid_t *)-1) {
        perror("2 shmat failed");
        exit(1);
    }

    size_t queue_size_males = 0;
    size_t queue_size_females = 0;
    for (size_t i = 0; i < num_children; i++) {
        if (genders[i] == 0) {
            shmptr_males[queue_size_males] = child_pids[i];
            queue_size_males++;
        } else {
            shmptr_females[queue_size_females] = child_pids[i];
            queue_size_females++;
        }
    }

    key_t keymale, keyfemale, keymetal;
    int msgidmale, msgidfemale;

    // Generate keys for the message queues
    keymale = ftok(".", 'A');
    keyfemale = ftok(".", 'B');
    keymetal = ftok(".", 'C');

    // Create the message queues
    msgidmale = msgget(keymale, IPC_CREAT | 0666);
    msgidfemale = msgget(keyfemale, IPC_CREAT | 0666);
    msgidmetal = msgget(keymetal, IPC_CREAT | 0666);

    if (msgidmale < 0 || msgidfemale < 0 || msgidmetal < 0) {
        perror("msgget");
        exit(1);
    }

    pid_t male_rolling_gate = fork();
    if (male_rolling_gate == 0) {
        // Detach shared memory segment from the process' address space
        if (shmdt(shmptr_males) == -1) {
            perror("shmptr_males failed");
            exit(1);
        }

        // Convert the shared memory segment ID to a string
        char shmid_str[16];
        sprintf(shmid_str, "%d", shmid_males);

        // Convert the queue size to a string
        char queue_size_str[16];
        sprintf(queue_size_str, "%ld", queue_size_males);

        char malemsg[50];
        sprintf(malemsg, "%d", msgidmale);

        char gatesize[50];
        sprintf(gatesize, "%d", GATE_SIZE);

        char totalvar[50];
        sprintf(totalvar, "%d", shmid_total_males);
        // Execute rolling_gate program with shared memory segment ID and queue size as arguments
        char *program_argv[] = { "./rolling_gate", malemsg, shmid_str, queue_size_str, gatesize, totalvar, NULL };
        execvp("./rolling_gate", program_argv);
        exit(1);
    }

    pid_t female_rolling_gate = fork();
    if (female_rolling_gate == 0) {
        // Detach shared memory segment from the process' address space
        if (shmdt(shmptr_females) == -1) {
            perror("shmptr_females failed");
            exit(1);
        }

        // Convert the shared memory segment ID to a string
        char shmid_str2[16];
        sprintf(shmid_str2, "%d", shmid_females);

        // Convert the queue size to a string
        char queue_size_str2[16];
        sprintf(queue_size_str2, "%ld", queue_size_females);

        char femalemsg[50];
        sprintf(femalemsg, "%d", msgidfemale);

        char gatesize[50];
        sprintf(gatesize, "%d", GATE_SIZE);

        char totalvar[50];
        sprintf(totalvar, "%d", shmid_total_females);
        // Execute rolling_gate program with shared memory segment ID and queue size as arguments
        char *program_argv[] = { "./rolling_gate", femalemsg, shmid_str2, queue_size_str2, gatesize, totalvar, NULL };
        execvp("./rolling_gate", program_argv);
        exit(1);
    }

    pid_t policeofficer = fork();
    if (policeofficer == 0) {
        char malemsg[50];
        sprintf(malemsg, "%d", msgidmale);

        char femalemsg[50];
        sprintf(femalemsg, "%d", msgidfemale);

        char metaldetectormsg[50];
        sprintf(metaldetectormsg, "%d", msgidmetal);

        char hallsize[50];
        sprintf(hallsize, "%d", HALL_SIZE);
        char *program_argv[] = { "./police_officer", malemsg, femalemsg, metaldetectormsg, hallsize, NULL };
        execvp("./police_officer", program_argv);
    }


    // Create shared memory segment for hall
    int shmid_hall = shmget(IPC_PRIVATE, HALL_SIZE * sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid_hall < 0) {
        perror("shmget failed");
        exit(1);
    }

    shmid_hall_size = shmget(IPC_PRIVATE, sizeof(size_t), IPC_CREAT | 0666);
    if (shmid_hall_size < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    size_t *shmptr_hall_size = (size_t *)shmat(shmid_hall_size, NULL, 0);
    if (shmptr_hall_size == (size_t *)-1) {
        perror("3 shmat failed");
        exit(1);
    }
    shmptr_hall_size = 0;

    int shmid_mutix = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shmid_mutix < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    int *shmptr_mutix = (int *)shmat(shmid_mutix, NULL, 0);
    if (shmptr_mutix == (int *)-1) {
        perror("4 shmat failed");
        exit(1);
    }
    shmptr_mutix = 0;

    pid_t metal_detector = fork();
    if (metal_detector == 0) {
        char metaldetectormsg[50];
        sprintf(metaldetectormsg, "%d", msgidmetal);

        // Convert the shared memory segment ID to a string
        char shmid[16];
        sprintf(shmid, "%d", shmid_hall);

        // Convert the queue size to a string
        char queue_size_hall[16];
        sprintf(queue_size_hall, "%d", shmid_hall_size);

        // Convert the queue size to a string
        char mutix[50];
        sprintf(mutix, "%d", shmid_mutix);

        char hallsize[50];
        sprintf(hallsize, "%d", HALL_SIZE);

        char *program_argv[] = { "./metal_detector", metaldetectormsg, shmid, queue_size_hall, mutix, hallsize, NULL };
        execvp("./metal_detector", program_argv);
    }

    // Create shared memory segment for hall
    shmid_offices = shmget(IPC_PRIVATE, NUM_OFFICES_EACH_TYPE * 4 * sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid_offices < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory segment to the process' address space
    pid_t *shmptr_shmid_offices = (pid_t *)shmat(shmid_offices, NULL, 0);
    if (shmptr_shmid_offices == (pid_t *)-1) {
        perror("5 shmat failed");
        exit(1);
    }

    for (int i = 0; i < NUM_OFFICES_EACH_TYPE*4; ++i) {
        shmptr_shmid_offices[i] =0;
    }

    pid_t hall = fork();
    if (hall == 0) {
        // Convert the shared memory segment ID to a string
        char shmid[16];
        sprintf(shmid, "%d", shmid_hall);

        // Convert the queue size to a string
        char queue_size_hall[16];
        sprintf(queue_size_hall, "%d", shmid_hall_size);

        // Convert the shared memory segment ID to a string
        char shmidoffices[50];
        sprintf(shmidoffices, "%d", shmid_offices);

        // Convert the shared memory segment ID to a string
        char size[50];
        sprintf(size, "%d", NUM_OFFICES_EACH_TYPE);

        char pp[50];
        sprintf(pp, "%d", pipechild[0]);

        // Convert the queue size to a string
        char mutix[50];
        sprintf(mutix, "%d", shmid_mutix);

        // Convert the queue size to a string
        char metal[50];
        sprintf(metal, "%d", metal_detector);

        char *program_argv[] = { "./hall", shmid, queue_size_hall, shmidoffices, size, pp, mutix, metal, NULL };
        execvp("./hall", program_argv);
    }

    pid_t tellersarray[NUM_OFFICES_EACH_TYPE*4];

    for (int i = 0; i < NUM_OFFICES_EACH_TYPE*4; ++i) {
        pid_t teller = fork();
        if (teller == 0) {
            // Convert the shared memory segment ID to a string
            char shmidoffices[50];
            sprintf(shmidoffices, "%d", shmid_offices);

            // Convert the shared memory segment ID to a string
            char id[50];
            sprintf(id, "%d", i);

            char *program_argv[] = { "./teller", shmidoffices, id, NULL };
            execvp("./teller", program_argv);
        } else{
            tellersarray[i] = teller;
        }
    }

    pid_t openGL = fork();
    if(openGL == 0){

        glutInit(&argc,argv);
        glutInitDisplayMode(GLUT_RGB);

        glutInitWindowPosition(200,100);
        glutInitWindowSize(500,500);

        glutCreateWindow("OIM");

        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutTimerFunc(0, repeat, 0);
        init();

        glutMainLoop();
    }

    int *shmptr_current_anger = (int *)shmat(current_anger, NULL, 0);
    if (shmptr_current_anger == (int *)-1) {
        perror("parent: shmptr_current_anger shmat failed");
        exit(1);
    }

    int *shmptr_current_satisfied = (int *)shmat(current_satisfied, NULL, 0);
    if (shmptr_current_satisfied == (int *)-1) {
        perror("parent: shmptr_current_satisfied shmat failed");
        exit(1);
    }

    int *shmptr_current_unhappy = (int *)shmat(current_unhappy, NULL, 0);
    if (shmptr_current_unhappy == (int *)-1) {
        perror("parent: shmptr_current_unhappy shmat failed");
        exit(1);
    }
    while (*shmptr_current_anger < ANGER_NUMBER && *shmptr_current_satisfied < SATISFIED_NUMBER && *shmptr_current_unhappy < UNHAPPY_NUMBER){
        struct msqid_ds buf;
        msgctl(msgidchild, IPC_STAT, &buf);
        for (int i = 0; i < buf.msg_qnum; ++i) {
            struct msgbuf message;
            msgrcv(msgidchild, &message, 1, 0, 0);
            if (message.value == 0){
                *shmptr_current_satisfied = *shmptr_current_satisfied + 1;
            } else if (message.value == 1){
                *shmptr_current_unhappy = *shmptr_current_unhappy + 1;
            } else {
                *shmptr_current_anger = *shmptr_current_anger + 1;
            }
        }
        printf("Anger Leave: %d\nSatisfied Leave: %d\nUnhappy Leave: %d\n", *shmptr_current_anger, *shmptr_current_satisfied, *shmptr_current_unhappy);
        sleep(1);
    }

    if(shmdt(shmptr_current_anger) == -1){
        perror("parent: shmptr_current_anger delete failed");
        exit(1);
    }
    if(shmdt(shmptr_current_satisfied) == -1){
        perror("parent: shmptr_current_satisfied delete failed");
        exit(1);
    }
    if(shmdt(shmptr_current_unhappy) == -1){
        perror("parent: shmptr_current_unhappy delete failed");
        exit(1);
    }

    for(int i=0; i< NUM_PEOPLE;i++){
        if(kill(child_pids[i], 0) == 0){
            kill(child_pids[i], SIGTERM);
        }
        
    }
    kill(child_pid, SIGTERM);
    kill(male_rolling_gate, SIGTERM);
    kill(female_rolling_gate, SIGTERM);
    kill(policeofficer, SIGTERM);
    kill(metal_detector, SIGTERM);
    kill(hall, SIGTERM);
    kill(openGL, SIGTERM);
    for (int i = 0; i < NUM_OFFICES_EACH_TYPE*4; i++){
        kill(tellersarray[i], SIGTERM);
    }

    if (msgctl(msgidchild, IPC_RMID, NULL) == -1){
        perror("parent: msgidchild failed");
    }
    if (msgctl(msgidfemale, IPC_RMID, NULL) == -1){
        perror("parent: msgidfemale failed");
    }
    if (msgctl(msgidgenerate, IPC_RMID, NULL) == -1){
        perror("parent: msgidgenerate failed");
    }
    if (msgctl(msgidmale, IPC_RMID, NULL) == -1){
        perror("parent: msgidmale failed");
    }
    if (msgctl(msgidmetal, IPC_RMID, NULL) == -1){
        perror("parent: msgidmetal failed");
    }

    if(shmdt(shmptr_females) == -1){
        perror("parent: shmptr_females failed");
        exit(1);
    }
    if(shmdt(shmptr_males) == -1){
        perror("parent: shmptr_males failed");
        exit(1);
    }

    if(shmdt(shmptr_shmid_offices) == -1){
        perror("parent: shmptr_shmid_offices failed");
        exit(1);
    }

    if (shmctl(shmid_females, IPC_RMID, NULL) == -1){
        perror("parent: shmid_females failed");
    }
    if (shmctl(shmid_hall, IPC_RMID, NULL) == -1){
        perror("parent: shmid_hall failed");
    }
    if (shmctl(shmid_hall_size, IPC_RMID, NULL) == -1){
        perror("parent: shmid_hall_size failed");
    }
    if (shmctl(shmid_males, IPC_RMID, NULL) == -1){
        perror("parent: shmid_males failed");
    }
    if (shmctl(shmid_mutix, IPC_RMID, NULL) == -1){
        perror("parent: shmid_mutix failed");
    }
    if (shmctl(shmid_offices, IPC_RMID, NULL) == -1){
        perror("parent: shmid_offices failed");
    }

    return 0;
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glBegin(GL_POLYGON);
    glColor3f(1,0,0);
    glVertex2f(95, 5);
    glVertex2f(95, 195);
    glVertex2f(195, 195);
    glVertex2f(195, 5);
    glEnd();
    double temp = (double)190/(double)(NUM_OFFICES_EACH_TYPE*4);
    int j=5;

    pid_t *shmptr_offices = (pid_t *)shmat(shmid_offices, NULL, 0);
    if (shmptr_offices == (pid_t *)-1) {
        perror("parent-> shmat failed");
        exit(1);
    }
    for(int i=0; i< (NUM_OFFICES_EACH_TYPE*4);i++){
        glBegin(GL_POLYGON);
        if(shmptr_offices[i] == 0 ){
            glColor3f(0,1,0);
        } else{
            glColor3f(1,1,0);
        }
        
        glVertex2f(130, 6 + temp*i);
        glVertex2f(130, 4 + temp*(i+1));
        glVertex2f(195, 4 + temp*(i+1));
        glVertex2f(195, 6 + temp*i);
        glEnd();
    }

    if(shmdt(shmptr_offices) == -1){
        perror("parent: shmptr_offices failed");
        exit(1);
    }

    glBegin(GL_POLYGON);
    glColor3f(0,1,0);
    glVertex2f(100, 6);
    glVertex2f(100, 26);
    glVertex2f(125, 26);
    glVertex2f(125, 6);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0,1,0);
    glVertex2f(40, 16.5);
    glVertex2f(40, 26);
    glVertex2f(90, 26);
    glVertex2f(90, 16.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0,1,0);
    glVertex2f(40, 6);
    glVertex2f(40, 15.5);
    glVertex2f(90, 15.5);
    glVertex2f(90, 6);
    glEnd();

    disTextRollingMale();
    disTextRollingFemale();
    basicInfo();
    metalGateText();
    disTextHallSize();
    glFlush();
    glutSwapBuffers();

   
}

void reshape(int w, int h){
    glViewport(0,0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,200,0,200);
    glMatrixMode(GL_MODELVIEW);
}

void repeat(int t){
    glutPostRedisplay();
    glutTimerFunc(1000/60, repeat, 0);

}

void render(double x, double y, void *font, char *string){
    char *c;
    glRasterPos2s(x,y);
    for(c=string;*c!='\0';c++){
        glutBitmapCharacter(font, *c);
    }
}
void disTextRollingMale(){
    // Attach shared memory segment to the process' address space
    size_t *shmptr_total_males = (size_t *)shmat(shmid_total_males, NULL, 0);
    if (shmptr_total_males == (size_t *)-1) {
        perror("parent: shmptr_total_males shmat failed");
        exit(1);
    }
    glColor3f(0,0,0);
    char text[100];
    sprintf(text,"Male: %ld", *shmptr_total_males);
    if(shmdt(shmptr_total_males) == -1){
        perror("parent: shmptr_total_males delete failed");
        exit(1);
    }
    render(43,8,GLUT_BITMAP_TIMES_ROMAN_24, text);
}

void disTextRollingFemale(){
    // Attach shared memory segment to the process' address space
    size_t *shmptr_total_females = (size_t *)shmat(shmid_total_females, NULL, 0);
    if (shmptr_total_females == (size_t *)-1) {
        perror("parent: shmptr_total_females shmat failed");
        exit(1);
    }
    glColor3f(0,0,0);
    char text[100];
    sprintf(text,"Female: %ld", *shmptr_total_females);
    if(shmdt(shmptr_total_females) == -1){
        perror("parent: shmptr_total_females delete failed");
        exit(1);
    }
    render(43,18,GLUT_BITMAP_TIMES_ROMAN_24, text);
}

void basicInfo(){
    glColor3f(1,1,0);
    char text[100];
    sprintf(text,"Total Number: %d", NUM_PEOPLE);
    render(5,195,GLUT_BITMAP_TIMES_ROMAN_10, text);

    glColor3f(1,1,0);
    char text2[100];
    sprintf(text2,"Max Number of Anger: %d", ANGER_NUMBER);
    render(5,185,GLUT_BITMAP_TIMES_ROMAN_10, text2);

    glColor3f(1,1,0);
    char text3[100];
    sprintf(text3,"Max Number of Satisfied: %d", SATISFIED_NUMBER);
    render(5,175,GLUT_BITMAP_TIMES_ROMAN_10, text3);

    glColor3f(1,1,0);
    char text4[100];
    sprintf(text4,"Max Number of Unhappy: %d", UNHAPPY_NUMBER);
    render(5,165,GLUT_BITMAP_TIMES_ROMAN_10, text4);

    int *shmptr_current_anger = (int *)shmat(current_anger, NULL, 0);
    if (shmptr_current_anger == (int *)-1) {
        perror("parent: shmptr_current_anger shmat failed");
        exit(1);
    }

    int *shmptr_current_satisfied = (int *)shmat(current_satisfied, NULL, 0);
    if (shmptr_current_satisfied == (int *)-1) {
        perror("parent: shmptr_current_satisfied shmat failed");
        exit(1);
    }

    int *shmptr_current_unhappy = (int *)shmat(current_unhappy, NULL, 0);
    if (shmptr_current_unhappy == (int *)-1) {
        perror("parent: shmptr_current_unhappy shmat failed");
        exit(1);
    }

    glColor3f(1,1,0);
    char text5[100];
    sprintf(text5,"Current Number of Anger: %d", *shmptr_current_anger);
    render(5,155,GLUT_BITMAP_TIMES_ROMAN_10, text5);

    glColor3f(1,1,0);
    char text6[100];
    sprintf(text6,"Current Number of Satisfied: %d", *shmptr_current_satisfied);
    render(5,145,GLUT_BITMAP_TIMES_ROMAN_10, text6);

    glColor3f(1,1,0);
    char text7[100];
    sprintf(text7,"Current Number of Unhappy: %d", *shmptr_current_unhappy);
    render(5,135,GLUT_BITMAP_TIMES_ROMAN_10, text7);

    if(shmdt(shmptr_current_anger) == -1){
        perror("parent: shmptr_current_anger delete failed");
        exit(1);
    }
    if(shmdt(shmptr_current_satisfied) == -1){
        perror("parent: shmptr_current_satisfied delete failed");
        exit(1);
    }
    if(shmdt(shmptr_current_unhappy) == -1){
        perror("parent: shmptr_current_unhappy delete failed");
        exit(1);
    }
}

void metalGateText(){
    struct msqid_ds buf;
    msgctl(msgidmetal, IPC_STAT, &buf);
    glColor3f(0,0,0);
    char text[100];
    int temp = buf.msg_qnum;
    sprintf(text,"%d", temp);
    char tt[100] = "Metal";
    render(101,18,GLUT_BITMAP_TIMES_ROMAN_24, tt);
    render(108,10,GLUT_BITMAP_TIMES_ROMAN_24, text);
}

void disTextHallSize(){
    // Attach shared memory segment to the process' address space
    size_t *shmptr_hall_size = (size_t *)shmat(shmid_hall_size, NULL, 0);
    if (shmptr_hall_size == (size_t *)-1) {
        perror("parent: shmptr_hall_size shmat failed");
        exit(1);
    }
    glColor3f(0,0,0);
    char text[100];
    sprintf(text,"%ld", *shmptr_hall_size);
    if(shmdt(shmptr_hall_size) == -1){
        perror("parent: shmptr_hall_size delete failed");
        exit(1);
    }
    render(105,180,GLUT_BITMAP_TIMES_ROMAN_24, "Hall");
    render(110,170,GLUT_BITMAP_TIMES_ROMAN_24, text);
}