/*
    Made by Nate, this is the starting point of the client code
    Copyright (C) 2022  Nate

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../include/colors.h"
#include "../include/common.h"

#define MAX_LEN 2048

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char loginInfo[64];
char name[64];
char buf[64];

void handle_sigint(int sig){
    flag = 1;
}

void str_overwrite_stdout() {
    printf(RED);
    printf("%s", "> ");
    printf(RESET);
    fflush(stdout);
}

void sendmsghandler(){
    char msg[MAX_LEN] = {};
    char buffer[MAX_LEN + 32] = {};

    while(1){
        str_overwrite_stdout();
        printf(BLU);
        fgets(msg, MAX_LEN, stdin);
        printf(RESET);
        msg[strcspn(msg, "\n")] = '\0';   //remove newline

        if(strcmp(msg, "/exit") == 0)
            break;
        else{
            sprintf(buffer, "%s: %s\n", name, msg);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(msg, MAX_LEN);
        bzero(buffer, MAX_LEN + 32);
    }

    handle_sigint(2);
}

void recvmsghandler(){
    char msg[MAX_LEN] = {};

    while(1){
        int receive = recv(sockfd, msg, MAX_LEN, 0);

        if(receive > 0){
            printf(GRE"%s"RESET, msg);
            str_overwrite_stdout();
        }
        else if(receive == 0)
            break;
        else{
            //-1
        }
        memset(msg, 0, sizeof(msg));
    }
}

int16_t socket_create(void){
    int16_t hSocket = 0;
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    /*
        AF_INET - internet protocal
        SOCK_STREAM - socket type
        3rd arg - flags, in this case it means protocal is choose automaticaly
    */
    return hSocket;
}

int main(int argc, char *argv[]){
    printf(CLEAR);

    if(argc != 3)
        ERROR("Usage: ./client <port> <ip>\n");

    int port = atoi(argv[1]);
    char *ip = argv[2];

    //handle user pressing ctrl+c
    signal(SIGINT, handle_sigint);

    printf(RED"{"YEL"|"GRE"["BLU"#--#--#--Welcome To The Server--#--#--#"GRE"]"YEL"|"RED"}"RESET"\n");
    printf(BACRED"     "BACYEL"     "BACGRE"     "BACBLU"     "BACMAG"     "BACRED"     "BACYEL"     "BACGRE"     "BACBLU"     "RESET"\n");

    printf("Enter your existing "YEL"UID "RESET"and "YEL"password"RESET" (fmt:<uid> <passwd> <name> [10 password bob]) :\n");
    fgets(loginInfo, 64, stdin);
    loginInfo[strcspn(loginInfo, "\n")] = '\0';   //remove newline

    sscanf(loginInfo, "%s %s %s", buf, buf, name);
    puts(name);

    if(strlen(loginInfo) > 64 || strlen(loginInfo) < 2)
        ERROR("UID, Password, and Name MUST be under 64 chars and above 2\n");

    struct sockaddr_in server_addr;

    //sock settings
    sockfd = socket_create();
    if(sockfd == -1)
        ERROR("Could not create socket!\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    //connect to the server
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(err == -1)
        ERROR("connect failed!\n");

    //send name
    send(sockfd, loginInfo, 32, 0);

    pthread_t sendmsgthread = 0;

    if(pthread_create(&sendmsgthread, NULL, (void*)sendmsghandler, NULL) != 0)
        ERROR("Could not create thread!\n");

    pthread_t recvmsgthread = 0;

    if(pthread_create(&recvmsgthread, NULL, (void*)recvmsghandler, NULL) != 0)
        ERROR("Could not create thread!\n");

    while(1){
        if(flag){
            printf("\nSee you later!\n");
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}