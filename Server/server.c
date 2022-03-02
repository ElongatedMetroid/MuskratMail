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

#include "server.h"

// atomic allows thread safe code by not allowing two threads to read the same
// value from memory at the same time
static _Atomic uint32_t cli_count = 0;
char *account_file_path = NULL;
struct client *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_message(char *str, int uid, bool isprivate) {
  pthread_mutex_lock(&clients_mutex);

  for (int i = 0; i < MAX_CLIENTS;
       i++) {         // write to all clients while we have not hit max
    if (clients[i]) { // if clients[i] contains any data...
      if (isprivate) {
        if (clients[i]->uid == uid) {
          if (write(clients[i]->sockfd, str, strlen(str)) < 0) {
            perror("Write to sockfd failed!\n");
            break;
          }
        }
      }
      if (!isprivate) {
        if (clients[i]->uid != uid) { // send message to everyone but self
          if (write(clients[i]->sockfd, str, strlen(str)) < 0) {
            perror("Write to sockfd failed!\n");
            break;
          } // end write success check
        }   // end if
      }     // end else
    }       // end if
  }         // end for

  pthread_mutex_unlock(&clients_mutex);
}

void client_list(int personal_uid) {
  pthread_mutex_lock(&clients_mutex);
  char buf[BUFSIZE];

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      sprintf(buf, "Client %d: Name: %s: UID: %d\n", i, clients[i]->name,
              clients[i]->uid);
      pthread_mutex_unlock(
          &clients_mutex); // unlock to allow sending private message
      send_message(buf, personal_uid, true);
      pthread_mutex_lock(&clients_mutex);
    }
  }

  pthread_mutex_unlock(&clients_mutex);
}

void queue_add(struct client *cli) {
  pthread_mutex_lock(&clients_mutex); // lock ourself in

  for (int i = 0; i < MAX_CLIENTS;
       ++i) {          // search while we have not hit max capacity
    if (!clients[i]) { // search for first occurence of empty slot
      clients[i] = cli;
      break;
    }
  }

  pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid) {
  pthread_mutex_lock(&clients_mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i]) {               // search for uid, provided when entering
      if (clients[i]->uid == uid) { // if uid is found...
        clients[i] = NULL;          // set it to NULL
        break;
      }
    }
  }

  pthread_mutex_unlock(&clients_mutex);
}

void manageArgs(char *buf_out, int uid) {
  if (strstr(buf_out, "/PRIVATE")) {
    int i = 0;
    sscanf(buf_out, "%d", &i);

    send_message(buf_out, i, true);
  } else if (strstr(buf_out, "/ls"))
    client_list(uid);
  else if (strstr(buf_out, "/clear"))
    send_message(CLEAR, uid, true);
}

void *handle_client(void *ptr) {
  char buff_out[BUFSIZE];
  char loginInfo[64];
  int leave_flg = 0;

  cli_count++;
  struct client *cli = (struct client *)ptr;

  if (recv(cli->sockfd, loginInfo, 64, 0) <= 0 || strlen(loginInfo) < 2 ||
      strlen(loginInfo) >= 64 - 1) {
    printf("Did not enter UID and Password\n");
    leave_flg = 0;
  } else {
    char *usr_uid = NULL;
    char *pass = NULL;
    char *name = NULL;

    usr_uid = xmalloc(sizeof(loginInfo));
    pass = xmalloc(sizeof(loginInfo));
    name = xmalloc(sizeof(loginInfo));

    sscanf(loginInfo, "%s %s %s", usr_uid, pass, name);

    if (account_load(cli, account_file_path, usr_uid, pass) == EXIT_FAILURE) {
      send_message("UID and Password not found\nYou are now disconnected.\n",
                   cli->uid, true);
      leave_flg = 1;
    }

    free(usr_uid);
    free(pass);

    strcpy(cli->name, name);
    free(name);
    sprintf(buff_out,
            GRE "%s" RESET " has " RED "j" YEL "o" GRE "i" BLU "n" MAG "e" RED
                "d" YEL "!" RESET "\n",
            cli->name);
    printf("%s", buff_out);
    send_message(buff_out, cli->uid, false);
  }

  bzero(buff_out, BUFSIZE); // clear out buffer

  while (1) {
    if (leave_flg) {
      printf("got leave flag %d\n", leave_flg);
      break;
    }

    int receive = recv(cli->sockfd, buff_out, BUFSIZE, 0);
    if (receive > 0) {

      if (strstr(buff_out, "/PRIVATE") || strstr(buff_out, "/ls") ||
          strstr(buff_out, "/clear")) {
        manageArgs(buff_out, cli->uid);
        continue; // so it does not show private message in public chat
      }

      if (strlen(buff_out) > 0) {
        send_message(buff_out, cli->uid, false);
        buff_out[strcspn(buff_out, "\n")] = '\0'; // remove newline
        printf("%s -> %s\n", buff_out, cli->name);
      }
    } else if (receive == 0 || strcmp(buff_out, "/exit") == 0) {
      sprintf(buff_out, "%s has left\n", cli->name);
      printf("%s", buff_out);
      send_message(buff_out, cli->uid, false);
      leave_flg = 1;
    } else {
      printf("Err: recv returned -1\n");
      leave_flg = 1;
    }

    bzero(buff_out, BUFSIZE); // clear out buffer
  }

  // remove client from queue and halt thread
  close(cli->sockfd);
  queue_remove(cli->uid);
  free(cli);
  cli = NULL;
  cli_count--;
  pthread_detach(pthread_self());

  return NULL;
}

void socket_bind(int port, int listenfd) {
  struct sockaddr_in serv_addr;
  int option = 1;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(
      port); // convert ports byte order to network byte order (host to short)

  signal(SIGPIPE, SIG_IGN);

  if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR),
                 (char *)&option, sizeof(option)) < 0)
    ERROR("setsockopt has failed!\n");

  if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    ERROR("socket binding failed!\n");
}

int16_t socket_create(void) {
  int16_t hSocket = 0;
  hSocket = socket(AF_INET, SOCK_STREAM, 0);
  /*
      AF_INET - internet protocal
      SOCK_STREAM - socket type
      3rd arg - flags, in this case it means protocal is choose automaticaly
  */
  if (hSocket == -1)
    ERROR("Could not create socket!\n");
  return hSocket;
}

int main(int argc, char *argv[]) {
  if (argc < 3)
    ERROR("Usage: ./server <port> <accountsfile>\n");

  printf(
      "Server started!\n (Dont Worry, Your server is not broken, more output "
      "will come once a person has joined or an error has occured)\n");

  int port = atoi(argv[1]);
  account_file_path = xmalloc(sizeof(argv[2] + 1));
  account_file_path = argv[2]; // Make global
  int listenfd = 0, connfd = 0;

  pthread_t threadid = 0;

  listenfd = socket_create();

  socket_bind(port, listenfd);

  if (listen(listenfd, MAX_CLIENTS) < 0)
    ERROR("listening failed!\n");

  struct sockaddr_in cli_addr;
  while (1) {
    socklen_t cli_len = sizeof(cli_addr);
    connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);

    if ((cli_count + 1) == MAX_CLIENTS) {
      printf("Server Has Reached the Max Amount of Clients!\n");
      close(connfd);
      continue;
    }

    struct client *cli = (struct client *)xmalloc(sizeof(struct client));

    cli->address = cli_addr;
    cli->sockfd = connfd;

    queue_add(cli); // put cli in first NULL spot in clients array
    pthread_create(&threadid, NULL, &handle_client, (void *)cli);
    sleep(1); // save some cpu usage
  }

  return EXIT_SUCCESS;
}
