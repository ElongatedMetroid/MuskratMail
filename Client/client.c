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
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>

#include "../include/common.h"
#include "include/account.h"

int16_t socket_create(void);
int socket_connect(int hSocket, int port, char *ip);
int socket_send(int hSocket, char *Rqst, int16_t lenRqst);
int socket_receive(int hSocket, char* Rsp, int16_t RvcSize);

int main(int argc, char *argv[]){
    if(argc < 4)
        ERROR("Not enough arguments provided! Please Provide the Port Number to Host the Server On and Maxclients!\n./client port serverip accdatapath\n");

    struct account_data acc_data;
    char *accdatapath = argv[3];
    char *server_ip = argv[2];
    int server_port = atoi(argv[1]);

    int hSocket = 0, read_size = 0;
    //struct sockaddr_in server;
    char sendToServer[100] = {0};
    char server_reply[200] = {0};

    account_load(&acc_data, accdatapath);

    hSocket = socket_create();

    if(hSocket == -1)
        ERROR("Could not create socket!\n");

    DEBUG("Socket Created! hSocket %d\n", hSocket);

    if(socket_connect(hSocket, server_port, server_ip) < 0)
        ERROR("Connect Failed!\n");

    printf("Sucessfully Connected With Server\n");

    printf("Enter the Message: ");
    fgets(sendToServer, 100, stdin);

    //Send data to server
    socket_send(hSocket, acc_data.ip_address, strlen(acc_data.ip_address));
    socket_send(hSocket, sendToServer, strlen(sendToServer));

    //Recive data from the server
    read_size = socket_receive(hSocket, server_reply, 200);
    printf("server response: %s\n\n", server_reply);

    close(hSocket);

    return 0;
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

int socket_connect(int hSocket, int port, char *ip){
    int iRetval = -1;
    struct sockaddr_in remote = {0};

    //internet address family
    remote.sin_family = AF_INET;

    //any incoming interface
    remote.sin_addr.s_addr = inet_addr(ip);
    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);              //set client port

    iRetval = connect(hSocket, (struct sockaddr *)&remote, sizeof(remote));
    return iRetval;
}

int socket_send(int hSocket, char *Rqst, int16_t lenRqst){
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20; //20 secs timeout
    tv.tv_usec = 0;

    if(setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv)) < 0)
        ERROR("Time out!\n");

    shortRetval = send(hSocket, Rqst, lenRqst, 0);
    return shortRetval;
}

int socket_receive(int hSocket, char* Rsp, int16_t RvcSize){
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20; //20 secs timeout
    tv.tv_usec = 0; 

    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) < 0)
        ERROR("Time Out!\n");

    shortRetval = send(hSocket, Rsp, RvcSize, 0);
    printf("Response %s\n", Rsp);

    return shortRetval;
}