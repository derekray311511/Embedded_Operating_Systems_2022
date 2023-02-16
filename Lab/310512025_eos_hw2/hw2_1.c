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

char *START_MENU = "\
1. shop list\n\
2. order\n\0";

char *SHOP_LIST = "\
Dessert Shop: 3km\n\
- cookie:$60 | cake:$80\n\
Beverage Shop: 5km\n\
- tea:$40 | boba:$70\n\
Diner: 8km\n\
- fried-rice:$120 | Egg-drop-soup:$50\n\0";

int distance[3] = {3, 5, 8};
unsigned short int cost[3][2] = {{60, 80}, {40, 70}, {120, 50}};
int sockfd, connfd; // socket descriptor
int stop = 0, test;

void intHandler(int signum);
void Order_func(int connfd);

int main(int argc, char *argv[])
{
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    if (argc != 2)
        errexit("Usage: %s port\n", argv[0]);
    
    // create socket and bind socket to port
    sockfd = passivesock(argv[1], "tcp", 1024);
    signal(SIGINT, intHandler);

    while(!stop)
    {
        // waiting for connection
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1) {
            errexit("Error: accept()\n");
            continue;
        }
        printf("Welcome!\n");
        Order_func(connfd);
    }
    printf("closing sockfd\n");
    close(sockfd);
    return 0;
}

// Close socket when catching SIGINT signal
void intHandler(int signum)
{
    stop = 1;
    printf("closing sockfd\n");
    close(sockfd);
}

void update_orderList(char *order_list, char *token, int *num, int *first)
{
    if (!(*first)){
        strcat(order_list, " | ");
    }
    strcat(order_list, token);
    strcat(order_list, " ");
    token = strtok(NULL, " ");
    *num = atoi(token);
    token = strtok(token, "\n");
    strcat(order_list, token);
    *first = 0;
}

void Order_func(int connfd)
{
    char msg_buf[BUFSIZE];
    char rcv[BUFSIZE];
    int n, money = 0;
    int i = 0;
    int shop = -1;
    int first = 1;
    char order_list[BUFSIZE];

    while(1)
    {
        printf("Money: %d\n", money);
        if ((n = read(connfd, rcv, BUFSIZE)) <= 0){
            close(connfd);
            printf("failed to read connfd: %d\n", connfd);
            pthread_exit(NULL);
        }
        char temp[BUFSIZE];
        strcpy(temp, rcv);
        printf("Client: %.*s", n, rcv); // print string with given length

        if (strcmp(rcv, "stop\n") == 0){
            stop = 1;
            close(connfd);
            break;
        }
        if (strcmp(rcv, "finish\n") == 0){
            close(connfd);
            return;
        }

        // Extract the first token
        char *token = strtok(temp, " ");
        if (strcmp(rcv, "shop list\n") == 0){
            printf("Show shop list\n");    
            strcpy(msg_buf, SHOP_LIST);
            write(connfd, msg_buf, BUFSIZE);
        }
        else if (strcmp(token, "order") == 0){
            int num;
            printf( "%s", token ); //printing second token
            token = strtok(NULL, " ");
            printf( " %s", token );
            if (strcmp(token, "cookie") == 0){
                if (shop == -1 || shop == 0){
                    update_orderList(order_list, token, &num, &first);
                    shop = 0;
                    money += cost[shop][0] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, msg_buf, BUFSIZE);
                }
            }
            else if (strcmp(token, "cake") == 0){
                if (shop == -1 || shop == 0){
                    update_orderList(order_list, token, &num, &first);
                    shop = 0;
                    money += cost[shop][1] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, msg_buf, BUFSIZE);
                }
            }
            else if (strcmp(token, "tea") == 0){
                if (shop == -1 || shop == 1){
                    update_orderList(order_list, token, &num, &first);
                    shop = 1;
                    money += cost[shop][0] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, msg_buf, BUFSIZE);
                }
            }
            else if (strcmp(token, "boba") == 0){
                if (shop == -1 || shop == 1){
                    update_orderList(order_list, token, &num, &first);
                    shop = 1;
                    money += cost[shop][1] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, msg_buf, BUFSIZE);
                }
            }
            else if (strcmp(token, "fried-rice") == 0){
                if (shop == -1 || shop == 2){
                    update_orderList(order_list, token, &num, &first);
                    shop = 2;
                    money += cost[shop][0] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, msg_buf, BUFSIZE);
                }
            }
            else if (strcmp(token, "Egg-drop-soup") == 0){
                if (shop == -1 || shop == 2){
                    update_orderList(order_list, token, &num, &first);
                    shop = 2;
                    money += cost[shop][1] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, msg_buf, BUFSIZE);
                }
            }
            else{
                strcpy(msg_buf, "No this one\n\0");
                write(connfd, msg_buf, BUFSIZE);
            }
            printf("\n");
        }
        else if (strcmp(rcv, "confirm\n") == 0){
            // Doesn't order anything
            if (money == 0){
                strcpy(msg_buf, "Please order some meals\0");
                write(connfd, msg_buf, BUFSIZE);
            }
            else{
                strcpy(msg_buf, "Please wait a few minutes...\0");
                write(connfd, msg_buf, BUFSIZE);
                // sleep(distance[shop]);
                char remain_time[10];
                for (i = 0; i < distance[shop]; i++){
                    sprintf(remain_time, "%d", distance[shop] - i);
                    strcpy(msg_buf, remain_time);
                    write(connfd, msg_buf, BUFSIZE);
                    sleep(1);
                }
                strcpy(msg_buf, "Delivery has arrived and you need to pay $");
                char money_to_pay[10];
                sprintf(money_to_pay, "%d", money);
                strcat(msg_buf, money_to_pay);
                strcat(msg_buf, "\n\0");
                write(connfd, msg_buf, BUFSIZE);
                printf("Closing connection from this client\n");
                close(connfd);
                return;
            }
        }
        else if (strcmp(rcv, "cancel\n") == 0){
            printf("Closing connection from this client\n\n");
            close(connfd);
            return;
        }
        else{
            strcpy(msg_buf, "\0");
            write(connfd, msg_buf, BUFSIZE);
        }
    }
    return;
}