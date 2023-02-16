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

#define BUFSIZE 256
#define SEM_MODE 0666 // rw(owner)-rw(group)-rw(other) permission
#define SEM_KEY 1122334455

char *START_MENU = "\
1. shop list\n\
2. order\n\0";

char *SHOP_LIST = "\
Dessert Shop:3km\n\
- cookie:60$|cake:80$\n\
Beverage Shop:5km\n\
- tea:40$|boba:70$\n\
Diner:8km\n\
- fried-rice:120$|Egg-drop-soup:50$\0";

char *MEAL[6] = {"cookie", "cake", "tea", "boba", "fried-rice", "Egg-drop-soup"};

int distance[3] = {3, 5, 8};
unsigned short int cost[3][2] = {{60, 80}, {40, 70}, {120, 50}};
int sockfd; // socket descriptor
int stop = 0, test;
int sem;
int delivery_man[2] = {0};

void intHandler(int signum);
void *Order_func(void *fd);
void create_sem();
void remove_sem();
int P(int s);
int V(int s);

int main(int argc, char *argv[])
{
    int connfd[64];
    int thread_num = 0;
    int conn_num = 0;
    pthread_t thread[64];
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    if (argc != 2)
        errexit("Usage: %s port\n", argv[0]);
    
    // create semaphore
    create_sem();
    // create socket and bind socket to port
    sockfd = passivesock(argv[1], "tcp", 256);
    signal(SIGINT, intHandler);

    while(!stop)
    {
        // waiting for connection
        connfd[conn_num] = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd[conn_num] == -1) {
            errexit("Error: accept()\n");
            continue;
        }
        printf("Welcome!\n\n");
        if (pthread_create(&thread[thread_num], NULL, Order_func, &connfd[conn_num]))
        {
            perror("Error: pthread_create()\n");
        }
        thread_num++;
        conn_num++;
        if (thread_num >= 64){
            thread_num = 0;
            conn_num = 0;
        }
    }
    
    printf("closing sockfd\n");
    close(sockfd);
    remove_sem(sem);
    return 0;
}

// Close socket when catching SIGINT signal
void intHandler(int signum)
{
    stop = 1;
    printf("closing sockfd\n");
    close(sockfd);
    remove_sem(sem);
}

void update_orderList(char *order_list, 
                      unsigned short int order_num[6], 
                      char *token, int *num, int *first, int order_one)
{
    char *temp;
    char temp2[10];
    int i = 0;
    temp = strtok(NULL, " ");
    *num = atoi(temp);
    order_num[order_one] += *num;
    sprintf(temp2, "%d", order_num[order_one]);
    for (i = 0; i < 6; i++){
        if (order_num[i] != 0){
            if ((i % 2 == 1) && (order_num[i-1] != 0)){
                strcat(order_list, "|");
                strcat(order_list, MEAL[i]);
                sprintf(temp2, "%d", order_num[i]);
                strcat(order_list, " ");
                strcat(order_list, temp2);
            }
            else{
                strcpy(order_list, MEAL[i]);
                strcat(order_list, " ");
                strcat(order_list, temp2);
            }
        }
    }
    printf("Order list: %s\n", order_list);
    printf("Order num: %d %d %d %d %d %d\n", order_num[0], order_num[1], order_num[2], order_num[3], order_num[4], order_num[5]);
    *first = 0;
}

void *Order_func(void *fd)
{
    unsigned short int order_num[6] = {0};
    char msg_buf[BUFSIZE];
    char rcv[BUFSIZE];
    int n, money = 0;
    int i = 0;
    int shop = -1;
    int first = 1;
    char order_list[BUFSIZE];

    int *temp = (int *)fd;
    int connfd = *temp;

    for (i = 0; i < BUFSIZE; i++){
        order_list[i] = 0;
    }
    while(1)
    {
        for (i = 0; i < BUFSIZE; i++){
            msg_buf[i] = 0;
        }
        int order_one = -1;
        printf("Money: %d\n", money);
        if ((n = read(connfd, rcv, BUFSIZE)) <= 0){
            // close(connfd);
            printf("failed to read connfd: %d\n", connfd);
        }

        char temp[BUFSIZE];
        strcpy(temp, rcv);
        // printf("n: %d\n", n);
        if (strlen(rcv) < 1){
            continue;
        }
        printf("Client %d: %.*s\n", connfd, n, rcv); // print string with given length

        // Extract the first token
        char *token = strtok(temp, " ");
        if (strcmp(rcv, "shop list") == 0){
            // printf("Show shop list\n");
            strcpy(msg_buf, SHOP_LIST);
            write(connfd, msg_buf, BUFSIZE);
        }
        else if (strcmp(token, "order") == 0){
            int num;
            // printf("%s", token); // First word
            token = strtok(NULL, " ");
            // printf(" %s ", token); // Second word
            if (strcmp(token, "cookie") == 0){
                if (shop == -1 || shop == 0){
                    order_one = 0;
                    update_orderList(order_list, order_num, token, &num, &first, order_one);
                    shop = 0;
                    money += cost[shop][0] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    // strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, order_list, BUFSIZE);
                }
            }
            else if (strcmp(token, "cake") == 0){
                if (shop == -1 || shop == 0){
                    order_one = 1;
                    update_orderList(order_list, order_num, token, &num, &first, order_one);
                    shop = 0;
                    money += cost[shop][1] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    // strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, order_list, BUFSIZE);
                }
            }
            else if (strcmp(token, "tea") == 0){
                if (shop == -1 || shop == 1){
                    order_one = 2;
                    update_orderList(order_list, order_num, token, &num, &first, order_one);
                    shop = 1;
                    money += cost[shop][0] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    // strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, order_list, BUFSIZE);
                }
            }
            else if (strcmp(token, "boba") == 0){
                if (shop == -1 || shop == 1){
                    order_one = 3;
                    update_orderList(order_list, order_num, token, &num, &first, order_one);
                    shop = 1;
                    money += cost[shop][1] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    // strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, order_list, BUFSIZE);
                }
            }
            else if (strcmp(token, "fried-rice") == 0){
                if (shop == -1 || shop == 2){
                    order_one = 4;
                    update_orderList(order_list, order_num, token, &num, &first, order_one);
                    shop = 2;
                    money += cost[shop][0] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    // strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, order_list, BUFSIZE);
                }
            }
            else if (strcmp(token, "Egg-drop-soup") == 0){
                if (shop == -1 || shop == 2){
                    order_one = 5;
                    update_orderList(order_list, order_num, token, &num, &first, order_one);
                    shop = 2;
                    money += cost[shop][1] * num;
                    write(connfd, order_list, BUFSIZE);
                }
                else{
                    // strcpy(msg_buf, "Can not order from two shop\n\0");
                    write(connfd, order_list, BUFSIZE);
                }
            }
            else{
                strcpy(msg_buf, "No this one\n\0");
                write(connfd, msg_buf, BUFSIZE);
            }
            printf("\n");
        }
        else if (strcmp(rcv, "confirm") == 0){
            // Doesn't order anything
            if (money == 0){
                strcpy(msg_buf, "Please order some meals\0");
                write(connfd, msg_buf, BUFSIZE);
            }
            else{
                int waiting_time, man_id;
                P(sem);
                if (delivery_man[0] <= delivery_man[1]){
                    delivery_man[0] += distance[shop];
                    waiting_time = delivery_man[0];
                    man_id = 0;
                }
                else{
                    delivery_man[1] += distance[shop];
                    waiting_time = delivery_man[1];
                    man_id = 1;
                }
                V(sem);
                if (waiting_time < 30){
                    strcpy(msg_buf, "Please wait a few minutes...\0");
                    write(connfd, msg_buf, BUFSIZE);
                    printf("Waiting time: %d\n", waiting_time);
                    sleep(waiting_time);
                    strcpy(msg_buf, "Delivery has arrived and you need to pay ");
                    P(sem);
                    delivery_man[man_id] -= distance[shop];
                    V(sem);
                    char money_to_pay[10];
                    sprintf(money_to_pay, "%d", money);
                    strcat(msg_buf, money_to_pay);
                    strcat(msg_buf, "$\0");
                    write(connfd, msg_buf, BUFSIZE);
                    printf("Closing connection from client%d\n", connfd);
                    close(connfd);
                    pthread_exit(NULL);
                }
                else{
                    strcpy(msg_buf, "Your delivery will take a long time, do you want to wait?\0");
                    write(connfd, msg_buf, BUFSIZE);
                    if ((n = read(connfd, rcv, BUFSIZE)) <= 0){
                        printf("failed to read connfd(2): %d\n", connfd);
                    }
                    if (strcmp(rcv, "Yes") == 0){
                        // strcpy(msg_buf, "Please wait a few minutes...\0");
                        // write(connfd, msg_buf, BUFSIZE);
                        printf("Waiting time: %d\n", waiting_time);
                        sleep(waiting_time);
                        strcpy(msg_buf, "Delivery has arrived and you need to pay ");
                        P(sem);
                        delivery_man[man_id] -= distance[shop];
                        V(sem);
                        char money_to_pay[10];
                        sprintf(money_to_pay, "%d", money);
                        strcat(msg_buf, money_to_pay);
                        strcat(msg_buf, "$\0");
                        write(connfd, msg_buf, BUFSIZE);
                        printf("Closing connection from client%d\n", connfd);
                        close(connfd);
                        pthread_exit(NULL);
                    }
                    else{
                        P(sem);
                        delivery_man[man_id] -= distance[shop];
                        V(sem);
                        printf("Closing connection from client%d\n\n", connfd);
                        close(connfd);
                        pthread_exit(NULL);
                    }
                }
            }
        }
        else if (strcmp(rcv, "cancel") == 0){
            printf("Closing connection from client%d\n\n", connfd);
            close(connfd);
            pthread_exit(NULL);
        }
        else{
            strcpy(msg_buf, "\0");
            write(connfd, msg_buf, BUFSIZE);
        }
    }
    pthread_exit(NULL);
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

void remove_sem(int sem)
{
    // remove semaphore 
    if (semctl(sem, 0, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "Unbale to remove semaphore %d\n", SEM_KEY);
        exit(1);
    }
    printf("Semaphore %d has been removed\n", SEM_KEY);
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