#pragma once

#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/colors.h"
#include "../include/common.h"
#include "include/account.h"

#define BUFSIZE 2048
#define MAX_CLIENTS 100
#define MAX_UID_LEN 3

struct client {
  struct sockaddr_in address;
  int sockfd;
  int uid;
  char name[32];
};

extern struct client *clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

int16_t socket_create(void);
void socket_bind(int port, int listenfd);
void send_message(char *str, int uid, bool isprivate);
void client_list(int personal_uid);
void queue_add(struct client *cli);
void queue_remove(int uid);
void *handle_client(void *ptr);
