#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "sockop.h"

#define BUFSIZE 1024

volatile int stop = 0;
int sockfd, connfd; // socket descriptor
void intHandler(int signum);
void chldHandler(int signum);

int main(int argc, char *argv[])
{
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    // int n;
    // char snd[BUFSIZE], rcv[BUFSIZE];
    pid_t child_pid;

    if (argc != 2)
        errexit("Usage: %s port\n", argv[0]);
    
    // create socket and bind socket to port
    sockfd = passivesock(argv[1], "tcp", 10);
    signal(SIGINT, intHandler);
    signal(SIGCHLD, chldHandler);

    while(!stop)
    {
        // waiting for connection
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1) {
            errexit("Error: accept()\n");
            continue;
        }

        // // read message from client
        // if ((n = read(connfd, rcv, BUFSIZE)) == -1)
        //     errexit("Error: read()\n");

        // // write message back to the client
        // n = sprintf(snd, "Server: %.*s", n, rcv);
        // if ((n = write(connfd, snd, n)) == -1)
        //     errexit("Error: write()\n");

        // close(connfd);

        child_pid = fork(); // Create new process
        if (child_pid >= 0) // fork succeed
        {
            if (child_pid == 0) // fork() returns 0 to the child process
            {
                dup2(connfd, STDOUT_FILENO);
                close(connfd);
                execlp("sl", "sl", "-l", NULL);
                exit(-1);
            }
            else    // fork returns new pid to the parent process
            {
                printf("Train ID: %d\n", (int)child_pid);
            }
        }
        else{   // fork returns -1 on failure
            perror("fork"); // display error message
            exit(0);
        }
    }
    close(sockfd);
    return 0;
}

// Close socket when catching SIGINT signal
void intHandler(int signum)
{
    stop = 1;
    close(sockfd);
}

// Remove zombie process
void chldHandler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}