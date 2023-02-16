
/*
 * game.c 
 * --------------------
 * Usage:
 * ./game <shm key> <answer>
 * 
 * - catch signal from guess
 * - read guess_data::guess of shared memory
 * - write guess_data::result of shared memory
 * - set timespec to wait 10 sec
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> // memset
#include "util.h"

#include <signal.h>  // sigaction
#include <time.h>    // timespec, nanosleep
#include <sys/shm.h> // shmget, shmat, shmdt, shmctl

int answer;
int shmid;
key_t key;
data *guess_data;    
sig_atomic_t sigusr1_count = 0;

void detach_shm()
{
    // Detach and delete shared memory
    shmdt(guess_data);
    if(shmctl(shmid, IPC_RMID, NULL) < 0){
        perror("Error: shmctl\n");
        exit(1);
    }
    printf("Shared memory successfully detached\n");
}

void sigHandler(int signal_number)
{
    ++sigusr1_count;    // add one, protected atomic action
    if (guess_data->guess == answer){
        printf("[game] Guess %d, bingo\n", guess_data->guess);
        strcpy(guess_data->result, "bingo\0");
        detach_shm();
        exit(0);
    }
    else if (guess_data->guess < answer){
        printf("[game] Guess %d, bigger\n", guess_data->guess);
        strcpy(guess_data->result, "bigger\0");
    }
    else{
        printf("[game] Guess %d, smaller\n", guess_data->guess);
        strcpy(guess_data->result, "smaller\0");
    }
}

void intHandler(int signal_number)
{
    detach_shm();
}

int main(int argc, char *argv[])
{
    int retval;
    struct sigaction sa;
    struct timespec timer;

    if (argc != 3)
    {
        printf("Usage: %s <shm_key> <answer>\n", argv[0]);
        exit(1);
    }
    key = atoi(argv[1]);
    answer = atoi(argv[2]);

    // create share memory
    if ((shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(1);
    }
    if ((guess_data = shmat(shmid, NULL, 0)) == (data *)-1){
        perror("Error: shmat\n");
        exit(1);
    }
    printf("Server created and attach the share memory.\n");
    sprintf(guess_data->result, "start");

    // register handler to SIGUSR1
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigHandler;
    sigaction(SIGUSR1, &sa, NULL);
    printf("Process %d is catching SIGUSR1...\n", getpid());

    // set the sleep time to 10 secs
    memset(&timer, 0, sizeof(struct timespec));
    timer.tv_sec = 10;
    timer.tv_nsec = 0;
    // sleep 10 secs
    do {
        retval = nanosleep(&timer, &timer);
    } while(retval);

    signal(SIGINT, intHandler);
    printf("SIGUSR1 was raised %d times\n", sigusr1_count);

    // Detach and delete shared memory
    detach_shm();

    return 0;
}