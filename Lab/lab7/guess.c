/*
 * guess.c 
 * --------------------
 * Usage:
 * ./guess <key> <upper_bound> <pid>
 * 
 * - catch signal from game
 * - read result_data::result of shared memory
 * - write guess_data::guess of shared memory
 * - set timespec to wait 10 sec
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> // memset
#include "util.h"

#include <signal.h>  // sigaction
#include <time.h>    // timespec, nanosleep
#include <sys/time.h> // itimerval
#include <sys/shm.h> // shmget, shmat, shmdt, shmctl

int upper_bound, lower_bound = 1;
pid_t pid;
int shmid;
int last_guess;
key_t key;
data *guess_data;    

void timeHandler(int signal_number)
{
    // static int count = 0;
    // printf("timer expired %d times\n", count++);
    if (strcmp(guess_data->result, "smaller") == 0){
        upper_bound = last_guess;
    }
    else if (strcmp(guess_data->result, "bigger") == 0){
        lower_bound = last_guess;
    }
    else if (strcmp(guess_data->result, "start") == 0){
        lower_bound = 1;
    }
    else if (strcmp(guess_data->result, "bingo") == 0){
        exit(0);
    }
    guess_data->guess = (lower_bound + upper_bound) / 2;
    last_guess = guess_data->guess;
    printf("[game] Guess: %d\n", guess_data->guess);
    // After guess
    kill(pid, SIGUSR1);
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    struct itimerval timer;

    if (argc != 4)
    {
        printf("Usage: %s <shm_key> <upper_bound> <game_pid>\n", argv[0]);
        exit(1);
    }
    key = atoi(argv[1]);
    upper_bound = atoi(argv[2]);
    pid = atoi(argv[3]);

    // create share memory
    if ((shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(1);
    }
    if ((guess_data = shmat(shmid, NULL, 0)) == (data *)-1){
        perror("Error: shmat\n");
        exit(1);
    }
    printf("Client created and attach the share memory.\n");

    // register handler to SIGALRM
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timeHandler;
    sigaction(SIGALRM, &sa, NULL);

    // set the sleep time to 1 secs
    memset(&timer, 0, sizeof(struct itimerval));
    // the time to next expired
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    // the new time to be set when expired
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
    while(1);

    return 0;
}