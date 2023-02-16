#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <pthread.h>
#include "sockop.h"

#define BUFSIZE 1024
#define SEM_MODE 0666 // rw(owner)-rw(group)-rw(other) permission
#define SEM_KEY 1122334455

int stop = 0;
int sockfd, connfd; // socket descriptor
int sem;
long int total = 0;

void intHandler(int signum);
void chldHandler(int signum);
void *ThreadCallBack(void *connfd);
void create_sem();
int P(int s);
int V(int s);
int call_count = 0;
int fail_count = 0;

int main(int argc, char *argv[])
{
    // ==================== Socket usage ==================== //
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    if (argc != 2)
        errexit("Usage: %s port\n", argv[0]);
    
    // create socket and bind socket to port
    sockfd = passivesock(argv[1], "tcp", 2000);
    signal(SIGINT, intHandler);

    // ==================== Thread usage ==================== //
    pthread_t   thread;

    // ==================== Semaphore usage ==================== //
    create_sem();

    // ==================== Start to run ==================== //
    int count = 0;
    while(!stop)
    {
        // count++;
        // if (count == 2000)
        //     printf("count: %d\n", count);
        // waiting for connection
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1) {
            errexit("Error: accept()\n");
            continue;
        }

        if (pthread_create(&thread, NULL, ThreadCallBack, &connfd))
        {
            perror("Error: pthread_create()\n");
        }
        // close(connfd);
    }
    printf("closing sockfd\n");
    close(sockfd);
    return 0;
}

void *ThreadCallBack(void *fd)
{
    call_count++;
    char rcv[BUFSIZE];
    int n;
    int *temp = (int *)fd;
    int connfd = *temp;
    
    // if (call_count == 2000){
    //     printf("call %d times\n", call_count);
    //     printf("fail %d times\n", fail_count);
    // }
    // printf("fd: %d, connfd: %d\n", *temp, connfd);
    if ((n = read(connfd, rcv, BUFSIZE)) <= 0){
        fail_count++;
        close(connfd);
        if (rcv[0] == '\0')
            printf("rcv = \\0\n");
        printf("failed to read connfd: %d\n", connfd);
        pthread_exit(NULL);
    }

    int times = atoi(rcv + 4);
    printf("rcv + 4: %s\n", rcv + 4);
    int j = 0;
    printf("times: %d\n", times);
    for (j = 0; j < times; j++){
        P(sem);
        printf("%.*s\n", n, rcv); // print string with given length
        if (rcv[0] == 'D'){
            // printf("+%d\n", atoi(rcv + 2));
            total += atoi(&rcv[2]);
            printf("after deposit: %ld\n", total);
        }
        else if (rcv[0] == 'W'){
            // printf("-%d\n", atoi(rcv + 2));
            total -= atoi(rcv + 2);
            printf("after withdraw: %ld\n", total);
        }
        else{
            printf("Wrong mode, should be 'D' or 'W'\n");
        }
        V(sem);
    }
    
    close(connfd);
    pthread_exit(NULL);
}

// Close socket when catching SIGINT signal
void intHandler(int signum)
{
    stop = 1;
    printf("closing sockfd\n");
    close(sockfd);

    // remove semaphore 
    if (semctl(sem, 0, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "Unbale to remove semaphore %d\n", SEM_KEY);
        exit(1);
    }
    printf("Semaphore %d has been removed\n", SEM_KEY);
}

// Remove zombie process
void chldHandler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void create_sem()
{
    // create smeaphore
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (sem < 0)
    {
        fprintf(stderr, "Creation of semaphore %d failed: %s\n",
            SEM_KEY, strerror(errno));
        exit(-1);
    }
    // initial semaphore value to 1 (binary semaphore)
    if (semctl(sem, 0, SETVAL, 1) < 0 )
    {
        fprintf(stderr, "Unable to initilize semaphore: %s\n",
            strerror(errno));
        exit(-1);
    }
    printf("Semaphore: %d has been created & initialized to 1\n", SEM_KEY);
}

// P() - returns 0 if OK; -1 if there was a problem
int P(int s)
{
    struct sembuf sop;  // the operation parameters
    sop.sem_num = 0;    // access the 1st (and only) sem in the arry
    sop.sem_op = -1;    // wait...
    sop.sem_flg = 0;    // no special options needed

    if (semop(s, &sop, 1) < 0){
        fprintf(stderr, "P(): semop failed: %s\n", strerror(errno));
        return -1;
    }
    else{
        return 0;
    }
}

// V() - returns 0 if OK; -1 if there was a problem
int V(int s)
{
    struct sembuf sop;  // the operation parameters
    sop.sem_num = 0;    // the 1st (and only) sem in the array
    sop.sem_op = 1;     // signal
    sop.sem_flg = 0;    // no special options needed

    if (semop(s, &sop, 1) < 0){
        fprintf(stderr, "V(): semop failed: %s\n", strerror(errno));
        return -1;
    }
    else{
        return 0;
    }
}