#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sockop.h"

#define BUFSIZE 1024

int main(int argc, char **argv)
{
    int conn_fd;
    char msg_buf[BUFSIZE];
    char rcv_buf[BUFSIZE];
    int end = 0;
    

    if (argc != 3)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(-1);
    }

    conn_fd = connectsock(argv[1], argv[2], "tcp");
    while(1)
    {
        if (end == 0){
            fgets(msg_buf, BUFSIZE, stdin);
            write(conn_fd, msg_buf, BUFSIZE);
            if (strcmp(msg_buf, "cancel\n") == 0)
                break;
        }
        read(conn_fd, rcv_buf, BUFSIZE);
        printf("%s\n", rcv_buf);
        if (strcmp(rcv_buf, "\0") == 0)
            continue;
        if (strcmp(rcv_buf, "Please wait a few minutes...") == 0)
            end = 1;
        char *token = strtok(rcv_buf, " ");
        if (strcmp(token, "Delivery") == 0)
            break;
    }
    close(conn_fd);

  return 0;
} 