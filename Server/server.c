/*
    Made by Nate, this is the starting point of the server code
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
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "../include/colors.h"
#include "../include/common.h"

#define BUFSIZE 1024
#define MAX_CLIENTS 100

struct client{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
};

//atomic allows thread safe code by not allowing two threads to read the same value from memory at the same time
static _Atomic uint32_t cli_count = 0;
static int uid = 10;

struct client *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int16_t socket_create(void){
    int16_t hSocket = 0;
    printf("Creating the Socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    /*
        AF_INET - internet protocal
        SOCK_STREAM - socket type
        3rd arg - flags, in this case it means protocal is choose automaticaly
    */
    return hSocket;
}

void send_message(char *str, int uid){
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; i++){                           //write to all clients while we have not hit max
        if(clients[i]){                                             //if clients[i] contains any data...
            if(clients[i]->uid != uid){                             //send message to everyone but self
                if(write(clients[i]->sockfd, str, strlen(str)) < 0){
                    perror("Write to sockfd failed!\n");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_add(struct client *cli){
    pthread_mutex_lock(&clients_mutex);     //lock ourself in

    for(int i = 0; i < MAX_CLIENTS; ++i){   //search while we have not hit max capacity
        if(!clients[i]){                    //search for first occurence of empty slot
            clients[i] = cli;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid){
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i]){                     //search for uid, provided when entering
            if(clients[i]->uid == uid){     //if uid is found...
                clients[i] = NULL;          //set it to NULL
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void *ptr){
    char buff_out[BUFSIZE];
    char name[32];
    int leave_flg = 0;

    cli_count++;
    struct client *cli = (struct client*)ptr;

    if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 2 || strlen(name) >= 32-1){
        printf("Did not enter name\n");
        leave_flg = 1;
    }
    else{
        strcpy(cli->name, name);
        sprintf(buff_out, "%s has joined!\n", cli->name);
        printf("%s", buff_out);
        send_message(buff_out, cli->uid);
    }

    bzero(buff_out, BUFSIZE);    //clear out buffer

    while(1){
        if(leave_flg)
            break;

        int receive = recv(cli->sockfd, buff_out, BUFSIZE, 0);
        if(receive > 0){
            if(strlen(buff_out) > 0){
                send_message(buff_out, cli->uid);

                buff_out[strcspn(buff_out, "\n")] = '\0';   //remove newline
                printf("%s -> %s\n", buff_out, cli->name);
            }
        }
        else if(receive == 0 || strcmp(buff_out, "exit") == 0){
            sprintf(buff_out, "%s has left\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->uid);
            leave_flg = 1;
        }
        else{
            printf("Err: recv returned -1\n");
            leave_flg = 1;
        }

        bzero(buff_out, BUFSIZE);   //clear out buffer
    }

    //remove client from queue and halt thread
    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char *argv[]){
    if(argc < 2)
        ERROR("Usage: ./server <port>\n");

    int port = atoi(argv[1]);
    int listenfd = 0, connfd = 0;
    int option = 1;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t threadid;

    listenfd = socket_create();
    if(listenfd == -1)
        ERROR("Could not create socket!\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    signal(SIGPIPE, SIG_IGN);

    if(setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0)
        ERROR("setsockopt has failed!\n");

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        ERROR("socket binding failed!\n");

    if(listen(listenfd, MAX_CLIENTS) < 0)
        ERROR("listening failed!\n");

    while(1){
        socklen_t cli_len = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);

        if((cli_count + 1) == MAX_CLIENTS){
            printf("Server Has Reached the Max Amount of Clients!\n");
            close(connfd);
            continue;
        }

        struct client *cli = (struct client*)xmalloc(sizeof(struct client));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        queue_add(cli);
        pthread_create(&threadid, NULL, &handle_client, (void*)cli);

        sleep(1);   //save some cpu usage
    }

    return EXIT_SUCCESS;
}