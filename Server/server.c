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

#include "../include/common.h"

int16_t socket_create(void);
int socket_bind(int hSocket, int port);

int main(int argc, char *argv[]){
    if(argc < 3)
        ERROR("Not enough arguments provided! Please Provide the Port Number to Host the Server On and Maxclients!\n./server port mxcli\n");

    int portnum = atoi(argv[1]);
    int maxclients = atoi(argv[2]);
    int socket_desc = 0, sock = 0, clientLen = 0;
    char client_message[200] = {0};
    char message[100] = {0};
    struct sockaddr_in client;

    socket_desc = socket_create();
    if(socket_desc == -1)
        ERROR("Could not create socket!\n");

    DEBUG("Socket Created!, socket_desc %d\n", socket_desc);

    if(socket_bind(socket_desc, portnum) < 0)
        ERROR("Bind failed!\n");

    DEBUG("Socket Binded to Port %d\n", portnum);

    listen(socket_desc, maxclients);

    while(1){
        puts("Waiting for incoming connections...");

        clientLen = sizeof(struct sockaddr_in);

        sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&clientLen);
    
        if(sock < 0)
            ERROR("Accept failed\n");

        DEBUG("Connection Accepted! sock %d\n", sock);

        //init buffers for recviving and sending
        memset(client_message, '\0', sizeof(client_message));

        if(recv(sock, client_message, 200, 0) < 0)
            ERROR("recv failed!\n");

        printf("Client: %s\n", client_message);

        //close(sock);
        sleep(1);
    }
}

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

int socket_bind(int hSocket, int port){
    int iRetval = -1;
    struct sockaddr_in remote = {0};

    //internet address family
    remote.sin_family = AF_INET;

    //any incoming interface
    remote.sin_addr.s_addr = htonl(INADDR_ANY); //allow any ip to connect
    remote.sin_port = htons(port);              //set client port

    iRetval = bind(hSocket, (struct sockaddr *)&remote, sizeof(remote));
    return iRetval;
}